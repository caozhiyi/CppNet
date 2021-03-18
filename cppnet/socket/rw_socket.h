#ifndef NET_SOCKET_READ_WRITE_SOCKET
#define NET_SOCKET_READ_WRITE_SOCKET

#include "socket_interface.h"
#include "include/cppnet_socket.h"

namespace cppnet {

class Event;
class Dispatcher;
class BufferQueue;
class AlloterWrap;
class BlockMemoryPool;

class RWSocket:
    public Socket, 
    public CNSocket,
    public std::enable_shared_from_this<RWSocket> { 

public:
    RWSocket(std::shared_ptr<AlloterWrap> alloter);
    virtual ~RWSocket();

    bool GetAddress(std::string& ip, uint16_t& port);

    bool Close();

    void Read();
    bool Write(const char* src, uint32_t len);
    void Connect(const std::string& ip, uint16_t port);
    void Disconnect();

    uint64_t AddTimer(uint32_t interval, bool always = false);
    void StopTimer(uint64_t timer_id);

    void OnTimer();
    void OnRead(uint32_t len = 0);
    void OnWrite(uint32_t len = 0);
    void OnConnect(uint16_t err);
    void OnDisConnect(uint16_t err);

    std::shared_ptr<AlloterWrap> GetAlocter() { return _alloter; }

    std::shared_ptr<BufferQueue> GetReadBuffer() { return _read_buffer; }

private:
    bool Recv();
    bool Send();
    
private:
    std::shared_ptr<Event>  _event;

    std::shared_ptr<BufferQueue> _write_buffer;
    std::shared_ptr<BufferQueue> _read_buffer;

    std::shared_ptr<AlloterWrap>     _alloter;
    std::shared_ptr<BlockMemoryPool> _block_pool;
};

}

#endif