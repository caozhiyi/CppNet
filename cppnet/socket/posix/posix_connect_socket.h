// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_POSIX_CONNECT_SOCKET
#define CPPNET_SOCKET_POSIX_CONNECT_SOCKET

#include "../connect_socket.h"

namespace cppnet {

class Event;
class PosixConnectSocket:
    public ConnectSocket { 
public:
    PosixConnectSocket();
    ~PosixConnectSocket();

    virtual void Accept();
    virtual void OnAccept();
private:
    Event*  _accept_event;
};

}

#endif