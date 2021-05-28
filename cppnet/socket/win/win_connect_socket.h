// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_POSIX_CONNECT_SOCKET
#define CPPNET_SOCKET_POSIX_CONNECT_SOCKET

#include "cppnet/socket/connect_socket.h"

namespace cppnet {

class Event;
class WinAcceptEvent;
class WinConnectSocket:
    public ConnectSocket { 
public:
    WinConnectSocket();
    ~WinConnectSocket();

    virtual bool Bind(const std::string& ip, uint16_t port);
    virtual void Accept();
    virtual void Accept(uint16_t index);
    virtual void Close();

    virtual void OnAccept(Event* event);

private:
    std::vector<Event*>  _accept_event_vec;
};

}

#endif