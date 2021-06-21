// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <errno.h>

#include "rw_socket.h"
#include "cppnet/dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

#include "common/log/log.h"
#include "common/network/socket.h"
#include "common/alloter/pool_block.h"
#include "common/buffer/buffer_queue.h"
#include "common/alloter/pool_alloter.h"

namespace cppnet {

RWSocket::RWSocket():
    RWSocket(0, std::make_shared<AlloterWrap>(MakePoolAlloterPtr())) {
}

RWSocket::RWSocket(std::shared_ptr<AlloterWrap> alloter):
    RWSocket(0, alloter) {

}

RWSocket::RWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    Socket(sock),
    _context(nullptr),
    _timer_id(0),
    _listen_port(0),
    _shutdown(false),
    _connecting(false),
    _event(nullptr),
    _alloter(alloter) {

    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);

    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

RWSocket::~RWSocket() {
    _write_buffer.reset();
    _read_buffer.reset();
    if (_timer_id > 0) {
        auto dispatcher = GetDispatcher();
        if (dispatcher) {
            dispatcher->StopTimer(_timer_id);
        }
        _timer_id = 0;
    }
}

bool RWSocket::GetAddress(std::string& ip, uint16_t& port) {
    ip = _addr.GetIp();
    port = _addr.GetAddrPort();
    return true;
}

void RWSocket::Close() {
    Disconnect();
    if (_timer_id > 0) {
        auto dispatcher = GetDispatcher();
        if (dispatcher) {
            dispatcher->StopTimer(_timer_id);
        }
        _timer_id = 0;
    }
}

void RWSocket::Read() {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddRecvEvent(_event);
    }
}

bool RWSocket::Write(const char* src, uint32_t len) {
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

void RWSocket::Connect(const std::string& ip, uint16_t port) {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    bool use_ipv4 = false;
    if (Address::IsIpv4(ip)) {
        use_ipv4 = true;
    }
    if (_sock == 0) {
        auto ret = OsHandle::TcpSocket(use_ipv4);
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. error:%d", ret._errno);
            return;
        }
        _sock = ret._return_value;
    }

    _addr.SetType(use_ipv4 ? AT_IPV4 : AT_IPV6);
    _addr.SetIp(ip);
    _addr.SetAddrPort(port);

    auto actions = GetEventActions();
    if (actions) {
        _connecting = true;
        actions->AddConnection(_event, _addr);
    }
}

void RWSocket::Disconnect() {
    if (!_event) {
        _event = _alloter->PoolNew<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddDisconnection(_event);
    }
}

void RWSocket::AddTimer(uint32_t interval, bool always) {
    if (_timer_id > 0) {
        return;
    }
    
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
        _timer_id = dispatcher->AddTimer(shared_from_this(), interval, always);
    }
}

void RWSocket::StopTimer() {
    if (_timer_id == 0) {
        return;
    }
    
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
        dispatcher->StopTimer(_timer_id);
        _timer_id = 0;
    }
}

void RWSocket::OnTimer() {
    /*if (_connecting) {
        if (CheckConnect(GetSocket())) {
            OnConnect(CEC_SUCCESS);
        } else {
            OnConnect(CEC_CONNECT_REFUSE);
        }
        return;
    }*/
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    cppnet_base->OnTimer(shared_from_this());
}

void RWSocket::OnRead(uint32_t len) {
    Recv(len);
}

void RWSocket::OnWrite(uint32_t len) {
    Send();
}

void RWSocket::OnConnect(uint16_t err) {
    _connecting = false;
    auto sock = shared_from_this();
    if (err == CEC_SUCCESS) {
        __all_socket_map[_sock] = sock;
    }
    
    auto cppnet_base = _cppnet_base.lock();
    if (cppnet_base) {
        cppnet_base->OnConnect(sock, err);
    }

    if (err == CEC_SUCCESS) {
        Read();
    }
}

void RWSocket::OnDisConnect(uint16_t err) {
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

bool RWSocket::Recv(uint32_t len) {
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
                OnDisConnect(CEC_CONNECT_BREAK);
                return false;
            }

        } else if (ret._return_value == 0) {
            OnDisConnect(CEC_CLOSED);
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

bool RWSocket::Send() {
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
                OnDisConnect(CEC_CONNECT_BREAK);
                return false;

            } else {
                OnDisConnect(CEC_CLOSED);
                return false;
            }
        }
    }
    cppnet_base->OnWrite(shared_from_this(), off_set);
    return true;
}

std::shared_ptr<RWSocket> MakeRWSocket() {
    return std::make_shared<RWSocket>();
}

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<RWSocket>(alloter);
}

std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter) {
    return std::make_shared<RWSocket>(sock, alloter);
}


}