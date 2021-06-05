#include <string>
#include <thread>
#include <string>
#include <iostream>

#include "include/cppnet.h"
#include "common/util/time.h"

using namespace cppnet;

int msg_index = 0;
std::string msg = "test msg => ";

std::string GetMsg() {
    msg_index++;
    return (msg + std::to_string(msg_index));
}

void WriteFunc(Handle handle, uint32_t len) {
    std::cout << "[WriteFunc]  length : " << len << std::endl;
}

void ReadFunc(Handle handle, std::shared_ptr<Buffer> data, uint32_t len) {
    std::cout << "[ReadFunc]" << std::endl;

    char buf[1024] = {0};
    data->Read(buf, 1024);
    std::cout << "recv :"<< buf << std::endl;
    data->Clear();

    std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
    std::cout << "Read size : " << len << std::endl << std::endl;

    auto msg = GetMsg();
    handle->Write(msg.c_str(), (uint32_t)msg.length());
}

void ConnectFunc(Handle handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::string ip;
        uint16_t port;
        handle->GetAddress(ip, port);
        std::cout << "[ConnectFunc] : ip: " << ip << "port: " << port << "listen port: " << handle->GetListenPort() << std::endl;

        auto msg = GetMsg();
        handle->Write(msg.c_str(), (uint32_t)msg.length());

    } else {
        std::cout << "[ConnectFunc] some thing error : " << err << std::endl;
    }
}

void DisConnectionFunc(Handle handle, uint32_t err) {
    std::cout << "[DisConnectionFunc] : " << err << std::endl;
}

int main() {

    cppnet::CppNet net;
    net.Init(1);

    net.SetAcceptCallback(ConnectFunc);
    net.SetWriteCallback(WriteFunc);
    net.SetReadCallback(ReadFunc);
    net.SetDisconnectionCallback(DisConnectionFunc);

    net.ListenAndAccept("::0:0", 8999);

    net.Join();
}