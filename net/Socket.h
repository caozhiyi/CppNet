#ifndef HEADER_NET_CSOCKET
#define HEADER_NET_CSOCKET

#include <string>
#include <memory>
#include <functional>

#include "PoolSharedPtr.h"
#include "SocketBase.h"

namespace cppnet {
    class CEventHandler;
    class CSocket : public CSocketBase, public base::CEnableSharedFromThis<CSocket> {
    public:
        CSocket(std::shared_ptr<CEventActions>& event_actions);
        ~CSocket();

        // post sync read event.
        void SyncRead();
        // post sync write event.
        void SyncWrite(char* src, int len);

        // post sync read event with time out
        void SyncRead(unsigned int interval);
        // post sync write event with time out
        void SyncWrite(unsigned int interval, char* src, int len);

        // post a sync task to io thread
        void PostTask(std::function<void(void)>& func);
#ifndef __linux__
        // sync connection. 
        void SyncConnection(const std::string& ip, short port, char* buf, int buf_len);
#else
        void SyncConnection(const std::string& ip, short port);
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
        unsigned int					        _post_event_num;
#endif
    };
}

#endif