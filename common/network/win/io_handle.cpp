// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <winsock2.h>

#include "../io_handle.h"


namespace cppnet {

SysCallInt64Result OsHandle::TcpSocket() {
    int64_t sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sock == INVALID_SOCKET) {
        return {sock, WSAGetLastError()};
    }
    return {sock, 0};
}

SysCallInt32Result OsHandle::Bind(int64_t sockfd, Address& address) {
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.GetPort());
    addr.sin_addr.S_un.S_addr = inet_addr(address.GetIp().c_str());

    int32_t ret = bind((SOCKET)sockfd, (sockaddr*)&addr, sizeof(addr));
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