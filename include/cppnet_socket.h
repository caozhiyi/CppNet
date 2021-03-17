#ifndef INCLUDE_SOCKET
#define INCLUDE_SOCKET

#include <memory>
#include <string>
#include "cppnet_type.h"

namespace cppnet {

    class Socket;
    // cpp net socket instance
    class CNSocket {
    public:
        CNSocket();
        ~CNSocket();
        // get socket ip and adress
        int16_t GetAddress(std::string& ip, uint16_t& port);
        // post sync write event.
        int16_t Write(const char* src, int32_t len);
        // close the connect
        int16_t Close();
    };
}

#endif