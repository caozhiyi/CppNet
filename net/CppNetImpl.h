#ifndef HEADER_NET_CNETOBJECT
#define HEADER_NET_CNETOBJECT

#include <unordered_map>
#include <thread>
#include <mutex>
#include <string>

#include "PoolSharedPtr.h"
#include "EventHandler.h"
#include "MemaryPool.h"
#include "Timer.h"
#include "Single.h"
#include "CppDefine.h"

namespace cppnet {

    class CEventActions;
    class CSocket;
    class CAcceptSocket;
    class CCppNetImpl : public base::CSingle<CCppNetImpl>
    {
    public:
        CCppNetImpl();
        ~CCppNetImpl();
        // common
        void Init(int thread_num);
        void Dealloc();
        void Join();

        // set call back
        void SetReadCallback(const read_call_back& func);
        void SetWriteCallback(const write_call_back& func);
        void SetDisconnectionCallback(const connection_call_back& func);

        // about timer
        uint64_t SetTimer(unsigned int interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false);
        void RemoveTimer(uint64_t timer_id);

        //server
        void SetAcceptCallback(const connection_call_back& func);
        bool ListenAndAccept(int port, std::string ip);

        //client
        void SetConnectionCallback(const connection_call_back& func);
#ifndef __linux__
        bool Connection(int port, std::string ip, char* buf, int buf_len);
#endif
        bool Connection(int port, std::string ip);

        // get socket
        base::CMemSharePtr<CSocketImpl> GetSocket(const Handle& handle);

    private:
        void _AcceptFunction(base::CMemSharePtr<CAcceptEventHandler>& event, int err);
        void _ReadFunction(base::CMemSharePtr<CEventHandler>& event, int err);
        void _WriteFunction(base::CMemSharePtr<CEventHandler>& event, int err);
        std::shared_ptr<CEventActions>& _RandomGetActions();

    private:
        friend class CSocketImpl;
        read_call_back	        _read_call_back          = nullptr;
        write_call_back	        _write_call_back         = nullptr;
        connection_call_back	_connection_call_back    = nullptr;
        connection_call_back	_disconnection_call_back = nullptr;
        connection_call_back	_accept_call_back        = nullptr;
        
        base::CMemoryPool       _pool;

        std::mutex			    _mutex;
        std::vector<std::shared_ptr<std::thread>>					            _thread_vec;
        std::unordered_map<uint64_t, base::CMemSharePtr<CAcceptSocket>>	        _accept_socket;
        std::unordered_map<uint64_t, base::CMemSharePtr<CSocketImpl>>		    _socket_map;
        std::unordered_map<std::thread::id, std::shared_ptr<CEventActions>>	    _actions_map;
        std::unordered_map<uint64_t, std::weak_ptr<CEventActions>>              _timer_actions_map;
    };

}
#endif