#ifdef __linux__

#ifndef HEADER_CEPOOL
#define HEADER_CEPOOL
#include <sys/epoll.h>
#include "EventActions.h"

#define MAX_BUFFER_LEN        8192
class Cevent;
class CEpoll: public CEventActions
{
public:
	CEpoll();
	~CEpoll();

	virtual bool Init();
	virtual bool Dealloc();

	virtual bool AddTimerEvent(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event);
	virtual bool AddSendEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool AddRecvEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool AddAcceptEvent(CMemSharePtr<CAcceptEventHandler>& event);
	virtual bool AddConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port);
	virtual bool AddDisconnection(CMemSharePtr<CEventHandler>& event);
	virtual bool DelEvent(unsigned int sock);

	virtual void ProcessEvent();

private:
	bool _AddEvent(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);
	bool _AddEvent(CMemSharePtr<CAcceptEventHandler>& event, int event_flag, unsigned int sock);
	bool _ModifyEvent(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock);
	bool _ReserOneShot(CMemSharePtr<CEventHandler>& event, unsigned int sock);

	void _DoTimeoutEvent(std::vector<TimerEvent>& timer_vec);
	void _DoEvent(std::vector<epoll_event>& event_vec, int num);
private:
	int	_epoll_handler;
};
#endif
#endif // __linux__
