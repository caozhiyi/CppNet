#ifndef HEADER_NET_CSOCKETBASE
#define HEADER_NET_CSOCKETBASE

#include <memory>
#include "CNConfig.h"

#ifndef __linux__
bool InitScoket();
void DeallocSocket();
#endif
namespace base {
    class CMemoryPool;
}

namespace cppnet {

    class CCppNetImpl;
    class CEventActions;
    class CSocketBase {
    public:
        CSocketBase();
        CSocketBase(std::shared_ptr<CEventActions>& event_actions);
        virtual  ~CSocketBase();

        void SetEventActions(std::shared_ptr<CEventActions>    &actions) { _event_actions = actions; }
        void SetSocket(const uint64_t& sock) { _sock = sock; }
        uint64_t GetSocket() { return _sock; }
        bool IsInActions() { return _add_event_actions; }
        void SetInActions(bool set) { _add_event_actions = set; }
        const char* GetAddress() const { return _ip; }
        short GetPort() const { return _port; }
        uint32_t GetPoolSize() {return _pool->GetLargeSize(); }
        void ReleasePoolHalf() { _pool->ReleaseLargeHalf(); }
        void SetCppnetInstance(std::shared_ptr<CCppNetImpl> ins);
        std::shared_ptr<CCppNetImpl> GetCppnetInstance();

    protected:
        bool            _add_event_actions;
        uint64_t        _sock;
        short           _port;
        char            _ip[__addr_str_len];

        std::weak_ptr<CCppNetImpl>            _cppnet_instance;
        std::shared_ptr<CEventActions>        _event_actions;
        std::shared_ptr<base::CMemoryPool>    _pool;
    };

}
#endif