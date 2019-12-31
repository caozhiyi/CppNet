#ifndef HEADER_INCLUDE_CSOCKET
#define HEADER_INCLUDE_CSOCKET

#include <string>
#include "CppDefine.h"

namespace cppnet {

    // get socket ip and adress
    int16_t GetIpAddress(const Handle& handle, std::string& ip, uint16_t& port);
    // post sync write event.
    int16_t Write(const Handle& handle, const char* src, int32_t len);
#ifndef __linux__
    // sync connection. 
    int16_t Connection(const std::string& ip, int16_t port, const char* buf, int32_t buf_len, int32_t handle = 1);
#endif
    int16_t Connection(const std::string& ip, int16_t port, int32_t handle = 1);

    int16_t Close(const Handle& handle);
}

#endif