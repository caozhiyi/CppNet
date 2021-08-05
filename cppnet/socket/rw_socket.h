// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_READ_WRITE_SOCKET
#define CPPNET_SOCKET_READ_WRITE_SOCKET

#include <atomic>
#include "socket_interface.h"
#include "include/cppnet_socket.h"
#include "foundation/buffer/buffer_queue.h"
#include "foundation/alloter/alloter_interface.h"

namespace cppnet {

class Event;
class BlockMemoryPool;

class RWSocket:
    public Socket, 
    public CNSocket,
    public std::enable_shared_from_this<RWSocket> { 
public:
    RWSocket();
    RWSocket(std::shared_ptr<fdan::AlloterWrap> alloter);
    RWSocket(uint64_t sock, std::shared_ptr<fdan::AlloterWrap> alloter);
    virtual ~RWSocket();

    virtual uint64_t GetSocket() { return _sock; }
    virtual void SetListenPort(uint16_t port) { _listen_port = port; }
    virtual uint16_t GetListenPort() { return _listen_port; }
    virtual bool GetAddress(std::string& ip, uint16_t& port);

    virtual void Close();

    virtual void Read();
    virtual bool Write(const char* src, uint32_t len);
    virtual void Connect(const std::string& ip, uint16_t port);
    virtual void Disconnect();

    virtual void AddTimer(uint32_t interval, bool always = false);
    virtual void StopTimer();

    virtual void OnTimer();
    virtual void OnRead(uint32_t len = 0);
    virtual void OnWrite(uint32_t len = 0);
    virtual void OnConnect(uint16_t err);
    virtual void OnDisConnect(uint16_t err);

    virtual void SetContext(void* context) { _context = context; }
    virtual void* GetContext() { return _context; }

    virtual void SetShutdown() { _shutdown = true; }
    virtual bool IsShutdown() { return _shutdown; }

    std::shared_ptr<fdan::AlloterWrap> GetAlloter() { return _alloter; }

private:
    bool Recv(uint32_t len);
    bool Send();

protected:
    void*    _context;
    uint32_t _timer_id;
    uint16_t _listen_port;
    std::atomic_bool _shutdown;
    std::atomic_bool _connecting;
    Event*           _event;

    std::shared_ptr<BufferQueue>     _write_buffer;
    std::shared_ptr<BufferQueue>     _read_buffer;

    std::shared_ptr<fdan::AlloterWrap>     _alloter;
    std::shared_ptr<BlockMemoryPool> _block_pool;

    static thread_local std::unordered_map<uint64_t, std::shared_ptr<Socket>> __connecting_socket_map;
};

std::shared_ptr<RWSocket> MakeRWSocket();
std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<fdan::AlloterWrap> alloter);
std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<fdan::AlloterWrap> alloter);

}

#endif