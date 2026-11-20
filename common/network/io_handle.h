// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_NETWORK_IO_HANDLE
#define COMMON_NETWORK_IO_HANDLE

#include <cstdint>
#include <errno.h>
#include "common/util/os_return.h"
#include "common/network/address.h"

// Winsock error codes, in case winsock2.h is not in this header's include chain.
#ifdef __win__
#ifndef WSAEINTR
#define WSAEINTR 10004
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK 10035
#endif
#ifndef WSAEINPROGRESS
#define WSAEINPROGRESS 10036
#endif
#endif

namespace cppnet {

// Classify an OS error captured in SysCallResult::_errno by the OsHandle
// wrappers: POSIX implementations store errno, winsock ones store
// WSAGetLastError(). Callers must pass the captured _errno, never the global
// errno macro, which winsock calls do not set.
inline bool WouldBlock(int32_t err) {
#ifdef __win__
    return err == WSAEWOULDBLOCK || err == WSAEINPROGRESS;
#else
    return err == EAGAIN || err == EWOULDBLOCK;
#endif
}

inline bool Interrupted(int32_t err) {
#ifdef __win__
    return err == WSAEINTR;
#else
    return err == EINTR;
#endif
}

struct Iovec {
#ifdef __win__
    size_t    _iov_len;       // size of buffer
    void*     _iov_base;      // starting address of buffer
#else
    void*     _iov_base;      // starting address of buffer
    size_t    _iov_len;       // size of buffer
#endif
    Iovec(void* base, size_t len): _iov_base(base), _iov_len(len) {}
};

class OsHandle {
public:
static SysCallInt64Result TcpSocket(bool ipv4 = false);

static SysCallInt32Result Bind(int64_t sockfd, Address& addr);

static SysCallInt32Result Listen(int64_t sockfd, uint32_t len = 0);

static SysCallInt32Result Connect(int64_t sockfd, Address& addr);

static SysCallInt32Result Close(int64_t sockfd);

static SysCallInt64Result Accept(int64_t sockfd, Address& address);

static SysCallInt32Result Write(int64_t sockfd, const char *data, uint32_t len);
static SysCallInt32Result Writev(int64_t sockfd, Iovec *vec, uint32_t vec_len);

static SysCallInt32Result Recv(int64_t sockfd, char *data, uint32_t len, uint16_t flag);
static SysCallInt32Result Readv(int64_t sockfd, Iovec *vec, uint32_t vec_len);

};

}

#endif