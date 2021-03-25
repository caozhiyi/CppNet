#ifndef CPPNET_SOCKET_POSIX_CONNECT_SOCKET
#define CPPNET_SOCKET_POSIX_CONNECT_SOCKET

#include "cppnet/socket/connect_socket.h"

namespace cppnet {

class WinConnectSocket:
    public ConnectSocket { 
public:
    WinConnectSocket();
    ~WinConnectSocket();

    virtual void Accept();

    virtual void OnAccept();
};

}

#endif