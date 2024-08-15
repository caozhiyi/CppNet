#include <atomic>
#include <iostream>

#include "include/cppnet.h"

#ifdef __win__
#include <winsock2.h>
void SetNoDelay(const uint64_t& sock) {
    int opt = 1;
    int ret = setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (const char*)&opt, sizeof(opt));
}
#else
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
void SetNoDelay(const uint64_t& sock) {
    int optval = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof(optval)));
}
#endif

void OnConnection(const cppnet::Handle& handle, uint32_t error) {
    if (error == cppnet::CEC_SUCCESS) {
        SetNoDelay(handle->GetSocket());
    }
}

void OnMessage(const cppnet::Handle& handle, cppnet::BufferPtr data, uint32_t) {
    char buff[65535];
    
    while (data->GetCanReadLength()) {
        int ret = data->Read(buff, 65535);
        handle->Write(buff, ret);
    }
}

int main() {
    cppnet::CppNet net;
    net.Init(4);

    net.SetAcceptCallback(OnConnection);
    net.SetReadCallback(OnMessage);

    net.ListenAndAccept("0.0.0.0", 8921);

    net.Join();
}

