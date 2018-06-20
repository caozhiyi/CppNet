#ifdef __linux__
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "CEpoll.h"
#include "OSInfo.h"
#include "Log.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "Socket.h"
#include "Timer.h"
#include "LinuxFunc.h"

CEpoll::CEpoll() {

}

CEpoll::~CEpoll() {

}

bool CEpoll::Init() {
	//get epoll handle
	_epoll_handler = epoll_create(1500);
	if (_epoll_handler == -1) {
		LOG_FATAL("epoll init failed! error code:%d", errno);
		return false;
	}
	SetCoreFileUnlimit();
	return true;
}

bool CEpoll::Dealloc() {
	if (close(_epoll_handler) == -1) {
		LOG_ERROR("IOCP close io completion port failed! error code:%d", errno);
	}
	return true;
}

bool CEpoll::AddTimerEvent(unsigned int interval, int event_flag, CMemSharePtr<CEventHandler>& event) {
	_timer.AddTimer(interval, event_flag, event);
	LOG_DEBUG("add a timer event, %d", interval);
	return true;
}

bool CEpoll::AddSendEvent(CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		bool res = false;
		epoll_event* content = (epoll_event*)event->_data;
		//if not add to epoll
		if (!(content->events & EPOLLOUT)) {
			if (socket_ptr->IsInActions()) {
				res = _ModifyEvent(event, EPOLLOUT, socket_ptr->GetSocket());

			} else {
				res = _AddEvent(event, EPOLLOUT, socket_ptr->GetSocket());
			}
		}

		//reset one shot flag
		res = _ReserOneShot(event, socket_ptr->GetSocket());
		socket_ptr->SetInActions(true);
		return res;

	}
	LOG_WARN("write event is already distroyed! in %s", "AddSendEvent");
	return false;
}

bool CEpoll::AddRecvEvent(CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		bool res = false;
		epoll_event* content = (epoll_event*)event->_data;
		//if not add to epoll
		if (!(content->events & EPOLLIN)) {
			if (socket_ptr->IsInActions()) {
				res = _ModifyEvent(event, EPOLLIN, socket_ptr->GetSocket());

			} else {
				res = _AddEvent(event, EPOLLIN, socket_ptr->GetSocket());
			}
		}

		//reset one shot flag
		res = _ReserOneShot(event, socket_ptr->GetSocket());
		socket_ptr->SetInActions(true);
		return res;

	}
	LOG_WARN("read event is already distroyed!in %s", "AddRecvEvent");
	return false;
}

bool CEpoll::AddAcceptEvent(CMemSharePtr<CAcceptEventHandler>& event) {
	bool res = false;
	epoll_event* content = (epoll_event*)event->_data;
	auto socket_ptr = event->_accept_socket;
	//if not add to epoll
	if (!(content->events & EPOLLIN)) {
		res = _AddEvent(event, EPOLLIN, socket_ptr->GetSocket());
	}

	socket_ptr->SetInActions(true);
	return res;
}

bool CEpoll::AddConnection(CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) {
	if (ip.empty()){
		return false;
	}
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		//the socket must not in epoll
		if (socket_ptr->IsInActions()) {
			return false;
		}
		socket_ptr->SetInActions(true);
		
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());

		int res = connect(socket_ptr->GetSocket(), (sockaddr *)&addr, sizeof(addr));
		if (errno == EINPROGRESS) {
			res = _AddEvent(event, EPOLLOUT, socket_ptr->GetSocket());
		}
		if (res == 0) {
			return true;
		}
		LOG_WARN("connect event failed! %d", errno);
		return false;
	}
	LOG_WARN("connection event is already distroyed!,%s", "AddConnection");
	return false;
}

bool CEpoll::AddDisconnection(CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		DelEvent(socket_ptr->GetSocket());
		close(socket_ptr->GetSocket());
	}
	return true;
}

bool CEpoll::DelEvent(unsigned int sock) {
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_DEL, sock, nullptr);
	if (res == -1) {
		LOG_ERROR("remove event from epoll faild! error :%d", errno);
		return false;
	}
	LOG_DEBUG("del a socket from epoll, %d", sock);
	return true;
}

