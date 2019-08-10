#include <iostream>
#include <string>
#include <thread>
#include "CppNet.h"
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

void ReadFunc(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error) {
	std::cout << "[ReadFunc]" << std::endl;
	std::cout << *(data) << std::endl;
    data->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << len << std::endl << std::endl;
	
	if (error != EVENT_ERROR_CLOSED) {
        auto msg = GetMsg();
        SyncWrite(handle, msg.c_str(), msg.length());
	}
    SyncRead(handle);
}

void ConnectFunc(const Handle& handle, uint32_t err) {
	std::cout << "[AcceptFunc]" << std::endl;
	SyncRead(handle);
}

int main() {

	cppnet::Init(1, true);

    cppnet::SetAcceptCallback(ConnectFunc);
    cppnet::SetWriteCallback(WriteFunc);
    cppnet::SetReadCallback(ReadFunc);
    cppnet::SetDisconnectionCallback(ConnectFunc);

    cppnet::ListenAndAccept(8921, "0.0.0.0", 20);

    cppnet::Join();
}