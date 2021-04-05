#ifndef CPPNET_SOCKET_READ_WRITE_SOCKET
#define CPPNET_SOCKET_READ_WRITE_SOCKET

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
    RWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);
    virtual ~RWSocket();

    virtual bool GetAddress(std::string& ip, uint16_t& port);

    virtual bool Close();

    virtual void Read();
    virtual bool Write(const char* src, uint32_t len) { return false; }
    virtual void Connect(const std::string& ip, uint16_t port);
    virtual void Disconnect();

    virtual uint64_t AddTimer(uint32_t interval, bool always = false);
    virtual void StopTimer(uint64_t timer_id);

    virtual void OnTimer();
    virtual void OnRead(uint32_t len = 0) {}
    virtual void OnWrite(uint32_t len = 0) {}
    virtual void OnConnect(uint16_t err);
    virtual void OnDisConnect(uint16_t err);

    std::shared_ptr<BufferQueue> GetReadBuffer() { return _read_buffer; }
    std::shared_ptr<BufferQueue> GetWriteBuffer() { return _write_buffer; }
    
protected:
    std::shared_ptr<BlockMemoryPool> _block_pool;

    std::shared_ptr<Event>  _event;

    std::shared_ptr<BufferQueue> _write_buffer;
    std::shared_ptr<BufferQueue> _read_buffer;
};

std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter);
std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);

}

#endif