#ifndef NET_SOCKET_READ_WRITE_SOCKET
#define NET_SOCKET_READ_WRITE_SOCKET

#include "socket_interface.h"

namespace cppnet {

class Event;
class BufferQueue;
class AlloterWrap;
class BlockMemoryPool;

class RWSocket:
    public Socket, 
    public std::enable_shared_from_this<RWSocket> { 

public:
    RWSocket(std::shared_ptr<AlloterWrap> alloter);
    ~RWSocket();

    void Read();
    void Write(const char* src, uint32_t len);
    void Connect(const std::string& ip, uint16_t port);
    void Disconnect();

    void OnRead(uint32_t len = 0);
    void OnWrite(uint32_t len = 0);
    void OnConnect(uint16_t err);
    void OnDisConnect(uint16_t err);

    std::shared_ptr<AlloterWrap> GetAlocter() { return _alloter; }

private:
    void Recv();
    void Send();
    
private:
    std::shared_ptr<Event>  _event;

    std::shared_ptr<BufferQueue> _write_buffer;
    std::shared_ptr<BufferQueue> _read_buffer;

    std::shared_ptr<AlloterWrap>     _alloter;
    std::shared_ptr<BlockMemoryPool> _block_pool;
};

}

#endif