#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "EventHandler.h"
#include "Buffer.h"
#include "Log.h"
#include "EventActions.h"
#include "AcceptSocket.h"
#include "Socket.h"
#include "LinuxFunc.h"

CAcceptSocket::CAcceptSocket(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions){
	
}

CAcceptSocket::~CAcceptSocket() {
	
}

bool CAcceptSocket::Bind(short port, const std::string& ip) {
	_sock = socket(PF_INET, SOCK_STREAM, 0);

	SetSocketNoblocking(_sock);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());

	int ret = bind(_sock, (sockaddr *)&addr, sizeof(sockaddr));

	if (-1 == ret) {
		LOG_FATAL("linux bind socket filed! error code:%d", errno);
		close(_sock);
		return false;
	}
	memcpy(_ip, ip.c_str(), ip.length());
	return true;
}

bool CAcceptSocket::Listen(unsigned int listen_size) {
	int ret = listen(_sock, listen_size);
	if (-1 == ret) {
		LOG_FATAL("linux listen socket filed! error code:%d", errno);
		close(_sock);
		return false;
	}
	_invalid = true;

	return true;
}

void CAcceptSocket::SyncAccept(const std::function<void(CMemSharePtr<CAcceptEventHandler>&, int error)>& call_back) {
	if (!_accept_event) {
		_accept_event = MakeNewSharedPtr<CAcceptEventHandler>(_pool.get());
	}
	if (!_accept_event->_data) {
		_accept_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_accept_event->_data)->events = 0;
	}

	if (!_accept_event->_client_socket) {
		_accept_event->_client_socket = MakeNewSharedPtr<CSocket>(_pool.get(), _event_actions);
	}
	if (!_accept_event->_client_socket->_read_event) {
		_accept_event->_client_socket->_read_event = MakeNewSharedPtr<CEventHandler>(_accept_event->_client_socket->_pool.get());
	}
	if (!_accept_event->_client_socket->_read_event->_buffer) {
		_accept_event->_client_socket->_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_accept_event->_client_socket->_pool.get(), _accept_event->_client_socket->_pool);
	}
	//set call back function
	if (!_accept_event->_call_back) {
		_accept_event->_call_back = call_back;
	}
	//add event to epoll
	if (_event_actions) {
		_accept_event->_event_flag_set |= EVENT_ACCEPT;
		_event_actions->AddAcceptEvent(_accept_event);
	}
}

void CAcceptSocket::SyncAccept(const std::function<void(CMemSharePtr<CAcceptEventHandler>&, int error)>& accept_back,
	const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& read_back) {
	if (!_accept_event) {
		_accept_event = MakeNewSharedPtr<CAcceptEventHandler>(_pool.get());
	}
	if (!_accept_event->_data) {
		_accept_event->_data = _pool->PoolNew<epoll_event>();
		((epoll_event*)_accept_event->_data)->events = 0;
	}
	if (!_accept_event->_accept_socket) {
		_accept_event->_accept_socket = this;
	}

	if (!_accept_event->_client_socket) {
		_accept_event->_client_socket = MakeNewSharedPtr<CSocket>(_pool.get(), _event_actions);
	}
	if (!_accept_event->_client_socket->_read_event) {
		_accept_event->_client_socket->_read_event = MakeNewSharedPtr<CEventHandler>(_accept_event->_client_socket->_pool.get());
	}
	if (!_accept_event->_client_socket->_read_event->_buffer) {
		_accept_event->_client_socket->_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_accept_event->_client_socket->_pool.get(), _accept_event->_client_socket->_pool);
	}

	if (!_accept_event->_call_back) {
		_accept_event->_call_back = accept_back;
	}
	if (!_accept_event->_client_socket->_read_event->_call_back) {
		_accept_event->_client_socket->_read_event->_call_back = read_back;
	}

	if (_event_actions) {
		_accept_event->_event_flag_set |= EVENT_ACCEPT;
		_event_actions->AddAcceptEvent(_accept_event);
	}
}

void CAcceptSocket::SetAcceptCallBack(const std::function<void(CMemSharePtr<CAcceptEventHandler>&, int error)>& call_back) {
	if (!_accept_event) {
		_accept_event = MakeNewSharedPtr<CAcceptEventHandler>(_pool.get());
	}
	_accept_event->_call_back = call_back;
}

void CAcceptSocket::SetReadCallBack(const std::function<void(CMemSharePtr<CEventHandler>&, int error)>& call_back) {
	if (!_accept_event->_client_socket) {
		_accept_event->_client_socket = MakeNewSharedPtr<CSocket>(_pool.get(), _event_actions);
	}
	if (!_accept_event->_client_socket->_read_event) {
		_accept_event->_client_socket->_read_event = MakeNewSharedPtr<CEventHandler>(_accept_event->_client_socket->_pool.get());
	}
	if (!_accept_event->_client_socket) {
		_accept_event->_client_socket = MakeNewSharedPtr<CSocket>(_accept_event->_client_socket->_pool.get(), _event_actions);
	}

	_accept_event->_client_socket->_read_event->_call_back = call_back;
}

void CAcceptSocket::_Accept(CMemSharePtr<CAcceptEventHandler>& event) {
	sockaddr_in client_addr;
	socklen_t addr_size = 0;

	int sock = 0;
	for (;;) {
		//may get more than one connections
		sock = accept(event->_accept_socket->GetSocket(), (sockaddr*)&client_addr, &addr_size);
		if (sock <= 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				break;
			}
			LOG_FATAL("accept socket filed! error code:%d", errno);
			break;
		}
		//set the socket noblocking
		SetSocketNoblocking(sock);
		event->_client_socket->_sock = sock;
		memcpy(event->_client_socket->_ip, inet_ntoa(client_addr.sin_addr), __addr_str_len);
		//get client socket
		event->_client_socket->_read_event->_client_socket = event->_client_socket;
		//call accept call back function
		if (event->_call_back) {
			event->_call_back(event, EVENT_ERROR_NO);
		}
		event->_event_flag_set = 0;
		event->_client_socket = MakeNewSharedPtr<CSocket>(_pool.get(), _event_actions);
		if (!_accept_event->_client_socket->_read_event) {
			_accept_event->_client_socket->_read_event = MakeNewSharedPtr<CEventHandler>(_accept_event->_client_socket->_pool.get());
		}
		if (!_accept_event->_client_socket->_read_event->_buffer) {
			_accept_event->_client_socket->_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_accept_event->_client_socket->_pool.get(), _accept_event->_client_socket->_pool);
		}
	}
}
#endif // __linux__
