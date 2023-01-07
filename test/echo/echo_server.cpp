#include <mutex>
#include <string>
#include <thread>
#include <string.h> // for strlen
#include <iostream>

#include "cinclude/c_cppnet.h"

static const int __buf_len = 2048;
static const char* __buf_spilt = "\r\n";

void ReadFunc(SocketHandle handle, BufferHandle data, int len) {
    char msg_buf[__buf_len] = {0};
    int need_len = 0;
    uint32_t find_len = (uint32_t)strlen(__buf_spilt);
    // get recv data to send back.
    uint32_t size  = ReadUntil(data, msg_buf, __buf_len, __buf_spilt, find_len, &need_len);
    Write(handle, msg_buf, size);
}

void ConnectFunc(SocketHandle handle, int error) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    if (error == CEC_CLOSED) {
        std::cout << "remote closed connect : " << GetSocket(handle) << std::endl;
    } else if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    }
}

int main() {

    // start 4 threads
    NetHandle net = Init(4);

    SetAcceptCallback(net, ConnectFunc);
    SetReadCallback(net, ReadFunc);
    SetDisconnectionCallback(net, ConnectFunc);

    ListenAndAccept(net, "0.0.0.0", 8921);

    Join(net);
}