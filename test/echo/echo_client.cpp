#include <map>
#include <string>
#include <atomic>
#include <iostream>
#include <algorithm> // for std::find

#include "common/util/time.h"
#include "include/cppnet.h"

using namespace cppnet;

std::atomic_bool __stop(false);
int msg_index = 0;
std::map<uint64_t, cppnet::Handle> handle_map;
static const char* __buf_spilt = "\r\n";


std::string GetMsg() {
    return "It is a test msg, It is a long test msg. index : " + std::to_string(msg_index++) + __buf_spilt;
}

void ReadFunc(Handle handle, cppnet::BufferPtr data, uint32_t len) {
    // print
    char buf[1024] = {0};
    data->Read(buf, 1024);
    std::cout << buf << std::endl;
    cppnet::Sleep(100);
    handle->Write(buf, len);
}

void ConnectFunc(Handle handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    } else {
        handle_map[handle->GetSocket()] = handle;

        auto msg = GetMsg();
        handle->Write(msg.c_str(), (uint32_t)msg.length());
    }
}

void DisConnectionFunc(Handle handle, uint32_t err) {
    __stop = true;
    handle_map.erase(handle->GetSocket());
    if (handle_map.empty()) {
        std::cout << "20 clients all disconnect" << std::endl;
    }
}

int main() {
    cppnet::CppNet net;
    net.Init(1);

    net.SetConnectionCallback(ConnectFunc);
    net.SetReadCallback(ReadFunc);
    net.SetDisconnectionCallback(DisConnectionFunc);
    for (size_t i = 0; i < 200; i++) {
        net.Connection("127.0.0.1", 8921);
    }
    // wait all connect success.
    cppnet::Sleep(20000);

    std::cout << "200 clients all connected" << std::endl;
    
    net.Join();
}