void CEpoll::ProcessEvent() {
	unsigned int		wait_time = 0;
	std::vector<TimerEvent> timer_vec;
	std::vector<epoll_event> event_vec;
	event_vec.resize(1000);
	for (;;) {
		wait_time = _timer.TimeoutCheck(timer_vec);
		//if there is no timer event. wait until recv something
		if (wait_time == 0 && timer_vec.empty()) {
			wait_time = -1;
		}

		int res = epoll_wait(_epoll_handler, &*event_vec.begin(), (int)(event_vec.size()), wait_time);

		if (res == -1) {
			int err = errno;
			LOG_ERROR("epoll_wait faild! error :%d", errno);
		}

		if (res > 0) {
			LOG_DEBUG("epoll_wait get events! num :%d", res);
			_DoEvent(event_vec, res);

		} else {
			if (!timer_vec.empty()) {
				_DoTimeoutEvent(timer_vec);
			}
		}
	}
}

bool CEpoll::_AddEvent(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= event_flag | EPOLLET;
	content->data.ptr = (void*)&event->_client_socket;

	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, sock, content);
	if (res == -1) {
		LOG_ERROR("add event to epoll faild! error :%d", errno);
		return false;
	}
	LOG_DEBUG("add a event to epoll, %d", event->_event_flag_set);
	return true;
}

bool CEpoll::_AddEvent(CMemSharePtr<CAcceptEventHandler>& event, int event_flag, unsigned int sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= event_flag | EPOLLET;
	content->data.ptr = event->_accept_socket;
	content->data.ptr = ((uintptr_t)content->data.ptr) | 1;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, sock, content);
	if (res == -1) {
		LOG_ERROR("add event to epoll faild! error :%d", errno);
		return false;
	}
	LOG_DEBUG("add a event to epoll, %d", event->_event_flag_set);
	return true;
}

bool CEpoll::_ModifyEvent(CMemSharePtr<CEventHandler>& event, int event_flag, unsigned int sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= event_flag;
	content->data.ptr = (void*)&event->_client_socket;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_MOD, sock, content);
	if (res == -1) {
		LOG_ERROR("modify event to epoll faild! error :%d", errno);
		return false;
	}
	LOG_DEBUG("modify a event to epoll, %d", event->_event_flag_set);
	return true;
}

bool CEpoll::_ReserOneShot(CMemSharePtr<CEventHandler>& event, unsigned int sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= EPOLLONESHOT;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_MOD, sock, content);
	if (res == -1) {
		LOG_ERROR("reset one shot flag faild! error :%d", errno);
		return false;
	}
	LOG_DEBUG("reset one shot, %d", event->_event_flag_set);
	return true;
}

void CEpoll::_DoTimeoutEvent(std::vector<TimerEvent>& timer_vec) {
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

void CEpoll::_DoEvent(std::vector<epoll_event>& event_vec, int num) {
	CMemWeakPtr<CSocket>* normal_sock = nullptr;
	CAcceptSocket* accept_sock = nullptr;
	void* event = nullptr;
	for (int i = 0; i < num; i++) {
		event = event_vec[i].data.ptr;
		if (!event) {
			LOG_WARN("the event is nullptr, index : %d", i);
			continue;
		}
		if (((uintptr_t)event) & 1) {
			event = (void*)(((uintptr_t)event) & (uintptr_t)~1);
			accept_sock = (CAcceptSocket*)event;
			accept_sock->_Accept(accept_sock->_accept_event);
		
		} else {
			normal_sock = (CMemWeakPtr<CSocket>*)event_vec[i].data.ptr;
			auto socket_ptr = normal_sock->Lock();
			if (event_vec[i].events & EPOLLIN) {
				if (socket_ptr) {
					LOG_DEBUG("get a epollin event, index : %d", i);
					socket_ptr->_Recv(socket_ptr->_read_event);
				}

			} else if (event_vec[i].events & EPOLLOUT) {
				auto socket_ptr = normal_sock->Lock();
				if (socket_ptr) {
					LOG_DEBUG("get a epollout event, index : %d", i);
					socket_ptr->_Send(socket_ptr->_write_event);
				}
			}
		}
	}
}
#endif // __linux__
