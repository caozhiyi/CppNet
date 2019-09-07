#ifdef __linux__

#ifndef HEADER_NET_LINUX_CEPOOL
#define HEADER_NET_LINUX_CEPOOL
#include <sys/epoll.h>
#include "EventActions.h"

#define MAX_BUFFER_LEN        8192

namespace cppnet {
    class Cevent;
    class CEpoll : public CEventActions
    {
    public:
        CEpoll(bool per_epoll);
        ~CEpoll();
        // thread_num not useful
        virtual bool Init(uint32_t thread_num = 0);
        virtual bool Dealloc();

        // timer event
        virtual uint64_t AddTimerEvent(uint32_t interval, const timer_call_back& call_back, void* param, bool always = false);
        virtual bool AddTimerEvent(uint32_t interval, base::CMemSharePtr<CEventHandler>& event);
        virtual bool RemoveTimerEvent(uint64_t timer_id);

        // net io event
        virtual bool AddSendEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool AddRecvEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event);
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port);
        virtual bool AddDisconnection(base::CMemSharePtr<CEventHandler>& event);
        virtual bool DelEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool DelEvent(const uint64_t& sock);

        // net io process
        virtual void ProcessEvent();

        virtual void PostTask(std::function<void(void)>& task);
        virtual void WakeUp();

    private:
        bool _AddEvent(base::CMemSharePtr<CEventHandler>& event, int32_t event_flag, uint64_t sock);
        bool _AddEvent(base::CMemSharePtr<CAcceptEventHandler>& event, int32_t event_flag, uint64_t sock);
        bool _ModifyEvent(base::CMemSharePtr<CEventHandler>& event, int32_t event_flag, uint64_t sock);
        bool _ReserOneShot(base::CMemSharePtr<CEventHandler>& event, int32_t event_flag, uint64_t sock);

        void _DoTimeoutEvent(std::vector<base::CMemSharePtr<CTimerEvent>>& timer_vec);
        void _DoEvent(std::vector<epoll_event>& event_vec, int32_t num);
        void _DoTaskList();
    private:
        std::atomic_bool    _run;

        bool                _per_epoll;
        uint32_t            _epoll_handler;
        uint32_t            _pipe[2];
        epoll_event         _pipe_content;

        std::mutex          _mutex;
        std::vector<std::function<void(void)>> _task_list;
    };
}

#endif
#endif // __linux__
