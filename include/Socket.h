#ifndef HEADER_INCLUDE_CSOCKET
#define HEADER_INCLUDE_CSOCKET

#include "CppDefine.h"

namespace cppnet {

    // post sync read event.
    int16_t SyncRead(const Handle& handle);
    // post sync write event.
    int16_t SyncWrite(const Handle& handle, char* src, int32_t len);

    // post sync read event with time out
    int16_t SyncRead(const Handle& handle, int32_t interval);
    // post sync write event with time out
    int16_t SyncWrite(const Handle& handle, int32_t interval, char* src, int32_t len);

    // post a sync task to io thread
    int16_t PostTask(std::function<void(void)>& func);
#ifndef __linux__
    // sync connection. 
    int16_t SyncConnection(const std::string& ip, int16_t port, char* buf, int32_t buf_len);
#else
    int16_t SyncConnection(const std::string& ip, int16_t port);
#endif
    int16_t SyncDisconnection();

}

#endif