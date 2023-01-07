#include <map>
#include <string>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm> // for std::find

#include "cinclude/c_cppnet.h"


std::atomic_bool __stop(false);
int msg_index = 0;
std::map<uint64_t, SocketHandle> handle_map;
static const char* __buf_spilt = "\r\n";


std::string GetMsg() {
    return "It is a test msg, It is a long test msg. index : " + std::to_string(msg_index++) + __buf_spilt;
}

void ReadFunc(SocketHandle handle, BufferHandle data, int len) {
    // print
    char buf[1024] = {0};
    Read(data, buf, 1024);
    std::cout << buf << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    Write(handle, buf, len);
}

void ConnectFunc(SocketHandle handle, int error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    } else {
        handle_map[GetSocket(handle)] = handle;

        auto msg = GetMsg();
        Write(handle, msg.c_str(), (uint32_t)msg.length());
    }
}

void DisConnectionFunc(SocketHandle handle, int err) {
    __stop = true;
    handle_map.erase(GetSocket(handle));
    if (handle_map.empty()) {
        std::cout << "20 clients all disconnect" << std::endl;
    }
}

int main() {
    NetHandle net = Init(1);

    SetConnectionCallback(net, ConnectFunc);
    SetReadCallback(net, ReadFunc);
    SetDisconnectionCallback(net, DisConnectionFunc);
    for (size_t i = 0; i < 200; i++) {
        Connection(net, "127.0.0.1", 8921);
    }
    // wait all connect success.
    std::this_thread::sleep_for(std::chrono::milliseconds(20000));

    std::cout << "200 clients all connected" << std::endl;
    
    Join(net);
}
