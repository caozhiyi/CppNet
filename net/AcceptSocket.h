#ifndef HEADER_NET_CACCEPTSOCKET
#define HEADER_NET_CACCEPTSOCKET

#include <string>
#include <memory>
#include <functional>

#include "Buffer.h"
#include "SocketBase.h"
#include "PoolSharedPtr.h"

namespace cppnet {
    class CAcceptEventHandler;

    class CAcceptSocket : public CSocketBase, public base::CEnableSharedFromThis<CAcceptSocket> {
    public:
        CAcceptSocket(std::shared_ptr<CEventActions>& event_actions);
        ~CAcceptSocket();

        bool Bind(uint16_t port, const std::string& ip = "");

        bool Listen();

        void SyncAccept();

    public:
        void _Accept(base::CMemSharePtr<CAcceptEventHandler>& event);

    public:
        base::CMemSharePtr<CAcceptEventHandler>		_accept_event;
    };
}
#endif