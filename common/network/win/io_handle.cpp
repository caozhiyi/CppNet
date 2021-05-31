// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <winsock2.h>
#include <WS2tcpip.h>

#include "common/network/io_handle.h"

namespace cppnet {

SysCallInt64Result OsHandle::TcpSocket(bool ipv4) {
    int32_t af = AF_INET6;
    if (ipv4) {
        af = AF_INET;
    }
    int64_t sock = WSASocket(af, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
        return {sock, WSAGetLastError()};
    }

    // both ipv6 and ipv4
    int32_t opt = 0;
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&opt, sizeof(opt)) != 0) {
        return { sock, WSAGetLastError() };
	}
    return {sock, 0};
}

SysCallInt32Result OsHandle::Bind(int64_t sockfd, Address& address) {
    int32_t ret = -1;
    if (address.GetType() == AT_IPV4) {
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(address.GetAddrPort());
		addr.sin_addr.S_un.S_addr = inet_addr(address.GetIp().c_str());
        ret = bind((SOCKET)sockfd, (sockaddr*)&addr, sizeof(addr));

    } else {
        SOCKADDR_IN6 addr;
        addr.sin6_flowinfo = 0;
        addr.sin6_scope_id = 0;
        addr.sin6_family = AF_INET6;
        addr.sin6_port = htons(address.GetAddrPort());
        inet_pton(AF_INET6, address.GetIp().c_str(), &addr.sin6_addr);
        ret = bind((SOCKET)sockfd, (sockaddr*)&addr, sizeof(addr));
    }
    
    if (SOCKET_ERROR == ret) {
        return {ret, (int32_t)GetLastError()};
    }
    return {ret, 0};
}

SysCallInt32Result OsHandle::Listen(int64_t sockfd, uint32_t len) {
    if (len == 0) {
        len = SOMAXCONN;
    }
    
    int32_t ret = listen((SOCKET)sockfd, SOMAXCONN);
    if (SOCKET_ERROR == ret) {
        return {ret, (int32_t)GetLastError()};
    }

    return {ret, 0};
}

SysCallInt32Result OsHandle::Connect(int64_t sockfd, Address& address) {
    return {0, 0};
}

SysCallInt32Result OsHandle::Close(int64_t sockfd) {
    int32_t ret = closesocket((SOCKET)sockfd);
    if (ret != 0) {
        return {ret, (int32_t)GetLastError()};
    }
    return {ret, 0};
}

SysCallInt64Result OsHandle::Accept(int64_t sockfd, Address& address) {
    return {0, 0};
}

SysCallInt32Result OsHandle::Write(int64_t sockfd, const char *data, uint32_t len) {
    return {0, 0};
}

SysCallInt32Result OsHandle::Writev(int64_t sockfd, Iovec *vec, uint32_t vec_len) {
    return {0, 0};
}

SysCallInt32Result OsHandle::Recv(int64_t sockfd, char *data, uint32_t len, uint16_t flag) {
    return {0, 0};
}

SysCallInt32Result OsHandle::Readv(int64_t sockfd, Iovec *vec, uint32_t vec_len) {
    return {0, 0};
}

}