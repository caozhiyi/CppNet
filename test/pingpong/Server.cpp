#include <atomic>
#include <iostream>
#include "CppNet.h"

using namespace cppnet;

#ifndef __linux__
#include <winsock2.h>
void SetNoDelay(const uint64_t& sock) {
    int opt = 1;
    int ret = setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (const char*)&opt, sizeof(opt));
}
#else
#include <netinet/tcp.h>
#include <netinet/in.h>
void SetNoDelay(const uint64_t& sock) {
    int optval = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof(optval)));
}
#endif

static std::atomic_int count;

void OnConnection(const Handle& handle, uint32_t error) {

    count++;
    if (error == CEC_SUCCESS) {
        std::cout << " accept a socket. count: " << count << std::endl;
        SetNoDelay(handle);
    }
}

void OnMessage(const Handle& handle, base::CBuffer* data, uint32_t, uint32_t error) {
    char buff[65535];
    if (error == CEC_SUCCESS) {
        while (data->GetCanReadLength()) {
           int ret = data->Read(buff, 65535);
           Write(handle, buff, ret);
        }

    } else {
        std::cout << " something error while reading. err : " << error << std::endl;
    }
}

int main() {
    count = 0;
    cppnet::Init(4);

    cppnet::SetAcceptCallback(OnConnection);
    cppnet::SetReadCallback(OnMessage);

    cppnet::ListenAndAccept("0.0.0.0", 8921);

    cppnet::Join();
}

