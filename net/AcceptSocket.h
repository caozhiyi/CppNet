#ifndef HEADER_NET_CACCEPTSOCKET
#define HEADER_NET_CACCEPTSOCKET
#include <string>
#include <memory>
#include <functional>
#include "Buffer.h"
#include "PoolSharedPtr.h"
#include "SocketBase.h"

namespace cppnet {
    class CEventHandler;
    class CAcceptEventHandler;
    class CMemoryPool;

    class CAcceptSocket : public CSocketBase, public base::CEnableSharedFromThis<CAcceptSocket> {
    public:
        CAcceptSocket(std::shared_ptr<CEventActions>& event_actions);
        ~CAcceptSocket();

        bool Bind(short port, const std::string& ip = "");

        bool Listen(unsigned int listen_size);

        void SyncAccept();

        void SetReadCallBack(const std::function<void(base::CMemSharePtr<CEventHandler>&, int error)>& call_back);
        void SetAcceptCallBack(const std::function<void(base::CMemSharePtr<CAcceptEventHandler>&, int error)>& call_back);

    public:
        void _Accept(base::CMemSharePtr<CAcceptEventHandler>& event);

    public:
        base::CMemSharePtr<CAcceptEventHandler>		_accept_event;
    };
}
#endif