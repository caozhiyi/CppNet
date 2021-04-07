// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_SOCKET_POSIX_READ_WRITE_SOCKET
#define CPPNET_SOCKET_POSIX_READ_WRITE_SOCKET

#include "../rw_socket.h"

namespace cppnet {

class PosixRWSocket:
    public RWSocket { 

public:
    PosixRWSocket(std::shared_ptr<AlloterWrap> alloter);
    PosixRWSocket(uint64_t sock, std::shared_ptr<AlloterWrap> alloter);
    virtual ~PosixRWSocket();

    virtual bool Write(const char* src, uint32_t len);

    virtual void OnRead(uint32_t len = 0);
    virtual void OnWrite(uint32_t len = 0);

private:
    bool Recv(uint32_t len);
    bool Send();
};

}

#endif