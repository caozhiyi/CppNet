#ifndef __linux__
#ifndef HEADER_NET_WIN_IOCP
#define HEADER_NET_WIN_IOCP

#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")

#include "EventActions.h"
#include "PoolSharedPtr.h"
#include "CNConfig.h"

namespace cppnet {
    class Cevent;
    struct EventOverlapped {
        OVERLAPPED  _overlapped;
        WSABUF      _wsa_buf;
        int         _event_flag_set;
        char        _lapped_buffer[__iocp_buff_size];
        void*       _event;

        EventOverlapped() {
            _event_flag_set = 0;
            memset(&_overlapped, 0, sizeof(_overlapped));
            memset(_lapped_buffer, 0, __iocp_buff_size);
            _wsa_buf.buf = _lapped_buffer;
            _wsa_buf.len = __iocp_buff_size;
        }

        ~EventOverlapped() {

        }

        void Clear() {
            _event_flag_set = 0;
            memset(_lapped_buffer, 0, __iocp_buff_size);
        }
    };

    class CIOCP : public CEventActions
    {
    public:
        CIOCP();
        ~CIOCP();

        virtual bool Init(uint32_t thread_num = 0);
        virtual bool Dealloc();

        // timer event
        virtual uint64_t AddTimerEvent(uint32_t interval, const std::function<void(void*)>& call_back, void* param, bool always = false);
        virtual bool AddTimerEvent(uint32_t interval, base::CMemSharePtr<CEventHandler>& event);
        virtual bool RemoveTimerEvent(uint64_t timer_id);

        // net io event
        virtual bool AddSendEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool AddRecvEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event);
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, const char* buf, uint32_t buf_len);
        virtual bool AddDisconnection(base::CMemSharePtr<CEventHandler>& event);
        virtual bool DelEvent(base::CMemSharePtr<CEventHandler>& event);

        // io thread process
        virtual void ProcessEvent();
        // post a task to net io thread
        virtual void PostTask(std::function<void(void)>& task);
        // weak up net io thread
        virtual void WakeUp();

    private:
        bool _PostRecv(base::CMemSharePtr<CEventHandler>& event);
        bool _PostAccept(base::CMemSharePtr<CAcceptEventHandler>& event);
        bool _PostSend(base::CMemSharePtr<CEventHandler>& event);
        bool _PostConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, const char* buf, uint32_t buf_len);
        bool _PostDisconnection(base::CMemSharePtr<CEventHandler>& event);

        void _DoTimeoutEvent(std::vector<base::CMemSharePtr<CTimerEvent>>& timer_vec);
        void _DoEvent(EventOverlapped *socket_context, uint32_t bytes);
        void _DoTaskList();

        bool _AddToActions(base::CMemSharePtr<CSocketImpl>& socket);
    private:
        HANDLE                _iocp_handler;
        bool                  _is_inited;
        std::mutex            _mutex;
        std::atomic_bool      _run;
        std::vector<std::function<void(void)>> _task_list;
    };
}
#endif
#endif