#include <string>
#include <thread>
#include <string>
#include <iostream>

#include "include/cppnet.h"
#include "common/util/time.h"

using namespace cppnet;

void ReadFunc(Handle handle, std::shared_ptr<Buffer> data, uint32_t len) {
    char msg_buf[128] = {0};
    // get recv data to send back.
    uint32_t size = data->Read(msg_buf, 128);
    
    std::cout << "get message: " << msg_buf << " listen port is: " << handle->GetListenPort() << std::endl;
}

void ConnectFunc(Handle handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::cout << handle->GetListenPort() << " port get connect socket: " << handle->GetSocket() << std::endl;

    } else {
        std::cout << "[ConnectFunc] some thing error : " << err << std::endl;
    }
}


int main() {

    cppnet::CppNet net;
    net.Init(1);

    net.SetAcceptCallback(ConnectFunc);
    net.SetReadCallback(ReadFunc);

    net.ListenAndAccept("::0:0", 8921);
    net.ListenAndAccept("::0:0", 8922);

    net.Join();
}