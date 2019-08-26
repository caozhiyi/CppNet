#ifdef __linux__
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include "CEpoll.h"
#include "OSInfo.h"
#include "Log.h"
#include "EventHandler.h"
#include "Buffer.h"
#include "Socket.h"
#include "Timer.h"
#include "LinuxFunc.h"

using namespace cppnet;

enum EPOLL_CODE {
	EXIT_EPOLL = 1,
	WEAK_EPOLL = 0
};

CEpoll::CEpoll(bool only_one_epoll) : _run(true), _only_one_epoll(only_one_epoll) {

}

CEpoll::~CEpoll() {

}

bool CEpoll::Init(uint32_t thread_num) {
	//Disable  SIGPIPE signal
	sigset_t set;
	sigprocmask(SIG_SETMASK, NULL, &set);
	sigaddset(&set, SIGPIPE);
	sigprocmask(SIG_SETMASK, &set, NULL);
	//get epoll handle. the param is invalid since linux 2.6.8
	_epoll_handler = epoll_create(1500);
	if (_epoll_handler == -1) {
        base::LOG_FATAL("epoll init failed! error : %d", errno);
		return false;
	}
	if (pipe((int*)_pipe) == -1) {
        base::LOG_FATAL("pipe init failed! error : %d", errno);
		return false;
	}
	_pipe_content.events |= EPOLLIN;
	_pipe_content.data.ptr = (void*)WEAK_EPOLL;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, _pipe[0], &_pipe_content);
	if (res == -1) {
        base::LOG_ERROR("add event to epoll faild! error :%d", errno);
		return false;
	}
	return true;
}

bool CEpoll::Dealloc() {
	_run = false;
	_pipe_content.events |= EPOLLIN;
	_pipe_content.data.ptr = (void*)EXIT_EPOLL;
	epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, _pipe[0], &_pipe_content);
	WakeUp();
	return true;
}

uint64_t CEpoll::AddTimerEvent(uint32_t interval, const timer_call_back& call_back, void* param, bool always) {
    return _timer.AddTimer(interval, call_back, param, always);
}

bool CEpoll::RemoveTimerEvent(uint64_t timer_id) {
    return _timer.DelTimer(timer_id);
}

bool CEpoll::AddTimerEvent(uint32_t interval, base::CMemSharePtr<CEventHandler>& event) {
	_timer.AddTimer(interval, event);
    base::LOG_DEBUG("add a timer event, %d", interval);
	return true;
}

bool CEpoll::AddSendEvent(base::CMemSharePtr<CEventHandler>& event) {
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
		res = _ReserOneShot(event, EPOLLOUT, socket_ptr->GetSocket());
		socket_ptr->SetInActions(true);
		return res;

	}
    base::LOG_WARN("write event is already distroyed! in %s", "AddSendEvent");
	return false;
}

bool CEpoll::AddRecvEvent(base::CMemSharePtr<CEventHandler>& event) {
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
		res = _ReserOneShot(event, EPOLLIN, socket_ptr->GetSocket());
		if (res) {
			socket_ptr->SetInActions(true);
		}
		return res;

	}
	base::LOG_WARN("read event is already distroyed!in AddRecvEvent");
	return false;
}

bool CEpoll::AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event) {
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

bool CEpoll::AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) {
	if (ip.empty()) {
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
		//block here in linux
		int res = connect(socket_ptr->GetSocket(), (sockaddr *)&addr, sizeof(addr));
		SetSocketNoblocking(socket_ptr->GetSocket());

		if (res == 0 || errno == EINPROGRESS) {
			//res = _AddEvent(event, EPOLLOUT, socket_ptr->GetSocket());
			socket_ptr->_Recv(socket_ptr->_read_event);
			return true;
		}
        base::LOG_WARN("connect event failed! %d", errno);
		return false;
	}
    base::LOG_WARN("connection event is already destroyed!,%s", "AddConnection");
	return false;
}

bool CEpoll::AddDisconnection(base::CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (socket_ptr) {
		if (DelEvent(event)) {
			close(socket_ptr->GetSocket());
			socket_ptr->_Recv(socket_ptr->_read_event);
		}
	}
	return true;
}

