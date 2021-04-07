// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_CONNECT_SOCKET
#define CPPNET_SOCKET_CONNECT_SOCKET

#include <memory>
#include "socket_interface.h"

namespace cppnet {

class Event;
class Buffer;
class AlloterWrap;
class BlockMemoryPool;

class ConnectSocket:
    public Socket, 
    public std::enable_shared_from_this<ConnectSocket> { 
public:
    ConnectSocket();
    virtual ~ConnectSocket();

    virtual bool Bind(const std::string& ip, uint16_t port);
    virtual bool Listen();
    virtual void Accept();

    virtual void OnAccept() {}

protected:
    std::shared_ptr<Event>  _accept_event;
};

std::shared_ptr<ConnectSocket> MakeConnectSocket();

}

#endif