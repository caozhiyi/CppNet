#include <string>
#include <vector>
#include <iostream>
#include <algorithm> // for std::find

#include "include/cppnet.h"
#include "include/cppnet_socket.h"

#include "common/util/time.h"

using namespace cppnet;

int msg_index = 0;
std::vector<cppnet::Handle> handle_vec;
static const char* __buf_spilt = "\r\n";

std::string GetMsg() {
    return "It is a test msg, It is a long test msg. index : " + std::to_string(msg_index++) + __buf_spilt;
}

void WriteFunc(const Handle& handle, uint32_t len) {
    // do nothing 
}

void ReadFunc(Handle handle, cppnet::BufferPtr data, uint32_t len) {
    // print
    char buf[1024] = {0};
    data->Read(buf, 1024);
    std::cout << buf << std::endl;
}

void ConnectFunc(Handle handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    } else {
        handle_vec.push_back(handle);
    }
}

void DisConnectionFunc(Handle handle, uint32_t err) {
    std::cout << "disconnect : " << handle << std::endl;
    auto iter = std::find(handle_vec.begin(), handle_vec.end(), handle);
    if (iter != handle_vec.end()) {
        handle_vec.erase(iter);
    }
}

int main() {

    cppnet::CppNet net;
    net.Init(1);

    net.SetConnectionCallback(ConnectFunc);
    net.SetWriteCallback(WriteFunc);
    net.SetReadCallback(ReadFunc);
    net.SetDisconnectionCallback(DisConnectionFunc);
    for (size_t i = 0; i < 1000; i++) {
        net.Connection("127.0.0.1", 8921);
    }

    // wait all connect success.
    cppnet::Sleep(5000);

    while (1) {
        // sleep 1s;
        for (auto iter = handle_vec.begin(); iter != handle_vec.end(); ++iter) {
            cppnet::Sleep(1000);
            std::string msg = GetMsg();
            (*iter)->Write(msg.c_str(), msg.length());
        }
    }
    
    net.Join();
}
