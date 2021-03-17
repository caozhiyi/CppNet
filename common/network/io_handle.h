#ifndef COMMON_NETWORK_IO_HANDLE
#define COMMON_NETWORK_IO_HANDLE

#include <cstdint>
#include "address.h"
#include "common/util/os_return.h"

namespace cppnet {

struct Iovec {
    void      *_iov_base;      // starting address of buffer
    size_t    _iov_len;        // size of buffer
    Iovec(void* base, size_t len) : _iov_base(base), _iov_len(len) {}
};

class OsHandle {
public:
static SysCallInt64Result TcpSocket();

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