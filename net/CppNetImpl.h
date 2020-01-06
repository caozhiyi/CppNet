#ifndef HEADER_NET_CPPNETIMPL
#define HEADER_NET_CPPNETIMPL

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "Timer.h"
#include "CppDefine.h"
#include "MemoryPool.h"
#include "EventHandler.h"
#include "PoolSharedPtr.h"

namespace cppnet {

    class CEventActions;
    class CSocket;
    class CAcceptSocket;
    struct CallBackHandle;
    class CCppNetImpl {
    public:
        CCppNetImpl(uint32_t net_index);
        ~CCppNetImpl();
        // common
        void Init(uint32_t thread_num);
        void Dealloc();
        void Join();

        // set call back
        void SetReadCallback(const read_call_back& func);
        void SetWriteCallback(const write_call_back& func);
        void SetDisconnectionCallback(const connection_call_back& func);

        // about timer
        uint64_t SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false);
        void RemoveTimer(uint64_t timer_id);

        //server
        void SetAcceptCallback(const connection_call_back& func);
        bool ListenAndAccept(const std::string& ip, uint16_t port);

        //client
        void SetConnectionCallback(const connection_call_back& func);
#ifndef __linux__
        Handle Connection(uint16_t port, std::string ip, const char* buf, uint32_t buf_len);
#endif
        Handle Connection(uint16_t port, std::string ip);

        // get socket
        base::CMemSharePtr<CSocketImpl> GetSocket(const uint32_t& handle);

        // get thread number
        uint32_t GetThreadNum();

    private:
        void _AcceptFunction(base::CMemSharePtr<CSocketImpl>& sock, uint32_t err);
        void _ReadFunction(base::CMemSharePtr<CEventHandler>& event, uint32_t err);
        void _WriteFunction(base::CMemSharePtr<CEventHandler>& event, uint32_t err);
        std::shared_ptr<CEventActions>& _RandomGetActions();

    private:
        uint32_t                _net_index; // cppnet instance index
        base::CMemoryPool       _pool;
        std::mutex              _mutex;
        
        read_call_back          _read_call_back = nullptr;
        write_call_back         _write_call_back = nullptr;
        connection_call_back    _connection_call_back = nullptr;
        connection_call_back    _disconnection_call_back = nullptr;
        connection_call_back    _accept_call_back = nullptr;

        std::shared_ptr<CallBackHandle>                                          _callback_handle;
        std::vector<std::shared_ptr<std::thread>>                                _thread_vec;
        std::unordered_map<uint32_t, base::CMemSharePtr<CAcceptSocket>>          _accept_socket;
        std::unordered_map<uint32_t, base::CMemSharePtr<CSocketImpl>>            _socket_map;
        std::unordered_map<std::thread::id, std::shared_ptr<CEventActions>>      _actions_map;
        std::unordered_map<uint64_t, std::weak_ptr<CEventActions>>               _timer_actions_map;
    };

}
#endif