#ifdef __linux__
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "EventHandler.h"
#include "Buffer.h"
#include "Log.h"
#include "EventActions.h"
#include "Socket.h"
#include "Runnable.h"
#include "LinuxFunc.h"

CSocket::CSocket(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions){
	_read_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	_write_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
}

CSocket::~CSocket() {
	LOG_DEBUG("delete from epoll, socket : %d, TheadId : %d", _sock, std::this_thread::get_id());
	//delete from epoll
	if (_event_actions) {
		if (_event_actions->DelEvent(_read_event)) {
			CRunnable::Sleep(5000);
			close(_sock);
		}
	}
	
	if (_read_event && _read_event->_data) {
		epoll_event* temp = (epoll_event*)_read_event->_data;
		_pool->PoolDelete<epoll_event>(temp);
		_read_event->_data = nullptr;
	}
	if (_write_event && _write_event->_data) {
		epoll_event* temp = (epoll_event*)_write_event->_data;
		_pool->PoolDelete<epoll_event>(temp);
		_write_event->_data = nullptr;
	}
}

void CSocket::SyncRead() {
	if (!_read_event->_call_back) {
		LOG_WARN("call back function is null");
		return;
	}

	if (!_read_event) {
		_read_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	}
	if (!_read_event->_data) {
		_read_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_read_event->_data)->events = 0;
	}
	if (!_read_event->_buffer) {
		_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_pool.get(), _pool);
	}
	
	if (_event_actions) {
		_read_event->_event_flag_set |= EVENT_READ;
		_event_actions->AddRecvEvent(_read_event);
	}
}

void CSocket::SyncWrite(char* src, int len) {
	if (!_write_event->_call_back) {
		LOG_WARN("call back function is null");
		return;
	}

	if (!_write_event) {
		_write_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	}
	if (!_write_event->_data) {
		_write_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_write_event->_data)->events = 0;
	}

	if (!_write_event->_buffer) {
		_write_event->_buffer = MakeNewSharedPtr<CBuffer>(_pool.get(), _pool);
	}
	_write_event->_buffer->Write(src, len);

	if (!_write_event->_client_socket) {
		_write_event->_client_socket = _read_event->_client_socket;
	}
	if (_event_actions) {
		_write_event->_event_flag_set |= EVENT_WRITE;
		_event_actions->AddSendEvent(_write_event);
	}
}

void CSocket::SyncConnection(const std::string& ip, short port) {
	if (!_write_event->_call_back) {
		LOG_WARN("call back function is null");
		return;
	}

	if (ip.length() > 16) {
		LOG_ERROR("a wrong ip! %s", ip.c_str());
		return;
	}
	strcpy(_ip, ip.c_str());
	if (!_read_event) {
		_read_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	}
	if (!_read_event->_data) {
		_read_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_read_event->_data)->events = 0;
	}

	if (!_read_event->_buffer) {
		_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_pool.get(), _pool);
	}

	if (!_read_event->_client_socket){
		_read_event->_client_socket = memshared_from_this();
	}

	//craete socket
	_sock = socket(PF_INET, SOCK_STREAM, 0);
	SetSocketNoblocking(_sock);

	if (_event_actions) {
		_read_event->_event_flag_set |= EVENT_CONNECT;
		_event_actions->AddConnection(_read_event, ip, port);
	}
}

void CSocket::SyncDisconnection() {
	if (!_read_event->_call_back) {
		LOG_WARN("call back function is null");
		return;
	}

	if (!_read_event) {
		_read_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	}
	if (!_read_event->_data) {
		_read_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_read_event->_data)->events = 0;
	}
	
	if (!_read_event->_buffer) {
		_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_pool.get(), _pool);
	}

	if (!_read_event->_client_socket) {
		_read_event->_client_socket = memshared_from_this();
	}

	if (_event_actions) {
		_read_event->_event_flag_set |= EVENT_DISCONNECT;
		_event_actions->AddDisconnection(_read_event);
	}
}

void CSocket::SyncRead(unsigned int interval) {
	if (!_read_event->_call_back) {
		LOG_WARN("call back function is null");
		return;
	}

	if (!_read_event) {
		_read_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	}
	if (!_read_event->_data) {
		_read_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_read_event->_data)->events = 0;
	}

	if (!_read_event->_buffer) {
		_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_pool.get(), _pool);
	}

	if (_event_actions) {
		_read_event->_event_flag_set |= EVENT_READ;
		_event_actions->AddRecvEvent(_read_event);
	}

	if (_event_actions) {
		_read_event->_event_flag_set |= EVENT_TIMER;
		_event_actions->AddTimerEvent(interval, EVENT_READ, _read_event);
	}
}

