#ifndef HEADER_NET_CSOCKETIMPL
#define HEADER_NET_CSOCKETIMPL

#include <string>
#include <memory>
#include <atomic>
#include <functional>

#include "Socket.h"
#include "SocketBase.h"
#include "PoolSharedPtr.h"

namespace cppnet {
    class CEventHandler;
    class CSocketImpl : public CSocketBase, public base::CEnableSharedFromThis<CSocketImpl> {
    public:
        CSocketImpl(std::shared_ptr<CEventActions>& event_actions);
        ~CSocketImpl();

        // post sync read event.
        void SyncRead();
        // post sync write event.
        void SyncWrite(const char* src, uint32_t len);
        // post a sync task to io thread
        void PostTask(std::function<void(void)>& func);
#ifndef __linux__
        // sync connection. 
        void SyncConnection(const std::string& ip, uint16_t port, const char* buf, uint32_t buf_len);
#endif
        void SyncConnection(const std::string& ip, uint16_t port);

        void SyncDisconnection();

    public:
        friend class CAcceptSocket;
        void Recv(base::CMemSharePtr<CEventHandler>& event);
        void Send(base::CMemSharePtr<CEventHandler>& event);

    public:
        base::CMemSharePtr<CEventHandler>        _read_event;
        base::CMemSharePtr<CEventHandler>        _write_event;
#ifndef __linux__
        //iocp use it save post event num;
        std::atomic<int16_t>                     _post_event_num;
#endif
    };
}

#endif