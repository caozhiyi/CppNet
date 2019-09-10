#include <iostream>
#include <string>
#include <string.h> // for strlen
#include <thread>
#include "CppNet.h"
using namespace cppnet;

static const int __buf_len = 2048;
static const char* __buf_spilt = "\r\n";

void WriteFunc(const Handle& handle, uint32_t len, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while write : " << error << std::endl;
    }
    // do nothing 
}

void ReadFunc(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while read : " << error << std::endl;
        
    } else {
        char msg_buf[__buf_len] = {0};
        int need_len = 0;
        int find_len = strlen(__buf_spilt);
        // get recv data to send back.
        int size = data->ReadUntil(msg_buf, __buf_len, __buf_spilt, find_len, need_len);
        Write(handle, msg_buf, size);
    }
}

void ConnectFunc(const Handle& handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    }
}

int main() {

    // start 4 threads
    cppnet::Init(4);

    cppnet::SetAcceptCallback(ConnectFunc);
    cppnet::SetWriteCallback(WriteFunc);
    cppnet::SetReadCallback(ReadFunc);
    cppnet::SetDisconnectionCallback(ConnectFunc);

    cppnet::ListenAndAccept("0.0.0.0", 8921);

    cppnet::Join();
}