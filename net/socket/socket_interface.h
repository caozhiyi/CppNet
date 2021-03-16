#ifndef NET_SOCKET_SOCKET_INTERFACE
#define NET_SOCKET_SOCKET_INTERFACE

#include <memory>

namespace cppnet {

    static const uint8_t __addr_buf_len = sizeof("xxx.xxx.xxx.xxx");

    class SocketInterface { 
    public:
        SocketInterface();
        SocketInterface(std::shared_ptr<CEventActions>& event_actions);
        virtual  ~SocketInterface();

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
        char            _ip[__addr_buf_len];

        std::weak_ptr<CCppNetImpl>            _cppnet_instance;
        std::shared_ptr<CEventActions>        _event_actions;
        std::shared_ptr<base::CMemoryPool>    _pool;
    };

}
#endif