#ifndef HEADER_CEPOOL
#define HEADER_CEPOOL

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
	virtual bool DelEvent(CMemSharePtr<CEventHandler>& event);

	virtual void ProcessEvent();

private:
	bool _PostRecv(CMemSharePtr<CEventHandler>& event);
	bool _PostAccept(CMemSharePtr<CAcceptEventHandler>& event);
	bool _PostSend(CMemSharePtr<CEventHandler>& event);
	bool _PostConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port);
	bool _PostDisconnection(CMemSharePtr<CEventHandler>& event);

	void _DoTimeoutEvent(std::vector<TimerEvent>& timer_vec);
private:
	int	_epoll_handler;
};

#endif