// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <errno.h>

#include "posix_connect_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/socket/rw_socket.h"

#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/network/io_handle.h"
#include "common/alloter/pool_alloter.h"

namespace cppnet {

std::shared_ptr<ConnectSocket> MakeConnectSocket() {
    return std::make_shared<PosixConnectSocket>();
}

PosixConnectSocket::PosixConnectSocket() {

}

PosixConnectSocket::~PosixConnectSocket() {

}

void PosixConnectSocket::OnAccept() {
    while (true) {
        std::shared_ptr<AlloterWrap> alloter = std::make_shared<AlloterWrap>(MakePoolAlloterPtr());
        Address address;
        //may get more than one connections
        auto ret = OsHandle::Accept(_sock, address);
        if (ret._return_value < 0) {
            if (errno == EAGAIN) {
                break;
            }
            LOG_ERROR("accept socket filed! errno:%d, info:%s", ret._errno, ErrnoInfo(ret._errno));
            break;
        }

        auto cppnet_base = _cppnet_base.lock();
        if (!cppnet_base) {
            return;
        }

        //set the socket noblocking
        SocketNoblocking(ret._return_value);
        
        //create a new socket.
        auto sock = MakeRWSocket(ret._return_value, alloter);

        sock->SetCppNetBase(cppnet_base);
        sock->SetEventActions(_event_actions);
        sock->SetAddress(std::move(address));
        sock->SetDispatcher(GetDispatcher());

        __all_socket_map[ret._return_value] = sock;

        //call accept call back function
        cppnet_base->OnAccept(sock);

        //start read
        sock->Read();
    }
}

}