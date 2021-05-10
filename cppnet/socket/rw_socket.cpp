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
#include "common/alloter/pool_block.h"
#include "common/buffer/buffer_queue.h"
#include "common/alloter/alloter_interface.h"

namespace cppnet {
RWSocket::RWSocket(std::shared_ptr<AlloterWrap> alloter): 
    Socket(alloter) {

    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);
    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

RWSocket::RWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter):
    Socket(sock, alloter) {

    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);
    _write_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
    _read_buffer = _alloter->PoolNewSharePtr<BufferQueue>(_block_pool, _alloter);
}

RWSocket::~RWSocket() {
    // free buffer early than pool
    _write_buffer.reset();
    _read_buffer.reset();
    _block_pool.reset();
}

bool RWSocket::GetAddress(std::string& ip, uint16_t& port) {
    ip = _addr.GetIp();
    port = _addr.GetAddrPort();

    return true;
}

bool RWSocket::Close() {
    Disconnect();
    return true;
}

void RWSocket::Read() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddRecvEvent(_event);
    }
}

void RWSocket::Connect(const std::string& ip, uint16_t port) {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
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

void RWSocket::Disconnect() {
    if (!_event) {
        _event = _alloter->PoolNewSharePtr<Event>();
        _event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        actions->AddDisconnection(_event);
    }
}

void RWSocket::OnTimer() {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    cppnet_base->OnTimer(shared_from_this());
}

uint64_t RWSocket::AddTimer(uint32_t interval, bool always) {
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
        return dispatcher->AddTimer(shared_from_this(), interval, always);
    }
    return 0;
}

void RWSocket::StopTimer(uint64_t timer_id) {
    auto dispatcher = GetDispatcher();
    if (dispatcher) {
        dispatcher->StopTimer(timer_id);
    }
}

void RWSocket::OnConnect(uint16_t err) {
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
    auto cppnet_base = _cppnet_base.lock();
    if (cppnet_base) {
        cppnet_base->OnDisConnect(sock, err);
    }

    // not active disconnection
    if (_event && !(_event->GetType() & ET_DISCONNECT)) {
        OsHandle::Close(_sock);
    }
}

}