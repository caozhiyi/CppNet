#ifndef HEADER_INCLUDE_CPPNET
#define HEADER_INCLUDE_CPPNET

#include "CppDefine.h"
#include "Socket.h"

namespace cppnet {

    // common
    // init cppnet library.
    // thread_num : the number of running threads.
    int32_t Init(int32_t thread_num);
    void Dealloc(int32_t net_handle = 1);

    // thread join
    // net_handle : cppnet instace handle, if only once, default 1
    void Join(int32_t net_handle = 1);

    // must set callback before listen
    // net_handle : cppnet instace handle, if only once, default 1
    void SetReadCallback(const read_call_back& func, int32_t net_handle = 1);
    void SetWriteCallback(const write_call_back& func, int32_t net_handle = 1);
    void SetDisconnectionCallback(const connection_call_back& func, int32_t net_handle = 1);

    //timer
    // net_handle : cppnet instace handle, if only once, default 1
    uint64_t SetTimer(int32_t interval, const timer_call_back& func, void* param = nullptr, bool always = false, int32_t net_handle = 1);
    void RemoveTimer(uint64_t timer_id, int32_t net_handle = 1);

    //server
    // net_handle : cppnet instace handle, if only once, default 1
    void SetAcceptCallback(const connection_call_back& func, int32_t net_handle = 1);
    bool ListenAndAccept(const std::string& ip, int16_t port, int32_t net_handle = 1);

    //client
    // net_handle : cppnet instace handle, if only once, default 1
    void SetConnectionCallback(const connection_call_back& func, int32_t net_handle = 1);
}

#endif