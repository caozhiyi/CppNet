// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../io_handle.h"


namespace cppnet {

SysCallInt64Result OsHandle::TcpSocket(bool ipv4) {
    int domain = PF_INET6;
    if (ipv4) {
        domain = PF_INET;
    }

    int64_t sock = socket(domain, SOCK_STREAM, 0);
    if (sock < 0) {
        return {sock, errno};
    }
    return {sock, 0};
}

SysCallInt32Result OsHandle::Bind(int64_t sockfd, Address& address) {
    int32_t ret = -1;
    if (address.GetType() == AT_IPV4) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(address.GetAddrPort());
        addr.sin_addr.s_addr = inet_addr(address.GetIp().c_str());
        ret = bind(sockfd, (sockaddr *)&addr, sizeof(sockaddr_in));

    } else {
        struct sockaddr_in6 addr;
        addr.sin6_flowinfo = 0;
        addr.sin6_scope_id = 0;
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(address.GetAddrPort());
        inet_pton(AF_INET6, address.GetIp().c_str(), &addr.sin6_addr);
        ret = bind(sockfd, (sockaddr *)&addr, sizeof(sockaddr_in6));
    }

    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Listen(int64_t sockfd, uint32_t len) {
    if (len == 0) {
        len = SOMAXCONN;
    }
    
    int32_t ret = listen(sockfd, len);
    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Connect(int64_t sockfd, Address& address) {
    int32_t ret = -1;
    if (address.GetType() == AT_IPV4) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(address.GetAddrPort());
        addr.sin_addr.s_addr = inet_addr(address.GetIp().c_str());
        ret = connect(sockfd, (sockaddr *)&addr, sizeof(addr));

    } else {
        struct sockaddr_in6 addr;
        addr.sin6_flowinfo = 0;
        addr.sin6_scope_id = 0;
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(address.GetAddrPort());
        inet_pton(AF_INET6, address.GetIp().c_str(), &addr.sin6_addr);
        ret = connect(sockfd, (sockaddr *)&addr, sizeof(addr));
    }

    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Close(int64_t sockfd) {
    int32_t ret = close(sockfd);

    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt64Result OsHandle::Accept(int64_t sockfd, Address& address) {
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int64_t ret = accept(sockfd, (sockaddr*)&client_addr, &addr_size);
    if (ret < 0) {
        return {ret, errno};
    }

    struct sockaddr* addr_pt = (struct sockaddr*)&client_addr;

    void *addr = nullptr;
    switch (addr_pt->sa_family) {
        case AF_INET:
            addr = &((struct sockaddr_in *)addr_pt)->sin_addr;
            address.SetAddrPort(ntohs(((struct sockaddr_in *)addr_pt)->sin_port));
            address.SetType(AT_IPV4);
            break;
        case AF_INET6:
            addr = &((struct sockaddr_in6 *)addr_pt)->sin6_addr;
            address.SetAddrPort((((struct sockaddr_in6 *)addr_pt)->sin6_port));
            address.SetType(AT_IPV6);
        break;
        default:
            return {-1, errno};
    }

    char str_addr[INET6_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET6, addr, str_addr, sizeof(str_addr));
    address.SetIp(str_addr);
    
    return {ret, 0};
}

SysCallInt32Result OsHandle::Write(int64_t sockfd, const char *data, uint32_t len) {
    int32_t ret = write(sockfd, data, len);
    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Writev(int64_t sockfd, Iovec *vec, uint32_t vec_len) {
    int32_t ret = writev(sockfd, (iovec*)vec, vec_len);
    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Recv(int64_t sockfd, char *data, uint32_t len, uint16_t flag) {
    int32_t ret = recv(sockfd, data, len, flag);
    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Readv(int64_t sockfd, Iovec *vec, uint32_t vec_len) {
    int32_t ret = readv(sockfd, (iovec*)vec, vec_len);
    if (ret < 0) {
        return {ret, errno};
    }
    return {ret, 0};
}

}