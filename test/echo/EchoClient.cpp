#include <iostream>
#include <string>
#include <algorithm> // for std::find
#include "CppNet.h"
#include "Runnable.h"
#include "Socket.h"
using namespace cppnet;

int index = 0;
std::vector<Handle> handle_vec;
static const char* __buf_spilt = "\r\n";

std::string GetMsg() {
    return "It is a test msg, It is a long test msg. index : " + std::to_string(index++) + __buf_spilt;
}

void WriteFunc(const Handle& handle, uint32_t len, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while write : " << error << std::endl;
    }
    // do nothing 
}

void ReadFunc(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error, bool& continue_read) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while read : " << error << std::endl;
    } else {
        // print
	    std::cout << *(data) << std::endl;
        data->Clear();
    }
}

void ConnectFunc(const Handle& handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    } else {
        handle_vec.push_back(handle);
    }
}

void DisConnectionFunc(const Handle& handle, uint32_t err) {
    std::cout << "disconnect : " << handle << std::endl;
    auto iter = std::find(handle_vec.begin(), handle_vec.end(), handle);
    if (iter != handle_vec.end()) {
        handle_vec.erase(iter);
    }
}

int main() {

	cppnet::Init(1, false);

    cppnet::SetConnectionCallback(ConnectFunc);
    cppnet::SetWriteCallback(WriteFunc);
    cppnet::SetReadCallback(ReadFunc);
    cppnet::SetDisconnectionCallback(DisConnectionFunc);
    for (size_t i = 0; i < 10000; i++) {
#ifndef __linux__
        std::string msg = GetMsg();
        cppnet::Connection("192.168.1.9", 8921, msg.c_str(), msg.length());
#else
        cppnet::Connection("192.168.233.128", 8921);
#endif // !__linux__
    }

    // wait all connect success.
    base::CRunnable::Sleep(5000);

    while (1) {
        // sleep 1s;
        for (auto iter = handle_vec.begin(); iter != handle_vec.end(); ++iter) {
            base::CRunnable::Sleep(1);
            std::string msg = GetMsg();
            cppnet::Write(*iter, msg.c_str(), msg.length());
        }
    }
    
    cppnet::Join();
}