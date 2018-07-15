#ifndef HEADER_EVENTHANDLER
#define HEADER_EVENTHANDLER
#include <memory>
#include <atomic>
#include "PoolSharedPtr.h"
#include "AcceptSocket.h"
#include "Socket.h"
#define INVALID_TIMER -1

enum EVENT_FLAG {
	EVENT_READ			= 0x0001,		//read event
	EVENT_WRITE			= 0x0002,		//write event
	EVENT_ACCEPT		= 0x0004,		//accept event
	EVENT_TIMER			= 0x0008,		//timer event
	EVENT_CONNECT		= 0x0010,		//connect event
	EVENT_DISCONNECT	= 0x0020		//disconnect event
};

enum EVENT_ERROR {
	EVENT_ERROR_NO		= 0x0100,
	EVENT_ERROR_TIMEOUT = 0x0200,
	EVENT_ERROR_CLOSED	= 0x0400,
	EVENT_ERROR_DONE	= 0x0800,
};

class Cevent {
public:
	void*						_data = nullptr;
	int							_event_flag_set = 0;
};

class CBuffer;
class CEventHandler : public Cevent {
public:
	CMemSharePtr<CBuffer>		_buffer;
	CMemWeakPtr<CSocket>		_client_socket;
	int							_off_set;				//read or write size

	bool						_timer_out = false;
	bool						_timer_set = false;		//is add in timer map?
	unsigned int				_timer_id = 0;
	std::function<void(CMemSharePtr<CEventHandler>&, int error)>	_call_back;
};

class CAcceptEventHandler : public Cevent {
public:
	CMemSharePtr<CSocket>		_client_socket;

	CMemSharePtr<CAcceptSocket>	_accept_socket = nullptr;
	std::function<void(CMemSharePtr<CAcceptEventHandler>&, int error)>	_call_back;
};
#endif