bool CEpoll::DelEvent(base::CMemSharePtr<CEventHandler>& event) {
	auto socket_ptr = event->_client_socket.Lock();
	if (!socket_ptr) {
		return false;
	}
	epoll_event* content = (epoll_event*)event->_data;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_DEL, socket_ptr->GetSocket(), content);
	if (res == -1) {
        base::LOG_ERROR("remove event from epoll faild! error :%d, socket : %d", errno, socket_ptr->GetSocket());
		return false;
	}
    base::LOG_DEBUG("del a socket from epoll, %d", socket_ptr->GetSocket());
	return true;
}

void CEpoll::ProcessEvent() {
	uint32_t		wait_time = 0;
    std::vector<base::CMemSharePtr<CTimerEvent>> timer_vec;;
	std::vector<epoll_event> event_vec;
	event_vec.resize(1000);
	while (_run) {
		wait_time = _timer.TimeoutCheck(timer_vec);
		//if there is no timer event. wait until recv something
        if (wait_time == 0 && timer_vec.empty()) {
            wait_time = -1;
        } else {
            wait_time = wait_time > 0 ? wait_time : 1;
        }

		int res = epoll_wait(_epoll_handler, &*event_vec.begin(), (int)(event_vec.size()), wait_time);
		if (res == -1) {
            base::LOG_ERROR("epoll_wait faild! error :%d", errno);
		}

		if (res > 0) {
            base::LOG_DEBUG("epoll_wait get events! num :%d, TheadId : %d", res, std::this_thread::get_id());

			_DoEvent(event_vec, res);
			_DoTaskList();

		} else {
			if (!timer_vec.empty()) {
				_DoTimeoutEvent(timer_vec);
			}
			_DoTaskList();
		}
	}

	if (close(_epoll_handler) == -1) {
        base::LOG_ERROR("epoll close failed! error : %d", errno);
	}
	if (close(_pipe[0]) == -1) {
        base::LOG_ERROR("_pipe[0] close failed! error : %d", errno);
	}
	if (close(_pipe[1]) == -1) {
        base::LOG_ERROR("_pipe[1] close failed! error : %d", errno);
	}
    base::LOG_INFO("return the net io thread");
}

void CEpoll::PostTask(std::function<void(void)>& task) {
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_task_list.push_back(task);
	}
	WakeUp();
}

void CEpoll::WakeUp() {
	write(_pipe[1], "0", 1);
}

bool CEpoll::_AddEvent(base::CMemSharePtr<CEventHandler>& event, int32_t event_flag, uint64_t sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= event_flag | EPOLLET;
	content->data.ptr = (void*)&event->_client_socket;

	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, sock, content);
	if (res == -1) {
		if (errno == EEXIST) {
			res = _ModifyEvent(event, event_flag, sock);
		}
		if (res == -1) {
            base::LOG_ERROR("add event to epoll faild! error :%d, sock: %d", errno, sock);
			return false;
		}
	}
    base::LOG_DEBUG("add a event to epoll, event : %d, sock : %d", event->_event_flag_set, sock);
	return true;
}

bool CEpoll::_AddEvent(base::CMemSharePtr<CAcceptEventHandler>& event, int32_t event_flag, uint64_t sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= event_flag | EPOLLET;
	content->data.ptr = (void*)&event->_accept_socket;
	content->data.ptr = (void*)(((uintptr_t)content->data.ptr) | 1);
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, sock, content);
	if (res == -1) {
        base::LOG_ERROR("add event to epoll faild! error :%d, sock: %d", errno, sock);
		return false;
	}
    base::LOG_DEBUG("add a event to epoll, event flag: %d, sock : %d", event->_event_flag_set, sock);
	return true;
}

bool CEpoll::_ModifyEvent(base::CMemSharePtr<CEventHandler>& event, int32_t event_flag, uint64_t sock) {
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= event_flag;
	content->data.ptr = (void*)&event->_client_socket;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_MOD, sock, content);
	if (res == -1) {
		if (errno == ENOENT) {
			res = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, sock, content);
		}
		if (res == -1) {
            base::LOG_ERROR("modify event to epoll faild! error :%d, sock: %d", errno, sock);
			return false;
		}
	}
    base::LOG_DEBUG("modify a event to epoll, event flag: %d, sock : %d", event->_event_flag_set, sock);
	return true;
}

