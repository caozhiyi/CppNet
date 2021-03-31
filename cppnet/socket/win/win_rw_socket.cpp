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
    RWSocket(alloter) {

}

WinRWSocket::WinRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    RWSocket(sock, alloter) {

}

WinRWSocket::~WinRWSocket() {

}

void WinRWSocket::Read() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<RWEvent>();
        _event->SetSocket(shared_from_this());
    }

    RWSocket::Read();
}

bool WinRWSocket::Write(const char* src, uint32_t len) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<RWEvent>();
        _event->SetSocket(shared_from_this());
    }

    //can't send now
    if (_write_buffer->GetCanReadLength() > 0) {
        if (_write_buffer->GetCanReadLength() > __max_write_cache) {
            return false;
        }
        
        _write_buffer->Write(src, len);
        return false;

    } else {
        _write_buffer->Write(src, len);
        return Send();
    }
}

void WinRWSocket::Connect(const std::string& ip, uint16_t port) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<RWEvent>();
        _event->SetSocket(shared_from_this());
    }
    RWSocket::Connect(ip, port);
}

void WinRWSocket::Disconnect() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    RWSocket::Disconnect();
}

void WinRWSocket::OnRead(uint32_t len) {
    Recv(len);
    // wait for read again
    Read();
}

void WinRWSocket::OnWrite(uint32_t len) {
    Send(len);
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
    }
    
    auto actions = GetEventActions();
    if (actions && _write_buffer->GetCanReadLength() > 0) {
        actions->AddSendEvent(_event);
    }

    return true;
}

}