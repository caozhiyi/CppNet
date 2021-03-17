#if ((defined __linux__) || (defined __APPLE__)) 

#include <errno.h>
#include <unistd.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "../io_handle.h"


namespace cppnet {

SysCallInt64Result OsHandle::TcpSocket() {
    int64_t sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return {sock, errno};
    }
    return {sock, 0};
}

SysCallInt32Result OsHandle::Bind(int64_t sockfd, Address& address) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.GetPort());
    addr.sin_addr.s_addr = inet_addr(address.GetIp().c_str());

    int32_t ret = bind(sockfd, (sockaddr *)&addr, sizeof(sockaddr));

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
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.GetPort());
    addr.sin_addr.s_addr = inet_addr(address.GetIp().c_str());

    int32_t ret = connect(sockfd, (sockaddr *)&addr, sizeof(addr));

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
    sockaddr_in client_addr;
    socklen_t addr_size = 0;
    int64_t ret = accept(sockfd, (sockaddr*)&client_addr, &addr_size);
    if (ret < 0) {
        return {ret, errno};
    }

    getpeername(ret, (struct sockaddr*)&client_addr, &addr_size);

    address.SetIp(inet_ntoa(client_addr.sin_addr));
    address.SetPort(ntohs(client_addr.sin_port));
    
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

#endif