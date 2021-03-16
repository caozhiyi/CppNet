#ifndef INCLUDE_CSOCKET
#define INCLUDE_CSOCKET

#include <memory>
#include <string>
#include "CppDefine.h"

namespace cppnet {

    class CCppNetImpl;
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
    private:
        friend class CCppNetImpl;
        std::weak_ptr<CCppNetImpl> _cppnet_instance;
        uint64_t                   _socket_handle;
    };
}

#endif