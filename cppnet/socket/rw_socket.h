// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_READ_WRITE_SOCKET
#define CPPNET_SOCKET_READ_WRITE_SOCKET

#include "socket_interface.h"
#include "include/cppnet_socket.h"

namespace cppnet {

class Event;
class BufferQueue;
class AlloterWrap;
class BlockMemoryPool;

class RWSocket:
    public Socket, 
    public CNSocket,
    public std::enable_shared_from_this<RWSocket> { 

public:
    RWSocket();
    RWSocket(std::shared_ptr<AlloterWrap> alloter);
    RWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);
    virtual ~RWSocket();

    virtual bool GetAddress(std::string& ip, uint16_t& port);

    virtual bool Close();

    virtual void Read() {}
    virtual bool Write(const char* src, uint32_t len) { return false; }
    virtual void Connect(const std::string& ip, uint16_t port) {}
    virtual void Disconnect() {}

    virtual uint64_t AddTimer(uint32_t interval, bool always = false);
    virtual void StopTimer(uint64_t timer_id);

    virtual void OnTimer();
    virtual void OnRead(Event* event, uint32_t len = 0) {}
    virtual void OnWrite(Event* event, uint32_t len = 0) {}
    virtual void OnConnect(Event* event, uint16_t err);
    virtual void OnDisConnect(Event* event, uint16_t err) {}

    virtual void SetShutdown() { }
    virtual bool IsShutdown() { return false; }

    virtual std::shared_ptr<BufferQueue> GetReadBuffer() { return nullptr; }

    std::shared_ptr<AlloterWrap> GetAlloter() { return _alloter; }

protected:
    std::shared_ptr<AlloterWrap>     _alloter;
    std::shared_ptr<BlockMemoryPool> _block_pool;
};

std::shared_ptr<RWSocket> MakeRWSocket();
std::shared_ptr<RWSocket> MakeRWSocket(std::shared_ptr<AlloterWrap> alloter);
std::shared_ptr<RWSocket> MakeRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);

}

#endif