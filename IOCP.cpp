#include "IOCP.h"
#include "OSInfo.h"
#include "Log.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "Socket.h"
#include "WinExpendFunc.h"
#include "Timer.h"

enum STATE_CODE {
	EXIT_IOCP = 0,
	WEAK_UP_IOCP = 1,
};

CIOCP::CIOCP() {

}

CIOCP::~CIOCP() {

}

bool CIOCP::Init() {
	int _threads_num = GetCpuNum() * 2;

	_iocp_handler = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, _threads_num);;
	if (_iocp_handler == INVALID_HANDLE_VALUE) {
		LOG_FATAL("IOCP create io completion port failed!");
		return false;
	}
	return true;
}

bool CIOCP::Dealloc() {
	if (CloseHandle(_iocp_handler) == -1) {
		LOG_ERROR("IOCP close io completion port failed!");
	}
	_iocp_handler = nullptr;
	return true;
}

bool CIOCP::AddTimerEvent(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event) {
	_timer.AddTimer(interval, event_flag, event);
	return true;
}

bool CIOCP::AddSendEvent(CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		if (!socket_ptr->IsInActions()) {
			if (CreateIoCompletionPort((HANDLE)(socket_ptr->GetSocket()), _iocp_handler, 0, 0) == NULL) {
				LOG_ERROR("IOCP bind socket to io completion port failed!");
				return false;
			}
		}
		((EventOverlapped*)event->_data)->_event = &event;
		socket_ptr->SetInActions(true);
		return _PostSend(event);
	}
	LOG_WARN("write event is already distroyed!");
	return false;
}

bool CIOCP::AddRecvEvent(CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		if (!socket_ptr->IsInActions()) {
			if (CreateIoCompletionPort((HANDLE)(socket_ptr->GetSocket()), _iocp_handler, 0, 0) == NULL) {
				LOG_ERROR("IOCP bind socket to io completion port failed!");
				return false;
			}
		}
		((EventOverlapped*)event->_data)->_event = &event;
		socket_ptr->SetInActions(true);
		return _PostRecv(event);
	}
	LOG_WARN("read event is already distroyed!");
	return false;
}

bool CIOCP::AddAcceptEvent(CMemSharePtr<CAcceptEventHandler>& event) {
	if (!event->_accept_socket->IsInActions()) {
		if (CreateIoCompletionPort((HANDLE)(event->_accept_socket->GetSocket()), _iocp_handler, 0, 0) == NULL) {
			LOG_ERROR("IOCP bind socket to io completion port failed!");
			return false;
		}
	}
	((EventOverlapped*)event->_data)->_event = &event;
	event->_accept_socket->SetInActions(true);
	return _PostAccept(event);
}

bool CIOCP::AddConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		if (!socket_ptr->IsInActions()) {
			if (CreateIoCompletionPort((HANDLE)(socket_ptr->GetSocket()), _iocp_handler, 0, 0) == NULL) {
				LOG_ERROR("IOCP bind socket to io completion port failed!");
				return false;
			}
		}
		((EventOverlapped*)event->_data)->_event = &event;
		socket_ptr->SetInActions(true);
		return _PostConnection(event, ip, port);
	}
	LOG_WARN("read event is already distroyed!");
	return false;
}

bool CIOCP::AddDisconnection(CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		if (!socket_ptr->IsInActions()) {
			if (CreateIoCompletionPort((HANDLE)(socket_ptr->GetSocket()), _iocp_handler, 0, 0) == NULL) {
				LOG_ERROR("IOCP bind socket to io completion port failed!");
				return false;
			}
		}
		((EventOverlapped*)event->_data)->_event = &event;
		socket_ptr->SetInActions(true);
		return _PostDisconnection(event);
	}
	LOG_WARN("read event is already distroyed!");
	return false;
	return true;
}

bool CIOCP::DelEvent(CMemSharePtr<CEventHandler>& event) {
	((EventOverlapped*)event->_data)->_event = nullptr;
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		CancelIoEx((HANDLE)socket_ptr->GetSocket(), &((EventOverlapped*)event->_data)->_overlapped);
	}
	return true;
}

void CIOCP::ProcessEvent() {
	DWORD				bytes_transfered = 0;
	EventOverlapped		*socket_context = nullptr;
	OVERLAPPED          *over_lapped = nullptr;
	unsigned int		wait_time = 0;
	std::vector<TimerEvent> timer_vec;
	for (;;) {
		wait_time = _timer.TimeoutCheck(timer_vec);
		//if there is no timer event. wait until recv something
		if (wait_time == 0 && timer_vec.empty()) {
			wait_time = INFINITE;
		}

		int res = GetQueuedCompletionStatus(_iocp_handler, &bytes_transfered, PULONG_PTR(&socket_context),
			&over_lapped, wait_time);

		DWORD dw_err = 0;
		if (res) {
			dw_err = NO_ERROR;

		} else {
			dw_err = GetLastError();
		}
		if (dw_err == WAIT_TIMEOUT) {
			if (!timer_vec.empty()) {
				_DoTimeoutEvent(timer_vec);
			}

		} else if (ERROR_NETNAME_DELETED == dw_err || NO_ERROR == dw_err || ERROR_IO_PENDING == dw_err) {
			if (over_lapped) {
				socket_context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
				LOG_DEBUG("Get a new event : %d", socket_context->_event_flag_set);
				_DoEvent(socket_context, bytes_transfered);
			}

		} else {
			LOG_ERROR("IOCP GetQueuedCompletionStatus return error : %d", dw_err);
			continue;
		}
	}
}

