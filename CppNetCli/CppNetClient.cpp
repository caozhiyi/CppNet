#include <iostream>
#include <string>
#include "CppNet.h"
#include "Runnable.h"
#include "Socket.h"
using namespace cppnet;

int index = 0;
std::string msg = "test msg => ";

std::string GetMsg() {
    index++;
    return (msg + std::to_string(index));
}

void WriteFunc(const Handle& handle, uint32_t len, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "[WriteFunc]  something error : " << error << std::endl;
    }
}

void ReadFunc(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error, bool& continue_read) {
	if (error != CEC_CLOSED && error != CEC_CONNECT_BREAK) {
        std::cout << "[ReadFunc]" << std::endl;
	    std::cout << *(data) << std::endl;
        data->Clear();
	    std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	    std::cout << "Read size : " << len << std::endl << std::endl;
        base::CRunnable::Sleep(1000);
        auto msg = GetMsg();
		SyncWrite(handle, msg.c_str(), msg.length());
    } else {
        continue_read = false;
        std::cout << "Close" << std::endl;
    }
}

void ConnectFunc(const Handle& handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::cout << "[ConnectFunc]" << std::endl;
        auto msg = GetMsg();
        SyncWrite(handle, msg.c_str(), msg.length());
        SyncRead(handle);

    } else {
        std::cout << "[ConnectFunc] some thing error : " << err << std::endl;
    }
}

void DisConnectionFunc(const Handle& handle, uint32_t err) {
    std::cout << "[DisConnectionFunc]" << std::endl;
}

int main() {

	cppnet::Init(1, true);

    cppnet::SetConnectionCallback(ConnectFunc);
    cppnet::SetWriteCallback(WriteFunc);
    cppnet::SetReadCallback(ReadFunc);
    cppnet::SetDisconnectionCallback(DisConnectionFunc);

    auto msg = GetMsg();
#ifndef __linux__
    cppnet::SyncConnection("192.168.1.9", 8921, msg.c_str(), msg.length());
#else
    cppnet::SyncConnection("172.21.193.122", 8921);
#endif // !__linux__

    cppnet::Join();
}