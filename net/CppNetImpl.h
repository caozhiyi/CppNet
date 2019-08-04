#ifndef HEADER_NET_CNETOBJECT
#define HEADER_NET_CNETOBJECT

#include <map>
#include <thread>
#include <mutex>
#include <string>

#include "PoolSharedPtr.h"
#include "EventHandler.h"
#include "MemaryPool.h"
#include "Timer.h"
#include "Single.h"

namespace cppnet {
    typedef std::function<void(base::CMemSharePtr<CSocket>&, int err)> call_back;

    class CEventActions;
    class CSocket;
    class CAcceptSocket;
    class CCppNetImpl : public base::CSingle<CCppNetImpl>
    {
    public:
        CCppNetImpl();
        ~CCppNetImpl();
        //common
        void Init(int thread_num);
        void Dealloc();
        void Join();

        void SetReadCallback(const call_back& func);
        void SetWriteCallback(const call_back& func);
        void SetDisconnectionCallback(const call_back& func);
        uint64_t SetTimer(unsigned int interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false);
        void RemoveTimer(uint64_t timer_id);

        //server
        void SetAcceptCallback(const call_back& func);
        bool ListenAndAccept(int port, std::string ip);

        //client
        void SetConnectionCallback(const call_back& func);

        base::CMemSharePtr<CSocket> Connection(int port, std::string ip, char* buf, int buf_len);
        base::CMemSharePtr<CSocket> Connection(int port, std::string ip);

    private:
        void _AcceptFunction(base::CMemSharePtr<CAcceptEventHandler>& event, int err);
        void _ReadFunction(base::CMemSharePtr<CEventHandler>& event, int err);
        void _WriteFunction(base::CMemSharePtr<CEventHandler>& event, int err);
        std::shared_ptr<CEventActions>& _RandomGetActions();

    private:
        call_back	_read_call_back          = nullptr;
        call_back	_write_call_back         = nullptr;
        call_back	_connection_call_back    = nullptr;
        call_back	_disconnection_call_back = nullptr;
        call_back	_accept_call_back        = nullptr;
        base::CMemoryPool _pool;

        std::mutex			_mutex;
        std::vector<std::shared_ptr<std::thread>>					_thread_vec;
        std::map<unsigned int, base::CMemSharePtr<CAcceptSocket>>	_accept_socket;
        std::map<unsigned int, base::CMemSharePtr<CSocket>>			_socket_map;
        std::map<std::thread::id, std::shared_ptr<CEventActions>>	_actions_map;
        std::map<uint64_t, std::weak_ptr<CEventActions>>            _timer_actions_map;
    };

}
#endif