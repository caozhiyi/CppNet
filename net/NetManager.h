#ifndef HEADER_NET_CPPNETMANAGER
#define HEADER_NET_CPPNETMANAGER

#include <unordered_map>
#include <memory>
#include "Single.h"
#include "CppDefine.h"
#include "PoolSharedPtr.h"

namespace cppnet {
    
    class CSocketImpl;
    class CCppNetImpl;
    class CCppNetManager : public base::CSingle<CCppNetManager> {
    public:
        CCppNetManager();
        ~CCppNetManager();
        // common
        Handle Init(uint32_t thread_num = 1);
        void Dealloc(Handle net_handle = 1);
        void Join(Handle net_handle = 1);
        void AllJoin();

        // set call back
        void SetReadCallback(const read_call_back& func, Handle net_handle = 1);
        void SetWriteCallback(const write_call_back& func, Handle net_handle = 1);
        void SetDisconnectionCallback(const connection_call_back& func, Handle net_handle = 1);

        // about timer
        uint64_t SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false, Handle net_handle = 1);
        void RemoveTimer(uint64_t timer_id, Handle net_handle = 1);

        //server
        void SetAcceptCallback(const connection_call_back& func, Handle net_handle = 1);
        bool ListenAndAccept(const std::string& ip, uint16_t port, Handle net_handle = 1);

        //client
        void SetConnectionCallback(const connection_call_back& func, Handle net_handle = 1);
#ifndef __linux__
        Handle Connection(uint16_t port, std::string ip, const char* buf, uint32_t buf_len, Handle net_handle = 1);
#endif
        Handle Connection(uint16_t port, std::string ip, Handle net_handle = 1);

        base::CMemSharePtr<CSocketImpl> GetSocket(const Handle& handle);

    private:
        std::unordered_map<uint32_t, std::shared_ptr<CCppNetImpl>>  _cppnet_map;
    };
}

#endif