// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "win_rw_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/win/iocp_action.h"

#include "common/log/log.h"
#include "common/buffer/buffer_queue.h"
#include "common/alloter/pool_alloter.h"

namespace cppnet {

WinRWSocket::WinRWSocket():
    RWSocket(),
    _is_reading(false) {

}

WinRWSocket::WinRWSocket(std::shared_ptr<AlloterWrap> alloter):
    RWSocket(alloter),
    _is_reading(false) {

}

WinRWSocket::WinRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    RWSocket(sock, alloter),
    _is_reading(false) {

}

WinRWSocket::~WinRWSocket() {

}

void WinRWSocket::Read() {
    if (_is_reading) {
        LOG_WARN_S << "already in reading. sock:" << _sock;
        return;
    }

    auto rw_event = _alloter->PoolNew<Event>();
    auto buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    rw_event->SetBuffer(buffer);
    rw_event->SetSocket(shared_from_this());

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddRecvEvent(rw_event)) {
            _is_reading = true;
            AddEvent(rw_event);
            return;
        }
    }

    if (!IsShutdown()) {
        _alloter->PoolDelete<Event>(rw_event);
    }
}

bool WinRWSocket::Write(const char* src, uint32_t len) {
    if (IsShutdown()) {
        LOG_WARN_S << "already shutdown when write. sock:" << _sock;
        return false;
    }
    // create new write buffer
    auto buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    buffer->Write(src, len);

    // create new write event
    auto rw_event = _alloter->PoolNew<Event>();
    rw_event->SetBuffer(buffer);
    rw_event->SetSocket(shared_from_this());

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddSendEvent(rw_event)) {
            AddEvent(rw_event);
            return true;
        }
    }
    if (!IsShutdown()) {
        _alloter->PoolDelete<Event>(rw_event);
    }
    return false;
}

void WinRWSocket::Connect(const std::string& ip, uint16_t port) {
    if (_sock == 0) {
        auto ret = OsHandle::TcpSocket();
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. error:%d", ret._errno);
            return;
        }
        _sock = ret._return_value;
    }

    // add to IOCP.
    auto action = GetEventActions();
    auto iocp = std::dynamic_pointer_cast<IOCPEventActions>(action);
    if (!iocp->AddToIOCP(_sock)) {
        LOG_FATAL("add connect socket to iocp failed!");
        OsHandle::Close(_sock);
        return;
    }

    _addr.SetIp(ip);
    _addr.SetAddrPort(port);

    auto rw_event = _alloter->PoolNew<Event>();
    rw_event->SetSocket(shared_from_this());

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddConnection(rw_event, _addr)) {
            AddEvent(rw_event);
            return;
        }
    }

    if (!IsShutdown()) {
        _alloter->PoolDelete<Event>(rw_event);
    }
}

void WinRWSocket::Disconnect() {
    auto rw_event = _alloter->PoolNew<Event>();
    rw_event->SetSocket(shared_from_this());

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddDisconnection(rw_event)) {
            AddEvent(rw_event);
        }
    }
}

void WinRWSocket::OnRead(Event* event, uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    if (len == 0) {
        LOG_ERROR("read invalid length. sock:%d", _sock);
        return;
    }

    auto rw_event = dynamic_cast<Event*>(event);
    auto buffer = rw_event->GetBuffer();
    buffer->MoveWritePt(len);

    if (!_read_buffer) {
        _read_buffer = buffer;

    } else {
        buffer->Read(_read_buffer, len);
    }

    cppnet_base->OnRead(shared_from_this(), _read_buffer, len);

    if (_read_buffer->GetCanReadLength() == 0) {
        _read_buffer.reset();
    }

    RemvoeEvent(event);
    _is_reading = false;
    // read again
    Read();
}

void WinRWSocket::OnWrite(Event* event, uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    if (len == 0) {
        LOG_ERROR("read invalid length. sock:%d", _sock);
        return;
    }

    auto rw_event = dynamic_cast<Event*>(event);
    auto buffer = rw_event->GetBuffer();
    buffer->MoveReadPt(len);

    cppnet_base->OnWrite(shared_from_this(), len);

    // post send event again.
    if (buffer->GetCanReadLength() > 0) {
        auto actions = GetEventActions();
        if (actions) {
            if (actions->AddSendEvent(event)) {
                LOG_ERROR("post send event. sock:%d", _sock);
            }
        }

    } else {
        RemvoeEvent(event);
    }
}

void WinRWSocket::OnDisConnect(Event* event, uint16_t err) {
    // local disconnect
    if (event->GetType() & ET_DISCONNECT) {
        auto actions = GetEventActions();
        if (actions) {
            std::lock_guard<std::mutex> lock(_event_mutex);
            for (auto iter = _event_set.begin(); iter != _event_set.end(); iter++) {
                actions->DelEvent(*iter);
                _event_set.erase(event);
                EventOverlapped* data = (EventOverlapped*)event->GetData();
                if (data) {
                    _alloter->PoolDelete<EventOverlapped>(data);
                }
                _alloter->PoolDelete<Event>(event);
            }
        }
    }

    RemvoeEvent(event);

    if (EventEmpty() && IsShutdown()) {
        if (__all_socket_map.Find(_sock)) {
            auto sock = shared_from_this();
            auto cppnet_base = _cppnet_base.lock();
            if (cppnet_base) {
                cppnet_base->OnDisConnect(sock, err);
            }
            OsHandle::Close(_sock);
            __all_socket_map.Erase(_sock);
        }
    }
}

std::shared_ptr<BufferQueue> WinRWSocket::GetReadBuffer() {
    if (!_read_buffer) {
        _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    }
    return _read_buffer;
}

void WinRWSocket::AddEvent(Event* event) {
    std::lock_guard<std::mutex> lock(_event_mutex);
    _event_set.insert(std::move(event));
}

void WinRWSocket::RemvoeEvent(Event* event) {
    std::lock_guard<std::mutex> lock(_event_mutex);
    _event_set.erase(event);
    EventOverlapped* data = (EventOverlapped*)event->GetData();
    if (data) {
        _alloter->PoolDelete<EventOverlapped>(data);
    }
    _alloter->PoolDelete<Event>(event);
}

bool WinRWSocket::EventEmpty() {
    std::lock_guard<std::mutex> lock(_event_mutex);
    return _event_set.empty();
}

std::shared_ptr<RWSocket> MakeRWSocket() {
    return std::make_shared<WinRWSocket>();
}

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<WinRWSocket>(alloter);
}

std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<WinRWSocket>(sock, alloter);
}


}