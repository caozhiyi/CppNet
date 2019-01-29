#ifdef __linux__

#ifndef HEADER_CEPOOL
#define HEADER_CEPOOL
#include <sys/epoll.h>
#include "EventActions.h"

#define MAX_BUFFER_LEN        8192
class Cevent;
class CEpoll : public CEventActions
{
public:
	CEpoll();
	~CEpoll();

	virtual bool Init();
	virtual bool Dealloc();

    virtual unsigned int AddTimerEvent(unsigned int interval, const std::function<void(void*)>& call_back, void* param, bool always = false);
    virtual bool AddTimerEvent(unsigned int interval, CMemSharePtr<CEventHandler>& event);
    virtual bool RemoveTimerEvent(unsigned int timer_id);
	virtual bool AddSendEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool AddRecvEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool AddAcceptEvent(CMemSharePtr<CAcceptEventHandler>& event);
	virtual bool AddConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port);
	virtual bool AddDisconnection(CMemSharePtr<CEventHandler>& event);
	virtual bool DelEvent(CMemSharePtr<CEventHandler>& event);

	virtual void ProcessEvent();

	virtual void PostTask(std::function<void(void)>& task);
	virtual void WakeUp();

private:
	bool _AddEvent(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);
	bool _AddEvent(CMemSharePtr<CAcceptEventHandler>& event, int event_flag, unsigned int sock);
	bool _ModifyEvent(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);
	bool _ReserOneShot(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);

    void _DoTimeoutEvent(std::vector<CMemSharePtr<CTimerEvent>>& timer_vec);
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
#endif
#endif // __linux__
