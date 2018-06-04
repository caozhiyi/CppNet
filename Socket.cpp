#ifndef linux
#include "EventHandler.h"
#include "Buffer.h"
#include "WinExpendFunc.h"
#include "Log.h"
#include "IOCP.h"
#include "EventActions.h"
#include "Socket.h"

CSocket::CSocket(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions) {
	_read_event = MakeNewSharedPtr<CEventHandler>(*(_pool.get()));
	_write_event = MakeNewSharedPtr<CEventHandler>(*(_pool.get()));
}

CSocket::~CSocket() {
	
}

void CSocket::SyncRead(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	if (!_read_event) {
		_read_event = MakeNewSharedPtr<CEventHandler>(*(_pool.get()));
	}
	if (!_read_event->_data) {
		_read_event->_data = _pool->PoolNew<EventOverlapped>();
	}
	if (!_read_event->_call_back) {
		_read_event->_call_back = call_back;
	}

	if (!_read_event->_buffer) {
		_read_event->_buffer = MakeNewSharedPtr<CBuffer>(*(_pool.get()), _pool);
	}
	
	if (_event_actions) {
		_event_actions->AddRecvEvent(_read_event);
	}
}

void CSocket::SyncWrite(char* src, int len, const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	if (!_write_event) {
		_write_event = MakeNewSharedPtr<CEventHandler>(*(_pool.get()));
	}
	if (!_write_event->_data) {
		_write_event->_data = _pool->PoolNew<EventOverlapped>();
	}
	if (!_write_event->_call_back) {
		_write_event->_call_back = call_back;
	}

	if (!_write_event->_buffer) {
		_write_event->_buffer = MakeNewSharedPtr<CBuffer>(*(_pool.get()), _pool);
	}
	_write_event->_buffer->Write(src, len);

	if (!_write_event->_client_socket) {
		_write_event->_client_socket = _read_event->_client_socket;
	}
	if (_event_actions) {
		_event_actions->AddSendEvent(_write_event);
	}
}

void CSocket::SyncRead(unsigned int interval, const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	if (!_read_event) {
		_read_event = MakeNewSharedPtr<CEventHandler>(*(_pool.get()));
	}
	if (!_read_event->_data) {
		_read_event->_data = _pool->PoolNew<EventOverlapped>();
	}

	if (!_read_event->_call_back) {
		_read_event->_call_back = call_back;
	}

	if (!_read_event->_buffer) {
		_read_event->_buffer = MakeNewSharedPtr<CBuffer>(*(_pool.get()), _pool);
	}

	if (_event_actions) {
		_event_actions->AddRecvEvent(_read_event);
	}

	if (_event_actions) {
		_event_actions->AddTimerEvent(interval, EVENT_READ, _read_event);
	}
}

void CSocket::SyncWrite(unsigned int interval, char* src, int len, const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	if (!_write_event) {
		_write_event = MakeNewSharedPtr<CEventHandler>(*(_pool.get()));
	}
	if (!_write_event->_data) {
		_write_event->_data = _pool->PoolNew<EventOverlapped>();
	}
	if (!_write_event->_call_back) {
		_write_event->_call_back = call_back;
	}

	if (!_write_event->_client_socket) {
		_write_event->_client_socket = _read_event->_client_socket;
	}
	if (!_write_event->_buffer) {
		_write_event->_buffer = MakeNewSharedPtr<CBuffer>(*(_pool.get()), _pool);
	}
	_write_event->_buffer->Write(src, len);

	if (_event_actions) {
		_event_actions->AddSendEvent(_write_event);
	}

	if (_event_actions) {
		_event_actions->AddTimerEvent(interval, EVENT_WRITE, _write_event);
	}
}

void CSocket::SetReadCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	_read_event->_call_back = call_back;
}

void CSocket::SetWriteCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	_read_event->_call_back = call_back;
}

bool operator>(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock > s2._sock;
}

bool operator<(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock < s2._sock;
}

bool operator==(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock == s2._sock;
}

bool operator!=(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock != s2._sock;
}


void CSocket::_Recv(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	int error = EVENT_ERROR_NO;
	if (event->_off_set) {
		event->_buffer->Write(context->_wsa_buf.buf, event->_off_set);
	
	} else {
		error = EVENT_ERROR_CLOSED;
	}

	if (event->_timer_out) {
		error = EVENT_ERROR_TIMEOUT;
	}
	event->_event_flag_set = 0;
	if (event->_call_back) {
		event->_call_back(event, error);
	}
}

void CSocket::_Send(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	int error = EVENT_ERROR_NO;
	if (!event->_off_set) {
		error = EVENT_ERROR_CLOSED;
	} 

	if (event->_timer_out) {
		error = EVENT_ERROR_TIMEOUT;
	}
	event->_event_flag_set = 0;
	if (event->_call_back) {
		event->_call_back(event, error);
	}
}

#endif