#include <random>

#include "CppNetImpl.h"
#include "EventActions.h"
#include "OSInfo.h"
#include "Log.h"
#include "Runnable.h"
#ifdef __linux__
#include "CEpoll.h"
#include "LinuxFunc.h"
#else
#include "IOCP.h"
#endif

using namespace cppnet;

CCppNetImpl::CCppNetImpl() {

}

CCppNetImpl::~CCppNetImpl() {

}

void CCppNetImpl::Init(int thread_num) {
#ifndef __linux__
	InitScoket();
#else
	SetCoreFileUnlimit();
#endif // __linux__

	int cpus = GetCpuNum();
	if (thread_num == 0 || thread_num > cpus * 2) {
		thread_num = cpus;
	}
	for (int i = 0; i < thread_num; i++) {
#ifdef __linux__
		std::shared_ptr<CEventActions> event_actions(new CEpoll);
#else
		static std::shared_ptr<CEventActions> event_actions(new CIOCP);
		//std::shared_ptr<CEventActions> event_actions(new CIOCP);
#endif
		event_actions->Init();
		std::shared_ptr<std::thread> thd(new std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
		_actions_map[thd->get_id()] = event_actions;
		_thread_vec.push_back(thd);
	}
}

void CCppNetImpl::Dealloc() {
	for (auto iter = _actions_map.begin(); iter != _actions_map.end(); ++iter) {
		iter->second->Dealloc();
	}
#ifndef __linux__
	DeallocSocket();
#endif // __linux__
}

void CCppNetImpl::Join() {
	for (size_t i = 0; i < _thread_vec.size(); ++i) {
        if (_thread_vec[i]) {
            _thread_vec[i]->join();
        }
	}
    _thread_vec.clear();
    _actions_map.clear();
}

void CCppNetImpl::SetReadCallback(const call_back& func) {
	_read_call_back = func;
}

void CCppNetImpl::SetWriteCallback(const call_back& func) {
	_write_call_back = func;
}

void CCppNetImpl::SetDisconnectionCallback(const call_back& func) {
	_disconnection_call_back = func;
}

uint64_t CCppNetImpl::SetTimer(unsigned int interval, const std::function<void(void*)>& func, void* param, bool always) {

    auto actions = _RandomGetActions();

    uint64_t timer_id = 0;
    timer_id = actions->AddTimerEvent(interval, func, param, always);
    actions->WakeUp();

    _timer_actions_map[timer_id] = actions;

    return timer_id;
}

void CCppNetImpl::RemoveTimer(uint64_t timer_id) {
    auto iter = _timer_actions_map.find(timer_id);
    if (iter != _timer_actions_map.end()) {
        auto actions = iter->second.lock();
        if (actions) {
            actions->RemoveTimerEvent(timer_id);
        }
    }
}

void CCppNetImpl::SetAcceptCallback(const call_back& func) {
	_accept_call_back = func;
}

bool CCppNetImpl::ListenAndAccept(int port, std::string ip) {
	if (!_accept_call_back) {
        base::LOG_ERROR("accept call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return false;
	}

	if (!_read_call_back) {
        base::LOG_ERROR("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return false;
	}

	if (_actions_map.size() <= 0) {
        base::LOG_ERROR("CCppNetImpl obj is not inited!, port : %d, ip : %s ", port, ip.c_str());
		return false;
	}

	for (auto iter = _actions_map.begin(); iter != _actions_map.end(); ++iter) {
        base::CMemSharePtr<CAcceptSocket> accept_socket = base::MakeNewSharedPtr<CAcceptSocket>(&_pool, _actions_map.begin()->second);
		if (!accept_socket->Bind(port, ip)) {
			return false;
		}

		if (!accept_socket->Listen(20)) {
			return false;
		}

		accept_socket->SetAcceptCallBack(std::bind(&CCppNetImpl::_AcceptFunction, this, std::placeholders::_1, std::placeholders::_2));
		accept_socket->SetReadCallBack(std::bind(&CCppNetImpl::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
		
		accept_socket->SyncAccept();
		_accept_socket[accept_socket->GetSocket()] = accept_socket;
#ifndef __linux__
		break;
#endif
	}
	return true;
}

void CCppNetImpl::SetConnectionCallback(const call_back& func) {
	_connection_call_back = func;
}

base::CMemSharePtr<CSocket> CCppNetImpl::Connection(int port, std::string ip, char* buf, int buf_len) {
	if (!_connection_call_back) {
        base::LOG_WARN("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}
	if (!_write_call_back) {
        base::LOG_WARN("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}

	auto actions = _RandomGetActions();
    base::CMemSharePtr<CSocket> sock = base::MakeNewSharedPtr<CSocket>(&_pool, actions);
	sock->SetWriteCallBack(std::bind(&CCppNetImpl::_WriteFunction, this, std::placeholders::_1, std::placeholders::_2));

#ifndef __linux__
	sock->SetReadCallBack(std::bind(&CCppNetImpl::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
	sock->SyncConnection(ip, port, buf, buf_len);
#else
	auto func = [buf, buf_len, sock, this](CMemSharePtr<CEventHandler>& event, int err) {
		if (err & EVENT_ERROR_NO) {
			sock->SyncWrite(buf, buf_len);
		}
		sock->SetReadCallBack(std::bind(&CCppNetImpl::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
		_ReadFunction(event, err);
	};
	sock->SetReadCallBack(func);
	sock->SyncConnection(ip, port);
#endif
	return sock;
}

base::CMemSharePtr<CSocket> CCppNetImpl::Connection(int port, std::string ip) {
	if (!_connection_call_back) {
        base::LOG_WARN("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}
	if (!_write_call_back) {
        base::LOG_WARN("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}

	auto actions = _RandomGetActions();
    base::CMemSharePtr<CSocket> sock = base::MakeNewSharedPtr<CSocket>(&_pool, actions);
	sock->SetWriteCallBack(std::bind(&CCppNetImpl::_WriteFunction, this, std::placeholders::_1, std::placeholders::_2));
	sock->SetReadCallBack(std::bind(&CCppNetImpl::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));

#ifndef __linux__
	sock->SyncConnection(ip, port, "", 0);
#else
	sock->SyncConnection(ip, port);
#endif
	return sock;
}

void CCppNetImpl::_AcceptFunction(base::CMemSharePtr<CAcceptEventHandler>& event, int err) {
	if (!event) {
        base::LOG_WARN("event is null while accept.");
		return;
	}
	
	auto socket_ptr = event->_client_socket;
	if (err & EVENT_ERROR_NO) {
		socket_ptr->SetReadCallBack(std::bind(&CCppNetImpl::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
		socket_ptr->SetWriteCallBack(std::bind(&CCppNetImpl::_WriteFunction, this, std::placeholders::_1, std::placeholders::_2));
		if (_accept_call_back) {
			_accept_call_back(socket_ptr, err);
		}

        base::LOG_DEBUG("get client num : %d", int(_socket_map.size()));

		std::unique_lock<std::mutex> lock(_mutex);
		_socket_map[event->_client_socket->GetSocket()] = event->_client_socket;
	}
}

void CCppNetImpl::_ReadFunction(base::CMemSharePtr<CEventHandler>& event, int err) {
	if (!event) {
        base::LOG_WARN("event is null while read.");
		return;
	}
	auto socket_ptr = event->_client_socket.Lock();
	if (err & EVENT_CONNECT && _connection_call_back) {
		err &= ~EVENT_CONNECT;
		_connection_call_back(socket_ptr, err);

	} else if (err & EVENT_DISCONNECT && _disconnection_call_back) {
		err &= ~EVENT_DISCONNECT;
		_disconnection_call_back(socket_ptr, err);

	} else if (err & EVENT_READ && _read_call_back) {
		err &= ~EVENT_READ;
		_read_call_back(socket_ptr, err);
		if (err == EVENT_ERROR_CLOSED) {
			std::unique_lock<std::mutex> lock(_mutex);
			_socket_map.erase(socket_ptr->GetSocket());
		}
	}
}

void CCppNetImpl::_WriteFunction(base::CMemSharePtr<CEventHandler>& event, int err) {
	if (!event) {
        base::LOG_WARN("event is null while write.");
		return;
	}

	auto socket_ptr = event->_client_socket.Lock();
	if (err & EVENT_WRITE && _write_call_back) {
		err &= ~EVENT_WRITE;
		if (event->_buffer->GetCanReadSize() == 0) {
			err |= EVENT_ERROR_DONE;
		}
		_write_call_back(socket_ptr, err);
		if (err & EVENT_ERROR_CLOSED) {
			std::unique_lock<std::mutex> lock(_mutex);
			_socket_map.erase(socket_ptr->GetSocket());
		}
	}
}

std::shared_ptr<CEventActions>& CCppNetImpl::_RandomGetActions() {
	std::random_device rd;
	std::mt19937 mt(rd());
	int index = mt() % int(_actions_map.size());
	auto iter = _actions_map.begin();
	for (int i = 0; i < index; i++) {
		iter++;
	}
	return iter->second;
}