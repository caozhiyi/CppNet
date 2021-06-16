// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_WINDOWS_READ_WRITE_SOCKET
#define CPPNET_SOCKET_WINDOWS_READ_WRITE_SOCKET

#include "cppnet/socket/rw_socket.h"

namespace cppnet {

class Event;
class PosixRWSocket:
    public RWSocket { 

public:
    PosixRWSocket();
    PosixRWSocket(std::shared_ptr<AlloterWrap> alloter);
    PosixRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);
    virtual ~PosixRWSocket();

    virtual void Read();
    virtual bool Write(const char* src, uint32_t len);
    virtual void Connect(const std::string& ip, uint16_t port);
    virtual void Disconnect();

    virtual void OnRead(Event* event, uint32_t len = 0);
    virtual void OnWrite(Event* event, uint32_t len = 0);
    virtual void OnDisConnect(Event* event, uint16_t err);

private:
    bool Recv(uint32_t len);
    bool Send();

private:
    Event* _event;
    std::shared_ptr<BufferQueue> _write_buffer;
    std::shared_ptr<BufferQueue> _read_buffer;
};

}

#endif