#ifndef HEADER_IOCP
#define HEADER_IOCP

#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")


#include "EventActions.h"
#include "PoolSharedPtr.h"

#define MAX_BUFFER_LEN        8192
class CEventHandler;
struct EventOverlapped {
	OVERLAPPED	_overlapped;
	WSABUF      _wsa_buf;   
	int			__event_flag_set;
	char        _lapped_buffer[MAX_BUFFER_LEN];
	CMemSharePtr<CEventHandler> _event;

	EventOverlapped() {
		__event_flag_set = 0;
		memset(&_overlapped, 0, sizeof(_overlapped));
		memset(_lapped_buffer, 0, MAX_BUFFER_LEN);
		_wsa_buf.buf = _lapped_buffer;
		_wsa_buf.len = MAX_BUFFER_LEN;
	}

	void Clear() {
		__event_flag_set = 0;
		memset(_lapped_buffer, 0, MAX_BUFFER_LEN);
	}
};

class CIOCP: public CEventActions
{
public:
	CIOCP();
	~CIOCP();

	virtual bool Init();
	virtual bool Dealloc();

	virtual bool AddTimerEvent(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event);
	virtual bool AddSendEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool AddRecvEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool AddAcceptEvent(CMemSharePtr<CEventHandler>& event);
	virtual bool DelEvent(CMemSharePtr<CEventHandler>& event);

	virtual void ProcessEvent();

private:
	bool _PostRecv(CMemSharePtr<CEventHandler>& event);
	bool _PostAccept(CMemSharePtr<CEventHandler>& event);
	bool _PostSend(CMemSharePtr<CEventHandler>& event);

private:
	HANDLE	_iocp_handler;
};

#endif