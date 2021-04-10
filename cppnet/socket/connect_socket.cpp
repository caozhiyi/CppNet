// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <string>
#include <errno.h>

#include "rw_socket.h"
#include "connect_socket.h"
#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/network/address.h"
#include "common/network/io_handle.h"
#include "common/alloter/pool_alloter.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/event/event_interface.h"
#include "cppnet/event/action_interface.h"

namespace cppnet {

ConnectSocket::ConnectSocket() {

}

ConnectSocket::~ConnectSocket() {

}

bool ConnectSocket::Bind(const std::string& ip, uint16_t port) {
    if (_sock == 0) {
        auto ret = OsHandle::TcpSocket();
        if (ret._return_value < 0) {
            LOG_ERROR("create socket failed. errno:%d, info:%s", ret._errno, ErrnoInfo(ret._errno));
            return false;
        }
        _sock = ret._return_value;
    }

    _addr.SetIp(ip);
    _addr.SetPort(port);

    auto ret = OsHandle::Bind(_sock, _addr);

    if (ret._return_value < 0) {
        LOG_FATAL("linux bind socket filed! error:%d, info:%s", ret._errno, ErrnoInfo(ret._errno));
        OsHandle::Close(_sock);
        return false;
    }

    return true;
}

bool ConnectSocket::Listen() {
    auto ret = OsHandle::Listen(_sock);
    if (ret._return_value < 0) {
        LOG_FATAL("linux listen socket filed! error:%d, info:%s", ret._errno, ErrnoInfo(ret._errno));
        OsHandle::Close(_sock);
        return false;
    }

    //set the socket noblocking
    SocketNoblocking(_sock);

    Accept();

    return true;
}

void ConnectSocket::Accept() {
    if (!_accept_event) {
        _accept_event = std::make_shared<Event>();
        _accept_event->SetSocket(shared_from_this());
    }
    __all_socket_map[_sock] = shared_from_this();
    auto actions = GetEventActions();
    if (actions) {
        actions->AddAcceptEvent(_accept_event);
    }
}

}