// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_WIN_READ_WRITE_SOCKET
#define CPPNET_SOCKET_WIN_READ_WRITE_SOCKET

#include "../rw_socket.h"

namespace cppnet {

class WinRWSocket:
    public RWSocket { 

public:
    WinRWSocket(std::shared_ptr<AlloterWrap> alloter);
    WinRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);
    virtual ~WinRWSocket();

    virtual void Read();
    virtual bool Write(const char* src, uint32_t len);
    virtual void Connect(const std::string& ip, uint16_t port);
    virtual void Disconnect();

    virtual void OnRead(uint32_t len = 0);
    virtual void OnWrite(uint32_t len = 0);

private:
    bool Recv(uint32_t len);
    bool Send(uint32_t len = 0);
};

}

#endif