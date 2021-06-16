// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <errno.h>
#include "win_rw_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

#include "common/log/log.h"
#include "common/buffer/buffer_queue.h"

namespace cppnet {

std::shared_ptr<RWSocket> MakeRWSocket() {
    return std::make_shared<PosixRWSocket>();
}

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<PosixRWSocket>(alloter);
}

std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<PosixRWSocket>(sock, alloter);
}

PosixRWSocket::PosixRWSocket():
    RWSocket(),
    _event(nullptr) {

    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

PosixRWSocket::PosixRWSocket(std::shared_ptr<AlloterWrap> alloter): 
    RWSocket(alloter),
    _event(nullptr) {
    
    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

PosixRWSocket::PosixRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    RWSocket(sock, alloter),
    _event(nullptr) {

    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

PosixRWSocket::~PosixRWSocket() {
    if (_event) {
        _alloter->PoolDelete<Event>(_event);
    }
}

void PosixRWSocket::Read() {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddRecvEvent(_event);
    }
}

bool PosixRWSocket::Write(const char* src, uint32_t len) {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    //can't send now
    if (_write_buffer->GetCanReadLength() > 0) {
        if (_write_buffer->GetCanReadLength() > __max_write_cache) {
            return false;
        }
        
        _write_buffer->Write(src, len);
        auto actions = GetEventActions();
        if (actions) {
            return actions->AddSendEvent(_event);
        }
        return false;

    } else {
        _write_buffer->Write(src, len);
        return Send();
    }
}

void PosixRWSocket::Connect(const std::string& ip, uint16_t port) {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    if (_sock == 0) {
        auto ret = OsHandle::TcpSocket();
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. error:%d", ret._errno);
            return;
        }
        _sock = ret._return_value;
    }


    _addr.SetIp(ip);
    _addr.SetAddrPort(port);

    auto actions = GetEventActions();
    if (actions) {
        actions->AddConnection(_event, _addr);
    }
}

void PosixRWSocket::Disconnect() {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddDisconnection(_event);
    }
}

void PosixRWSocket::OnRead(Event*, uint32_t len) {
    Recv(len);
}

void PosixRWSocket::OnWrite(Event*,uint32_t len) {
    Send();
}

void PosixRWSocket::OnDisConnect(Event*, uint16_t err) {
    auto sock = shared_from_this();
    __all_socket_map.erase(_sock);

    if (!IsShutdown()) {
        auto cppnet_base = _cppnet_base.lock();
        if (cppnet_base) {
            cppnet_base->OnDisConnect(sock, err);
        }
    }
    SetShutdown();

    // peer disconnect or connection break.
    if (_event && err != CEC_SUCCESS) {
        auto actions = GetEventActions();
        if (actions) {
            actions->DelEvent(_event);
        }
        OsHandle::Close(_sock);
    }
}

bool PosixRWSocket::Recv(uint32_t len) {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
    }
    if (len == 0) {
        len = __linux_read_buff_expand_len;
    }

    uint32_t off_set = 0;
    //read all data.
    uint32_t expand_buff_len = len;
    bool need_expend = false;
    while (true) {
        uint32_t expand = 0;
        if (need_expend) {
            expand = expand_buff_len;
            if (expand_buff_len < __linux_read_buff_expand_max) {
                expand_buff_len *= 2;
            }
        }

        std::vector<Iovec> io_vec;
        uint32_t buff_len = _read_buffer->GetFreeMemoryBlock(io_vec, expand);
        auto ret = OsHandle::Readv(_sock, &*io_vec.begin(), io_vec.size());
        if (ret._return_value < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
                _read_buffer->MoveWritePt(ret._return_value);
                off_set += ret._return_value;
                break;

            } else {
                OnDisConnect(nullptr, CEC_CONNECT_BREAK);
                return false;
            }

        } else if (ret._return_value == 0) {
            OnDisConnect(nullptr, CEC_CLOSED);
            return false;

        } else {
            _read_buffer->MoveWritePt(ret._return_value);
            off_set += ret._return_value;
            // read all
            if ((uint32_t)ret._return_value < buff_len) {
                break;
            }
            need_expend = true;
        }
    }
    cppnet_base->OnRead(shared_from_this(), _read_buffer, off_set);
    return true;
}

bool PosixRWSocket::Send() {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return false;
    }

    uint32_t off_set = 0;
    while(_write_buffer && _write_buffer->GetCanReadLength() > 0) {
        std::vector<Iovec> io_vec;
        _write_buffer->GetUseMemoryBlock(io_vec, __linux_write_buff_get);
        auto ret = OsHandle::Writev(_sock, &*io_vec.begin(), io_vec.size());
        if (ret._return_value >= 0) {
            _write_buffer->MoveReadPt(ret._return_value);
            off_set += ret._return_value;

        } else {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
                //can't send complete
                auto actions = GetEventActions();
                if (actions) {
                    return actions->AddSendEvent(_event);
                }
                return false;

            } else if (errno == EBADMSG) {
                OnDisConnect(nullptr, CEC_CONNECT_BREAK);
                return false;

            } else {
                OnDisConnect(nullptr, CEC_CLOSED);
                return false;
            }
        }
    }

    cppnet_base->OnWrite(shared_from_this(), off_set);
    return true;
}

}