bool CEpoll::_ReserOneShot(base::CMemSharePtr<CEventHandler>& event, int32_t event_flag, uint64_t sock) {
	// if not only epoll handle, do nothing. 
	if (!_only_one_epoll) {
		return true;
	}
	epoll_event* content = (epoll_event*)event->_data;
	content->events |= EPOLLONESHOT;
	int res = epoll_ctl(_epoll_handler, EPOLL_CTL_MOD, sock, content);
	if (res == -1) {
		if (errno == ENOENT) {
			res = _ModifyEvent(event, EPOLLONESHOT | event_flag, sock);
		}
		if (res == -1) {
            base::LOG_ERROR("reset one shot flag faild! error :%d, sock: %d", errno, sock);
			return false;
		}
	}
    base::LOG_DEBUG("reset one shot, event flag: %d, sock : %d", event->_event_flag_set, sock);
	return true;
}

void CEpoll::_DoTimeoutEvent(std::vector<base::CMemSharePtr<CTimerEvent>>& timer_vec) {
    for (auto iter = timer_vec.begin(); iter != timer_vec.end(); ++iter) {
        if ((*iter)->_event_flag & EVENT_READ) {
            base::CMemSharePtr<CEventHandler> event_ptr = (*iter)->_event.Lock();
            base::CMemSharePtr<CSocketImpl> socket_ptr = event_ptr->_client_socket.Lock();
            if (socket_ptr) {
                event_ptr->_event_flag_set |= EVENT_TIMER;
                socket_ptr->_Recv(event_ptr);
            }

        } else if ((*iter)->_event_flag & EVENT_WRITE) {
            base::CMemSharePtr<CEventHandler> event_ptr = (*iter)->_event.Lock();
            base::CMemSharePtr<CSocketImpl> socket_ptr = event_ptr->_client_socket.Lock();
            if (socket_ptr) {
                event_ptr->_event_flag_set |= EVENT_TIMER;
                socket_ptr->_Send(event_ptr);
            }

        } else if ((*iter)->_event_flag & EVENT_TIMER) {
            auto func = (*iter)->_timer_call_back;
            if (func) {
                func((*iter)->_timer_param);
            }
        }
    }
    timer_vec.clear();
}

void CEpoll::_DoEvent(std::vector<epoll_event>& event_vec, int num) {
    base::CMemWeakPtr<CSocketImpl>* normal_sock = nullptr;
    base::CMemSharePtr<CAcceptSocket>* accept_sock = nullptr;
	void* sock = nullptr;
	for (int i = 0; i < num; i++) {
		if (&_pipe_content == &event_vec[i] && event_vec[i].data.u32 == EXIT_EPOLL) {
			_run = false;
		}
		sock = event_vec[i].data.ptr;
		if (sock == (void*)EXIT_EPOLL) {
			_run = false;
			continue;
		}
		if (!sock) {
			base::LOG_WARN("the event is nullptr, index : %d", i);
            _run = false;
			continue;
		}
		if (((uintptr_t)sock) & 1) {
			sock = (void*)(((uintptr_t)sock) & (uintptr_t)~1);
			accept_sock = (base::CMemSharePtr<CAcceptSocket>*)sock;
			(*accept_sock)->_Accept((*accept_sock)->_accept_event);

		} else {
			normal_sock = (base::CMemWeakPtr<CSocketImpl>*)event_vec[i].data.ptr;
			if (!normal_sock) {
				continue;
			}
			auto socket_ptr = normal_sock->Lock();
			if (!socket_ptr) {
				continue;
			}
			if (event_vec[i].events & EPOLLIN) {
				if (socket_ptr) {
					socket_ptr->_Recv(socket_ptr->_read_event);
				}

			} else if (event_vec[i].events & EPOLLOUT) {
				auto socket_ptr = normal_sock->Lock();
				if (socket_ptr) {
					socket_ptr->_Send(socket_ptr->_write_event);
				}
			}
		}
	}
}

void CEpoll::_DoTaskList() {
	std::vector<std::function<void(void)>> func_vec;
	{
		std::unique_lock<std::mutex> lock(_mutex);
		func_vec.swap(_task_list);
	}

	for (size_t i = 0; i < func_vec.size(); ++i) {
		func_vec[i]();
	}
}
#endif // __linux__
