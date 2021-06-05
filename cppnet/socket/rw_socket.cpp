// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <errno.h>

#include "rw_socket.h"
#include "cppnet/dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"

#include "common/log/log.h"
#include "common/alloter/pool_block.h"
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
    _timer_id(0),
    _listen_port(0),
    _alloter(alloter) {
    _block_pool = _alloter->PoolNewSharePtr<BlockMemoryPool>(__mem_block_size, __mem_block_add_step);
}

RWSocket::~RWSocket() {
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

void RWSocket::OnTimer() {
    auto cppnet_base = _cppnet_base.lock();
    if (!cppnet_base) {
        return;
    }
    cppnet_base->OnTimer(shared_from_this());
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

void RWSocket::OnConnect(Event*, uint16_t err) {
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

}