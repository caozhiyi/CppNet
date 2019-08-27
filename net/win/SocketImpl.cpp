#ifndef __linux__
#include "EventHandler.h"
#include "Buffer.h"
#include "WinExpendFunc.h"
#include "Log.h"
#include "IOCP.h"
#include "EventActions.h"
#include "SocketImpl.h"
#include "CppNetImpl.h"

using namespace cppnet;

CSocketImpl::CSocketImpl(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions), _post_event_num(0) {
	_read_event = base::MakeNewSharedPtr<CEventHandler>(_pool.get());
	_write_event = base::MakeNewSharedPtr<CEventHandler>(_pool.get());

    _read_event->_data = _pool->PoolNew<EventOverlapped>();
    _read_event->_buffer = base::MakeNewSharedPtr<base::CBuffer>(_pool.get(), _pool);

    _write_event->_data = _pool->PoolNew<EventOverlapped>();
    _write_event->_buffer = base::MakeNewSharedPtr<base::CBuffer>(_pool.get(), _pool);
}

CSocketImpl::~CSocketImpl() {
    // remove from iocp
	if (_read_event && _read_event->_data) {
		EventOverlapped* temp = (EventOverlapped*)_read_event->_data;
		_pool->PoolDelete<EventOverlapped>(temp);
		_read_event->_data = nullptr;
	}
	if (_write_event && _write_event->_data) {
		EventOverlapped* temp = (EventOverlapped*)_write_event->_data;
		_pool->PoolDelete<EventOverlapped>(temp);
		_write_event->_data = nullptr;
	}
}

void CSocketImpl::SyncRead() {
    if (!_read_event->_client_socket) {
        _read_event->_client_socket = memshared_from_this();
    }

	if (_event_actions) {
		_read_event->_event_flag_set = 0;
		_read_event->_event_flag_set |= EVENT_READ;
		if (_event_actions->AddRecvEvent(_read_event)) {
			_post_event_num++;

        // something wrong
        }else {
            CCppNetImpl::Instance()._ReadFunction(_write_event, _read_event->_event_flag_set | CEC_CLOSED);
        }
	}
}

void CSocketImpl::SyncWrite(const char* src, uint32_t len) {
	_write_event->_buffer->Write(src, len);

    if (!_write_event->_client_socket) {
        _write_event->_client_socket = memshared_from_this();
    }

	if (_event_actions) {
        _write_event->_event_flag_set = 0;
		_write_event->_event_flag_set |= EVENT_WRITE;
		if (_event_actions->AddSendEvent(_write_event)) {
			_post_event_num++;

        // something wrong
        } else {
            CCppNetImpl::Instance()._WriteFunction(_write_event, _write_event->_event_flag_set | CEC_CLOSED);
        }
	}
}

void CSocketImpl::SyncConnection(const std::string& ip, uint16_t port, const char* buf, uint32_t buf_len) {
	if (ip.length() > 16 || ip.empty()) {
        base::LOG_ERROR("a wrong ip! ip : %s", ip.c_str());
		return;
	}

    if (!_read_event->_client_socket) {
        _read_event->_client_socket = memshared_from_this();
    }

    // set address info
	strcpy(_ip, ip.c_str());
	_port = port;

	if (_event_actions) {
		_read_event->_event_flag_set = 0;
		_read_event->_event_flag_set |= EVENT_CONNECT;
		if (_event_actions->AddConnection(_read_event, ip, port, buf, buf_len)) {
			_post_event_num++;
		}
	}
}

void CSocketImpl::SyncConnection(const std::string& ip, uint16_t port) {
    SyncConnection(ip, port, nullptr, 0);
}

void CSocketImpl::SyncDisconnection() {
    if (!_read_event->_client_socket) {
        _read_event->_client_socket = memshared_from_this();
    }

	if (_event_actions) {
		_read_event->_event_flag_set = 0;
		_read_event->_event_flag_set |= EVENT_DISCONNECT;
		if (_event_actions->AddDisconnection(_read_event)) {
			_post_event_num++;
		}
	}
}

void CSocketImpl::SyncRead(uint32_t interval) {

    SyncRead();

    // add to timer
	if (_event_actions) {
		_read_event->_event_flag_set |= EVENT_TIMER;
		_event_actions->AddTimerEvent(interval, _read_event);
		_post_event_num++;
	}
}

void CSocketImpl::SyncWrite(uint32_t interval, const char* src, uint32_t len) {

    SyncWrite(src, len);

	if (_event_actions) {
		_write_event->_event_flag_set |= EVENT_TIMER;
		_event_actions->AddTimerEvent(interval, _write_event);
		_post_event_num++;
	}
}

void CSocketImpl::PostTask(std::function<void(void)>& func) {
	_event_actions->PostTask(func);
}

bool cppnet::operator>(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock > s2._sock;
}

bool cppnet::operator<(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock < s2._sock;
}

bool cppnet::operator==(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock == s2._sock;
}

bool cppnet::operator!=(const CSocketBase& s1, const CSocketBase& s2) {
	return s1._sock != s2._sock;
}

void CSocketImpl::_Recv(base::CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	_post_event_num--;
	int err = -1;
	if (event->_event_flag_set & EVENT_TIMER) {
		err = CEC_TIMEOUT | event->_event_flag_set;
        event->_event_flag_set &= ~EVENT_TIMER;

	//get a connection event
	} else if (event->_event_flag_set == EVENT_CONNECT) {
		err = CEC_SUCCESS | event->_event_flag_set;

	} else if (event->_event_flag_set & EVENT_DISCONNECT) {
		err = CEC_SUCCESS | event->_event_flag_set;

	//get 0 bytes means close
	} else if (!event->_off_set) {
		if (_post_event_num == 0) {
			err = CEC_CLOSED | event->_event_flag_set;
		}

	} else {
		err = CEC_SUCCESS | event->_event_flag_set;
		event->_buffer->Write(context->_wsa_buf.buf, event->_off_set);
	}
	if (err > -1) {
        CCppNetImpl::Instance()._ReadFunction(event, err);
		event->_event_flag_set = 0;
	}
}

void CSocketImpl::_Send(base::CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	_post_event_num--;
	int err = -1;
	if (event->_event_flag_set & EVENT_TIMER) {
		err = CEC_TIMEOUT | event->_event_flag_set;
        event->_event_flag_set &= ~EVENT_TIMER;

	} else if (!event->_off_set) {
		if (_post_event_num == 0) {
			err = CEC_CLOSED | event->_event_flag_set;
		}

	} else {
		err = CEC_SUCCESS | event->_event_flag_set;
	}

	if (err > -1) {
        CCppNetImpl::Instance()._WriteFunction(event, err);
		event->_event_flag_set = 0;
	}
}

#endif