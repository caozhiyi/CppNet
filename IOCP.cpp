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
	event->_event_flag_set |= EVENT_TIMER;
	return true;
}

bool CIOCP::AddSendEvent(CMemSharePtr<CEventHandler>& event) {
	if (!event->_client_socket->IsInActions()) {
		if (CreateIoCompletionPort((HANDLE)(event->_client_socket->GetSocket()), _iocp_handler, 0, 0) == NULL) {
			LOG_ERROR("IOCP bind socket to io completion port failed!");
			return false;
		}
	}
	
	((EventOverlapped*)event->_data)->_event = event;
	event->_event_flag_set |= EVENT_WRITE;
	event->_client_socket->SetInActions(true);
	return _PostSend(event);
}

bool CIOCP::AddRecvEvent(CMemSharePtr<CEventHandler>& event) {
	if (!event->_client_socket->IsInActions()) {
		if (CreateIoCompletionPort((HANDLE)(event->_client_socket->GetSocket()), _iocp_handler, 0, 0) == NULL) {
			LOG_ERROR("IOCP bind socket to io completion port failed!");
			return false;
		}
	}
	((EventOverlapped*)event->_data)->_event = event;
	event->_client_socket->SetInActions(true);
	event->_event_flag_set |= EVENT_READ;
	return _PostRecv(event);
}

bool CIOCP::AddAcceptEvent(CMemSharePtr<CEventHandler>& event) {
	if (!event->_accept_socket->IsInActions()) {
		if (CreateIoCompletionPort((HANDLE)(event->_accept_socket->GetSocket()), _iocp_handler, 0, 0) == NULL) {
			LOG_ERROR("IOCP bind socket to io completion port failed!");
			return false;
		}
	}

	((EventOverlapped*)event->_data)->_event = event;
	event->_accept_socket->SetInActions(true);
	event->_event_flag_set |= EVENT_ACCEPT;
	return _PostAccept(event);
}

bool CIOCP::DelEvent(CMemSharePtr<CEventHandler>& event) {
	return true;
}

void CIOCP::ProcessEvent() {
	DWORD				bytes_transfered = 0;
	EventOverlapped		*socket_context = nullptr;
	OVERLAPPED          *over_lapped = nullptr;
	unsigned int		wait_time = 0;
	std::vector<TimerEvent> timer_vec;
	for (;;) {
		/*wait_time = _timer.TimeoutCheck(timer_vec);
		if (wait_time == 0 && timer_vec.empty()) {
			wait_time = 100;
		}*/
		wait_time = INFINITE;
		int res = GetQueuedCompletionStatus(_iocp_handler, &bytes_transfered, PULONG_PTR(&socket_context),
			&over_lapped, wait_time);

		if (!res) {
			DWORD dwErr = GetLastError();
			if (WAIT_TIMEOUT == GetLastError()) {
				if (!timer_vec.empty()) {
					for (auto iter = timer_vec.begin(); iter != timer_vec.end(); ++iter) {
						if (iter->_event_flag & EVENT_READ) {
							iter->_event->_client_socket->_Recv(iter->_event);

						}
						else if (iter->_event_flag & EVENT_WRITE) {
							iter->_event->_client_socket->_Send(iter->_event);
						}
					}
					timer_vec.clear();
				}

			} else if (ERROR_NETNAME_DELETED !=GetLastError()) {
				LOG_ERROR("IOCP GetQueuedCompletionStatus return error : %d", GetLastError());
				continue;
			}
		}

		if (over_lapped) {
			socket_context = CONTAINING_RECORD(over_lapped, EventOverlapped, _overlapped);
			socket_context->_event->_off_set = bytes_transfered;
		}

		if (socket_context->_event->_event_flag_set & EVENT_READ) {
			socket_context->_event->_client_socket->_Recv(socket_context->_event);

		} else if (socket_context->_event->_event_flag_set & EVENT_WRITE) {
			socket_context->_event->_client_socket->_Send(socket_context->_event);

		} else if (socket_context->_event->_event_flag_set & EVENT_ACCEPT) {
			socket_context->_event->_accept_socket->_Accept(socket_context->_event);
		}
	}
}

bool CIOCP::_PostRecv(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;
	context->Clear();
	OVERLAPPED *lapped = &context->_overlapped;
	int res = WSARecv(event->_client_socket->GetSocket(), &context->_wsa_buf, 1, &dwFlags, &dwBytes, lapped, nullptr);

	if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
		LOG_FATAL("IOCP post recv event failed!");
		return false;
	}
	return true;
}

bool CIOCP::_PostAccept(CMemSharePtr<CEventHandler>& event) {
	if (!__AcceptEx) {
		LOG_ERROR("__AcceptEx function is null!");
		return false;
	}

	EventOverlapped* context = (EventOverlapped*)event->_data;
	context->Clear();
	DWORD dwBytes = 0;
	OVERLAPPED *lapped = &context->_overlapped;
	// Í¶µÝAcceptEx
	if (FALSE == __AcceptEx(event->_accept_socket->GetSocket(), event->_client_socket->GetSocket(), &context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &dwBytes, lapped)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			LOG_ERROR("IOCP post recv accept failed! error code:%d", WSAGetLastError());
			return false;
		}
	}

	return true;
}

bool CIOCP::_PostSend(CMemSharePtr<CEventHandler>& event) {
	EventOverlapped* context = (EventOverlapped*)event->_data;

	context->Clear();
	context->_wsa_buf.len = event->_buffer->Read(context->_lapped_buffer, MAX_BUFFER_LEN);
	
	OVERLAPPED *lapped = &context->_overlapped;
	int res = WSASend(event->_client_socket->GetSocket(), &context->_wsa_buf, 1, nullptr, 0, lapped, nullptr);

	if ((SOCKET_ERROR == res) && (WSA_IO_PENDING != WSAGetLastError())) {
		LOG_FATAL("IOCP post send event failed!");
		return false;
	}
	return true;
}