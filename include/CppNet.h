#ifndef HEADER_INCLUDE_CPPNET
#define HEADER_INCLUDE_CPPNET

#include "PoolSharedPtr.h"
#include "Socket.h"

namespace cppnet {

    typedef std::function<void(base::CMemSharePtr<CSocket>&, int err)> call_back;

    //common
    void Init(int thread_num, bool log = false);
    void Dealloc();

    // thread join
    void Join();

    // must set callback before listen
    void SetReadCallback(const call_back& func);
    void SetWriteCallback(const call_back& func);
    void SetDisconnectionCallback(const call_back& func);

    //timer
    uint64_t SetTimer(unsigned int interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(const call_back& func);
    bool ListenAndAccept(int port, std::string ip);

    //client
    void SetConnectionCallback(const call_back& func);
    base::CMemSharePtr<CSocket> Connection(int port, std::string ip, char* buf, int buf_len);
    base::CMemSharePtr<CSocket> Connection(int port, std::string ip);

}

#endif