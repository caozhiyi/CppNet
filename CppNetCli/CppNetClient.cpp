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
	std::cout << "[WriteFunc]  len :" << len << std::endl;
}

void ReadFunc(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error, bool& continue_read) {
	std::cout << "[ReadFunc]" << std::endl;
	std::cout << *(data) << std::endl;
    data->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << len << std::endl << std::endl;
    base::CRunnable::Sleep(1000);
	if (error != CEC_CLOSED) {
        auto msg = GetMsg();
		SyncWrite(handle, msg.c_str(), msg.length());
	}
}

void ConnectFunc(const Handle& handle, uint32_t err) {
	std::cout << "[ConnectFunc]" << std::endl;
    auto msg = GetMsg();
    SyncWrite(handle, msg.c_str(), msg.length());
    SyncRead(handle);
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
    cppnet::SyncConnection("192.168.1.4", 8921, msg.c_str(), msg.length());
#else
    cppnet::SyncConnection("192.168.233.128", 8921);
#endif // !__linux__

    cppnet::Join();
}