void CSocket::SyncWrite(unsigned int interval, char* src, int len) {
	if (!_write_event->_call_back) {
		LOG_WARN("call back function is null");
		return;
	}

	if (!_write_event) {
		_write_event = MakeNewSharedPtr<CEventHandler>(_pool.get());
	}
	if (!_write_event->_data) {
		_write_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_write_event->_data)->events = 0;
	}
	
	if (!_write_event->_client_socket) {
		_write_event->_client_socket = _read_event->_client_socket;
	}
	if (!_write_event->_buffer) {
		_write_event->_buffer = MakeNewSharedPtr<CBuffer>(_pool.get(), _pool);
	}
	_write_event->_buffer->Write(src, len);

	if (_event_actions) {
		_write_event->_event_flag_set |= EVENT_WRITE;
		_event_actions->AddSendEvent(_write_event);
	}

	if (_event_actions) {
		_write_event->_event_flag_set |= EVENT_TIMER;
		_event_actions->AddTimerEvent(interval, EVENT_WRITE, _write_event);
	}
}

void CSocket::SetReadCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	_read_event->_call_back = call_back;
}

void CSocket::SetWriteCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	_write_event->_call_back = call_back;
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
	if (!event->_client_socket) {
		return;
	}
	auto socket_ptr = event->_client_socket.Lock();
	if (!socket_ptr) {
		return;
	}
	LOG_ERROR("event->_event_flag_set : %d", event->_event_flag_set);
	int err = -1;
	if (event->_timer_out) {
		err = EVENT_ERROR_TIMEOUT | event->_event_flag_set;
		event->_timer_out = false;
		//reset timer flag
		event->_event_flag_set &= ~EVENT_TIMER;

	//get a connection event
	} else if (event->_event_flag_set & EVENT_CONNECT) {
		err = EVENT_ERROR_NO | event->_event_flag_set;
		event->_event_flag_set &= ~EVENT_CONNECT;

	} else if (event->_event_flag_set & EVENT_DISCONNECT) {
		err = EVENT_ERROR_NO | event->_event_flag_set;
		event->_event_flag_set &= ~EVENT_DISCONNECT;

	} else {
		err = EVENT_ERROR_NO | event->_event_flag_set;
		if (event->_event_flag_set & EVENT_READ) {
			event->_off_set = 0;
			//read all data.
			for (;;) {
				char buf[65536] = { 0 };
				int recv_len = 0;
				recv_len = recv(socket_ptr->GetSocket(), buf, 65536, 0);
				if (recv_len < 0) {
					if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
						break;

					} else if (errno == EBADMSG || errno == ECONNRESET) {
						err = EVENT_ERROR_CLOSED | event->_event_flag_set;
						break;

					} else {
						LOG_ERROR("recv filed! %d", errno);
						break;
					}
				} else if (recv_len == 0) {
					err = EVENT_ERROR_CLOSED | event->_event_flag_set;
					break;
				}
				event->_buffer->Write(buf, recv_len);
				event->_off_set += recv_len;
				memset(buf, 0, recv_len);
			}
		}
	}
	if (event->_call_back) {
		event->_call_back(event, err);
	}
}

void CSocket::_Send(CMemSharePtr<CEventHandler>& event) {
	if (!event->_client_socket) {
		return;
	}
	auto socket_ptr = event->_client_socket.Lock();
	if (!socket_ptr) {
		return;
	}

	int err = -1;
	if (event->_timer_out) {
		err = EVENT_ERROR_TIMEOUT | event->_event_flag_set;
		event->_timer_out = false;
		event->_event_flag_set &= ~EVENT_TIMER;

	} else {
		err = EVENT_ERROR_NO | event->_event_flag_set;
		event->_off_set = 0;
		if (event->_buffer && event->_buffer->GetCanReadSize()) {
			char buf[65536] = { 0 };
			int send_len = 0;
			send_len = event->_buffer->Read(buf, 65536);
			int res = send(socket_ptr->GetSocket(), buf, send_len, 0);
			if (res < 0) {
				if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) {
					//wait next time to do

				} else if (errno == EBADMSG) {
					err = EVENT_ERROR_CLOSED | event->_event_flag_set;

				} else {
					err = EVENT_ERROR_CLOSED | event->_event_flag_set;
					LOG_ERROR("send filed! %d", errno);
				}
			}
			event->_off_set = res;
		}
		
		if (event->_call_back) {
			event->_call_back(event, err);
		}
	}
}
#endif