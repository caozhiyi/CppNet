#include <string>
#include <vector>
#include <iostream>

#include "include/cppnet.h"
#include "common/util/time.h"

using namespace cppnet;

void ConnectFunc(Handle handle, uint32_t err) {
    if (err == CEC_SUCCESS) {
        std::string msg("it is a test message.");
        handle->Write(msg.c_str(), (uint32_t)msg.length());
        handle->Close();

    } else {
        std::cout << " [ConnectFunc] some thing error : " << err << std::endl;
    }
}


int main() {

    cppnet::CppNet net;
    net.Init(1);
    net.SetConnectionCallback(ConnectFunc);

    net.Connection("::0:1", 8921);
    net.Connection("::0:1", 8922);

    Sleep(2000);
}