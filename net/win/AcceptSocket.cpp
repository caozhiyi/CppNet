#ifndef __linux__
#include "EventHandler.h"
#include "Buffer.h"
#include "WinExpendFunc.h"
#include "Log.h"
#include "IOCP.h"
#include "EventActions.h"
#include "AcceptSocket.h"
#include "Socket.h"

CAcceptSocket::CAcceptSocket(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions){
	
}

CAcceptSocket::~CAcceptSocket() {
	
}

bool CAcceptSocket::Bind(short port, const std::string& ip) {
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());

	int ret = bind(_sock, (sockaddr *)&addr, sizeof(sockaddr));

	if (SOCKET_ERROR == ret) {
		LOG_FATAL("win32 bind socket filed!");
		WSACleanup();
		closesocket(_sock);
		return false;
	}
	memcpy(_ip, ip.c_str(), ip.length());
	return true;
}

bool CAcceptSocket::Listen(unsigned int listen_size) {
	int ret = listen(_sock, listen_size);
	if (SOCKET_ERROR == ret) {
		LOG_FATAL("win32 listen socket filed!");
		WSACleanup();
		closesocket(_sock);
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
		_accept_event->_data = _pool->PoolNew<EventOverlapped>();
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
		_accept_event->_call_back = call_back;
	}

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
		_accept_event->_data = _pool->PoolNew<EventOverlapped>();
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
	EventOverlapped* context = (EventOverlapped*)event->_data;

	SOCKADDR_IN* client_addr = NULL;
	int remote_len = sizeof(SOCKADDR_IN);
	SOCKADDR_IN* LocalAddr = NULL;
	int localLen = sizeof(SOCKADDR_IN);

	__AcceptExScokAddrs(context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&client_addr, &remote_len);

	memcpy(event->_client_socket->_ip, inet_ntoa(client_addr->sin_addr), __addr_str_len);
	event->_client_socket->_read_event->_buffer->Write(context->_lapped_buffer, event->_client_socket->_read_event->_off_set);
	//get client socket
	event->_client_socket->_read_event->_client_socket = event->_client_socket;

	//call accept call back function
	event->_event_flag_set = 0;
	if (event->_call_back) {
		event->_call_back(event, 0);
	}

	//call read call back function
	if (event->_client_socket->_read_event->_call_back) {
		event->_client_socket->_read_event->_call_back(event->_client_socket->_read_event, 0);
	}

	//post Accept
	context->Clear();
	event->_client_socket = MakeNewSharedPtr<CSocket>(_pool.get(), _event_actions);
	if (!_accept_event->_client_socket->_read_event) {
		_accept_event->_client_socket->_read_event = MakeNewSharedPtr<CEventHandler>(_accept_event->_client_socket->_pool.get());
	}
	if (!_accept_event->_client_socket->_read_event->_buffer) {
		_accept_event->_client_socket->_read_event->_buffer = MakeNewSharedPtr<CBuffer>(_accept_event->_client_socket->_pool.get(), _accept_event->_client_socket->_pool);
	}
	_accept_event->_event_flag_set |= EVENT_ACCEPT;
	_event_actions->AddAcceptEvent(event);
}
#endif