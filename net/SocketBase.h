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

    class CEventActions;
    struct CallBackHandle;
    class CSocketBase {
    public:
        CSocketBase(uint32_t net_index, std::shared_ptr<CallBackHandle>& call_back_handle);
        CSocketBase(std::shared_ptr<CEventActions>& event_actions, uint32_t net_index, std::shared_ptr<CallBackHandle>& call_back_handle);
        virtual  ~CSocketBase();

        void SetEventActions(std::shared_ptr<CEventActions>    &actions) { _event_actions = actions; }
        void SetSocket(const uint32_t& sock) { _sock = sock; }
        uint32_t GetSocket() { return _sock; }
        bool IsInActions() { return _add_event_actions; }
        void SetInActions(bool set) { _add_event_actions = set; }
        const char* GetAddress() const { return _ip; }
        short GetPort() const { return _port; }
        uint32_t GetPoolSize() {return _pool->GetLargeSize(); }
        void ReleasePoolHalf() { _pool->ReleaseLargeHalf(); }

    protected:
        bool            _add_event_actions;
        uint32_t        _sock;
        short           _port;
        char            _ip[__addr_str_len];
        uint32_t        _net_index; // cppnet instance index

        std::shared_ptr<CallBackHandle>       _callback_handle;
        std::shared_ptr<CEventActions>        _event_actions;
        std::shared_ptr<base::CMemoryPool>    _pool;
    };

}
#endif