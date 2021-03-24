#ifndef CPPNET_SOCKET_POSIX_CONNECT_SOCKET
#define CPPNET_SOCKET_POSIX_CONNECT_SOCKET

#include "../connect_socket.h"

namespace cppnet {

class PosixConnectSocket:
    public ConnectSocket { 
public:
    PosixConnectSocket();
    ~PosixConnectSocket();

    virtual void OnAccept();
};

}

#endif