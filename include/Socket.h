#ifndef HEADER_INCLUDE_CSOCKET
#define HEADER_INCLUDE_CSOCKET

#include <string>
#include "CppDefine.h"

namespace cppnet {

    // get socket ip and adress
    int16_t GetIpAddress(const Handle& handle, std::string& ip, uint16_t& port);
    // post sync read event.
    int16_t SyncRead(const Handle& handle);
    // post sync write event.
    int16_t SyncWrite(const Handle& handle, const char* src, int32_t len);

    // post sync read event with time out
    int16_t SyncRead(const Handle& handle, int32_t interval);
    // post sync write event with time out
    int16_t SyncWrite(const Handle& handle, int32_t interval, const char* src, int32_t len);

    // post a sync task to io thread
    int16_t PostTask(std::function<void(void)>& func);
#ifndef __linux__
    // sync connection. 
    int16_t SyncConnection(const std::string& ip, int16_t port, const char* buf, int32_t buf_len);
#endif
    int16_t SyncConnection(const std::string& ip, int16_t port);

    int16_t SyncDisconnection(const Handle& handle);

    int16_t Close(const Handle& handle);
}

#endif