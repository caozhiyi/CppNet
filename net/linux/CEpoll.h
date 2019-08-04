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
        CEpoll();
        ~CEpoll();

        virtual bool Init();
        virtual bool Dealloc();

        virtual uint64_t AddTimerEvent(unsigned int interval, const std::function<void(void*)>& call_back, void* param, bool always = false);
        virtual bool AddTimerEvent(unsigned int interval, base::CMemSharePtr<CEventHandler>& event);
        virtual bool RemoveTimerEvent(unsigned int timer_id);
        virtual bool AddSendEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool AddRecvEvent(base::CMemSharePtr<CEventHandler>& event);
        virtual bool AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event);
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port);
        virtual bool AddDisconnection(base::CMemSharePtr<CEventHandler>& event);
        virtual bool DelEvent(base::CMemSharePtr<CEventHandler>& event);

        virtual void ProcessEvent();

        virtual void PostTask(std::function<void(void)>& task);
        virtual void WakeUp();

    private:
        bool _AddEvent(base::CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);
        bool _AddEvent(base::CMemSharePtr<CAcceptEventHandler>& event, int event_flag, unsigned int sock);
        bool _ModifyEvent(base::CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);
        bool _ReserOneShot(base::CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);

        void _DoTimeoutEvent(std::vector<base::CMemSharePtr<CTimerEvent>>& timer_vec);
        void _DoEvent(std::vector<epoll_event>& event_vec, int num);
        void _DoTaskList();
    private:
        std::atomic_bool	_run;

        int				_epoll_handler;
        unsigned int	_pipe[2];
        epoll_event		_pipe_content;

        std::mutex		_mutex;
        std::vector<std::function<void(void)>> _task_list;
    };
}

#endif
#endif // __linux__
