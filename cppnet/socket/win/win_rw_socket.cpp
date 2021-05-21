// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "win_rw_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/win/rw_event.h"
#include "cppnet/event/action_interface.h"

#include "common/log/log.h"
#include "common/buffer/buffer_queue.h"

namespace cppnet {

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<WinRWSocket>(alloter);
}

std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<WinRWSocket>(sock, alloter);
}

WinRWSocket::WinRWSocket(std::shared_ptr<AlloterWrap> alloter): 
    RWSocket(alloter),
    _ref_count(0),
    _shutdown(false) {

}

WinRWSocket::WinRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    RWSocket(sock, alloter),
    _ref_count(0),
    _shutdown(false) {

}

WinRWSocket::~WinRWSocket() {

}

void WinRWSocket::Read() {
    if (_is_reading) {
        LOG_WARN_S << "already in reading. sock:" << _sock;
        return;
    }

    auto rw_event = _alloter->PoolNewSharePtr<WinRWEvent>();
    auto buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    rw_event->SetBuffer(buffer);

    auto event = std::dynamic_pointer_cast<Event>(rw_event);
    event->SetSocket(shared_from_this());

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddRecvEvent(event)) {
            _is_reading = true;
            AddEvent(event);
        }
    }
}

bool WinRWSocket::Write(const char* src, uint32_t len) {
    auto rw_event = _alloter->PoolNewSharePtr<WinRWEvent>();
    auto buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    rw_event->SetBuffer(buffer);

    buffer->Write(src, len);

    auto event = std::dynamic_pointer_cast<Event>(rw_event);
    event->SetSocket(shared_from_this());

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddSendEvent(event)) {
            AddEvent(event);
            return true;
        }
    }
    return false;
}

void WinRWSocket::Connect(const std::string& ip, uint16_t port) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<WinRWEvent>();
        _event->SetSocket(shared_from_this());
    }
    RWSocket::Connect(ip, port);
}

void WinRWSocket::Disconnect() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        if (actions->AddDisconnection(_event)) {
            Incref();
        }
    }
}

void WinRWSocket::OnRead(uint32_t len) {
    // do nothind
}

void WinRWSocket::OnRead(std::shared_ptr<Event>& event, uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    if (len == 0) {
        LOG_ERROR("read invalid length. sock:%d", _sock);
        return;
    }

    auto rw_event = std::dynamic_pointer_cast<WinRWEvent>(event);

    auto buffer = rw_event->GetBuffer();
    buffer->MoveWritePt(len);

    if (!_read_buffer) {
        _read_buffer = buffer;

    } else {
        buffer->Read(_read_buffer, len);
    }

    RemvoeEvent(event);
    _is_reading = false;

    cppnet_base->OnRead(shared_from_this(), len);

    if (_read_buffer->GetCanReadLength() == 0) {
        _read_buffer.reset();
    }

    // read again
    Read();
}

void WinRWSocket::OnWrite(uint32_t len) {
    // do nothing
}

void WinRWSocket::OnWrite(std::shared_ptr<Event>& event, uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    if (len == 0) {
        LOG_ERROR("read invalid length. sock:%d", _sock);
        return;
    }

    auto rw_event = std::dynamic_pointer_cast<WinRWEvent>(event);
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

void WinRWSocket::OnDisConnect(uint16_t err) {
    // do nothing
}

void WinRWSocket::OnDisConnect(std::shared_ptr<Event>& event, uint16_t err) {
    RemvoeEvent(event);

    if (EventEmpty() && IsShutdown()) {
        auto sock = shared_from_this();
        __all_socket_map.Erase(_sock);
        auto cppnet_base = _cppnet_base.lock();

        if (cppnet_base) {
            cppnet_base->OnDisConnect(sock, err);
        }

        OsHandle::Close(_sock);
    }
}

void WinRWSocket::AddEvent(std::shared_ptr<Event>& event) {
    std::lock_guard<std::mutex> lock(_event_mutex);
    _event_set.insert(event);
}

void WinRWSocket::RemvoeEvent(std::shared_ptr<Event>& event) {
    std::lock_guard<std::mutex> lock(_event_mutex);
    _event_set.erase(event);
}

bool WinRWSocket::EventEmpty() {
    std::lock_guard<std::mutex> lock(_event_mutex);
    return _event_set.empty();
}

bool WinRWSocket::Decref(uint16_t err) {
    int16_t ref = _ref_count.fetch_sub(1);
    if (ref == 1 && IsShutdown()) {
        RWSocket::OnDisConnect(err);
        return false;
    }
    return true;
}

bool WinRWSocket::Recv(uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
    }
    if (len == 0) {
        LOG_ERROR("read invalid length. sock:%d", _sock);
    }

    _read_buffer->MoveWritePt(len);
    cppnet_base->OnRead(shared_from_this(), len);
    return true;
}

bool WinRWSocket::Send(uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
    }
    if (len > 0) {
        _write_buffer->MoveReadPt(len);
        cppnet_base->OnWrite(shared_from_this(), len);
        // do read again
        Read();
    }
    
    return true;
}

}