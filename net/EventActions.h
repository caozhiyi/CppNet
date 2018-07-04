#ifndef HEADER_EVENTACTIONS
#define HEADER_EVENTACTIONS
#include <string>
#include "Timer.h"
class CEventHandler;
class CEventActions
{
public:
	CEventActions() {}
	virtual ~CEventActions() {}

	//param is net io thread num, default cpu number
	virtual bool Init() = 0;
	virtual bool Dealloc() = 0;

	virtual bool AddTimerEvent(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event) = 0;
	virtual bool AddSendEvent(CMemSharePtr<CEventHandler>& event) = 0;
	virtual bool AddRecvEvent(CMemSharePtr<CEventHandler>& event) = 0;
	virtual bool AddAcceptEvent(CMemSharePtr<CAcceptEventHandler>& event) = 0;
#ifndef __linux__
	virtual bool AddConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, char* buf, int buf_len) = 0;
#else
	virtual bool AddConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) = 0;
#endif
	virtual bool AddDisconnection(CMemSharePtr<CEventHandler>& event) = 0;
	virtual bool DelEvent(CMemSharePtr<CEventHandler>& event) = 0;
	virtual void ProcessEvent() = 0;

	virtual CTimer& Timer() { return _timer; }
protected:
	CTimer			_timer;
	int				_need_init;
	std::string		_name;
};

#endif