bool CIOCP::_PostRecv(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	context->Clear();
	context->_event_flag_set = event->_event_flag_set;
	OVERLAPPED *lapped = &context->_overlapped;
	auto socket_ptr = event->_client_socket.Lock();
	int res = WSARecv(socket_ptr->GetSocket(), &context->_wsa_buf, 1, &dwFlags, &dwBytes, lapped, nullptr);

	if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
		LOG_FATAL("IOCP post recv event failed! error code: %d", WSAGetLastError());
		return false;
	}
	LOG_DEBUG("post a new event : %d", context->_event_flag_set);
	return true;
}

bool CIOCP::_PostAccept(CMemSharePtr<CAcceptEventHandler>& event) {
	if (!__AcceptEx) {
		LOG_ERROR("__AcceptEx function is null!");
		return false;
	}

	EventOverlapped* context = (EventOverlapped*)event->_data;
	context->Clear();
	DWORD dwBytes = 0;
	context->_event_flag_set |= event->_event_flag_set;
	OVERLAPPED *lapped = &context->_overlapped;
	
	int res = __AcceptEx((SOCKET)event->_accept_socket->GetSocket(), (SOCKET)event->_client_socket->GetSocket(), &context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2), 
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, lapped);
	if (FALSE == res) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			LOG_ERROR("IOCP post accept failed! error code:%d", WSAGetLastError());
			return false;
		}
	}
	LOG_DEBUG("post a new event : %d", context->_event_flag_set);
	return true;
}

bool CIOCP::_PostSend(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	context->Clear();
	context->_event_flag_set = event->_event_flag_set;
	context->_wsa_buf.len = event->_buffer->Read(context->_lapped_buffer, MAX_BUFFER_LEN);
	
	OVERLAPPED *lapped = &context->_overlapped;
	auto socket_ptr = event->_client_socket.Lock();
	int res = WSASend(socket_ptr->GetSocket(), &context->_wsa_buf, 1, nullptr, 0, lapped, nullptr);

	if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
		LOG_FATAL("IOCP post send event failed! error code: %d", WSAGetLastError());
		return false;
	}
	LOG_DEBUG("post a new event : %d", context->_event_flag_set);
	return true;
}

bool CIOCP::_PostConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	context->Clear();
	context->_event_flag_set = event->_event_flag_set;
	OVERLAPPED *lapped = &context->_overlapped;

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

	SOCKADDR_IN local;
	local.sin_family = AF_INET;
	local.sin_port = htons(0);
	local.sin_addr.S_un.S_addr = INADDR_ANY;

	auto socket_ptr = event->_client_socket.Lock();
	if (SOCKET_ERROR == bind(socket_ptr->GetSocket(), (sockaddr*)&local, sizeof(local))) {
		LOG_FATAL("bind local host failed! error code: %d", WSAGetLastError());
	}
	int res = __ConnectEx(socket_ptr->GetSocket(), (sockaddr*)&addr, sizeof(addr), nullptr, 0, nullptr, lapped);

	if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
		LOG_FATAL("IOCP post connect event failed! error code: %d", WSAGetLastError());
		return false;
	}
	LOG_DEBUG("post a new event : %d", context->_event_flag_set);
	return true;
}

#include <Mswsock.h>
bool CIOCP::_PostDisconnection(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	context->Clear();
	context->_event_flag_set = event->_event_flag_set;
	context->_wsa_buf.len = event->_buffer->Read(context->_lapped_buffer, MAX_BUFFER_LEN);

	OVERLAPPED *lapped = &context->_overlapped;
	auto socket_ptr = event->_client_socket.Lock();
	int res = __DisconnectionEx(socket_ptr->GetSocket(), lapped, 0, 0);

	if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
		LOG_FATAL("IOCP post send event failed! error code: %d", WSAGetLastError());
		return false;
	}
	LOG_DEBUG("post a new event : %d", context->_event_flag_set);
	return true;
}

void CIOCP::_DoTimeoutEvent(std::vector<TimerEvent>& timer_vec) {
	for (auto iter = timer_vec.begin(); iter != timer_vec.end(); ++iter) {
		if (iter->_event_flag & EVENT_READ) {
			auto socket_ptr = iter->_event->_client_socket.Lock();
			if (socket_ptr) {
				socket_ptr->_Recv(iter->_event);
			}

		}
		else if (iter->_event_flag & EVENT_WRITE) {
			auto socket_ptr = iter->_event->_client_socket.Lock();
			if (socket_ptr) {
				socket_ptr->_Send(iter->_event);
			}
		}
	}
	timer_vec.clear();
}

void CIOCP::_DoEvent(EventOverlapped *socket_context, int bytes) {
	if (socket_context->_event_flag_set & EVENT_ACCEPT) {
		CMemSharePtr<CAcceptEventHandler>* event = (CMemSharePtr<CAcceptEventHandler>*)socket_context->_event;
		if (event) {
			(*event)->_client_socket->_read_event->_off_set = bytes;
			(*event)->_accept_socket->_Accept((*event));
		}

	} else {
		CMemSharePtr<CEventHandler>* event = (CMemSharePtr<CEventHandler>*)socket_context->_event;
		if (event) {
			(*event)->_off_set = bytes;
			if (socket_context->_event_flag_set & EVENT_READ || socket_context->_event_flag_set & EVENT_CONNECT) {
				auto socket_ptr = (*event)->_client_socket.Lock();
				if (socket_ptr) {
					socket_ptr->_Recv((*event));
				}

			} else if ((*event)->_event_flag_set & EVENT_WRITE) {
				auto socket_ptr = (*event)->_client_socket.Lock();
				if (socket_ptr) {
					socket_ptr->_Send((*event));
				}
			}
		}
	}
}