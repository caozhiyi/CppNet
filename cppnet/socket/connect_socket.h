#ifndef NET_SOCKET_CONNECT_SOCKET
#define NET_SOCKET_CONNECT_SOCKET

#include <memory>
#include "socket_interface.h"

namespace cppnet {

class Buffer;
class AlloterWrap;
class BlockMemoryPool;

class ConnectSocket:
    public Socket, 
    public std::enable_shared_from_this<ConnectSocket> { 
public:
    ConnectSocket();
    ~ConnectSocket();

    bool Bind(const std::string& ip, uint16_t port);
    bool Listen();
    void Accept();

    void OnAccept();

private:
    std::shared_ptr<Event>  _read_event;
};

}

#endif