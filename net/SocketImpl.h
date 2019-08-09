#ifndef HEADER_NET_CSOCKETIMPL
#define HEADER_NET_CSOCKETIMPL

#include <string>
#include <memory>
#include <functional>

#include "PoolSharedPtr.h"
#include "SocketBase.h"
#include "Socket.h"

namespace cppnet {
    class CEventHandler;
    class CSocketImpl : public CSocketBase, public base::CEnableSharedFromThis<CSocketImpl> {
    public:
        CSocketImpl(std::shared_ptr<CEventActions>& event_actions);
        ~CSocketImpl();

        // post sync read event.
        void SyncRead();
        // post sync write event.
        void SyncWrite(char* src, uint32_t len);

        // post sync read event with time out
        void SyncRead(uint32_t interval);
        // post sync write event with time out
        void SyncWrite(uint32_t interval, char* src, uint32_t len);

        // post a sync task to io thread
        void PostTask(std::function<void(void)>& func);
#ifndef __linux__
        // sync connection. 
        void SyncConnection(const std::string& ip, uint16_t port, char* buf, uint32_t buf_len);
#else
        void SyncConnection(const std::string& ip, uint16_t port);
#endif
        void SyncDisconnection();

        // set read and write event call back
        void SetReadCallBack(const std::function<void(base::CMemSharePtr<CEventHandler>&, int error)>& call_back);
        void SetWriteCallBack(const std::function<void(base::CMemSharePtr<CEventHandler>&, int error)>& call_back);

        friend bool operator>(const CSocketBase& s1, const CSocketBase& s2);
        friend bool operator<(const CSocketBase& s1, const CSocketBase& s2);
        friend bool operator==(const CSocketBase& s1, const CSocketBase& s2);
        friend bool operator!=(const CSocketBase& s1, const CSocketBase& s2);

    public:
        void _Recv(base::CMemSharePtr<CEventHandler>& event);
        void _Send(base::CMemSharePtr<CEventHandler>& event);

    public:
        base::CMemSharePtr<CEventHandler>		_read_event;
        base::CMemSharePtr<CEventHandler>		_write_event;
#ifndef __linux__
        //iocp use it save post event num;
        uint32_t					        _post_event_num;
#endif
    };
    bool operator>(const CSocketBase& s1, const CSocketBase& s2);
    bool operator<(const CSocketBase& s1, const CSocketBase& s2);
    bool operator==(const CSocketBase& s1, const CSocketBase& s2);
    bool operator!=(const CSocketBase& s1, const CSocketBase& s2);
}

#endif