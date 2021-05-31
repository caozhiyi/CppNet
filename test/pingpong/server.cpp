#include <atomic>
#include <iostream>

#include "include/cppnet.h"

using namespace cppnet;

#ifdef __win__
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
        //SetNoDelay(handle);
    }
}

void OnMessage(const Handle& handle, std::shared_ptr<cppnet::Buffer> data, uint32_t) {
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

