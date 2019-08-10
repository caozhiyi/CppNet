#include <random>

#include "CppNetImpl.h"
#include "EventActions.h"
#include "OSInfo.h"
#include "Log.h"
#include "Runnable.h"
#include "SocketImpl.h"
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

void CCppNetImpl::Init(uint32_t thread_num) {
#ifndef __linux__
	InitScoket();
#else
	SetCoreFileUnlimit();
#endif // __linux__

	uint32_t cpus = GetCpuNum();
	if (thread_num == 0 || thread_num > cpus * 2) {
		thread_num = cpus;
	}
	for (size_t i = 0; i < thread_num; i++) {
#ifdef __linux__
		std::shared_ptr<CEventActions> event_actions(new CEpoll);
#else
        // only one iocp
		static std::shared_ptr<CEventActions> event_actions(new CIOCP);
		//std::shared_ptr<CEventActions> event_actions(new CIOCP);
#endif
        // start net io thread
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

void CCppNetImpl::SetReadCallback(const read_call_back& func) {
	_read_call_back = func;
}

void CCppNetImpl::SetWriteCallback(const write_call_back& func) {
	_write_call_back = func;
}

void CCppNetImpl::SetDisconnectionCallback(const connection_call_back& func) {
	_disconnection_call_back = func;
}

uint64_t CCppNetImpl::SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param, bool always) {

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

void CCppNetImpl::SetAcceptCallback(const connection_call_back& func) {
	_accept_call_back = func;
}

bool CCppNetImpl::ListenAndAccept(uint16_t port, std::string ip, uint32_t listen_num) {
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

		if (!accept_socket->Listen(listen_num)) {
			return false;
		}

		accept_socket->SyncAccept();
		_accept_socket[accept_socket->GetSocket()] = accept_socket;
#ifndef __linux__
		break;
#endif
	}
	return true;
}

void CCppNetImpl::SetConnectionCallback(const connection_call_back& func) {
	_connection_call_back = func;
}

#ifndef __linux__
Handle CCppNetImpl::Connection(uint16_t port, std::string ip, const char* buf, uint32_t buf_len) {
	if (!_connection_call_back) {
        base::LOG_ERROR("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return 0;
	}
	if (!_write_call_back) {
        base::LOG_ERROR("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return 0;
	}

	auto actions = _RandomGetActions();
    base::CMemSharePtr<CSocketImpl> sock = base::MakeNewSharedPtr<CSocketImpl>(&_pool, actions);
	sock->SyncConnection(ip, port, buf, buf_len);

    std::unique_lock<std::mutex> lock(_mutex);
    _socket_map[sock->GetSocket()] = sock;
	return sock->GetSocket();
}
#endif

Handle CCppNetImpl::Connection(uint16_t port, std::string ip) {
	if (!_connection_call_back) {
        base::LOG_ERROR("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return 0;
	}
	if (!_write_call_back) {
        base::LOG_ERROR("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return 0;
	}

	auto actions = _RandomGetActions();
    base::CMemSharePtr<CSocketImpl> sock = base::MakeNewSharedPtr<CSocketImpl>(&_pool, actions);

#ifndef __linux__
	sock->SyncConnection(ip, port, "", 0);
#else
	sock->SyncConnection(ip, port);
#endif
    std::unique_lock<std::mutex> lock(_mutex);
    _socket_map[sock->GetSocket()] = sock;
    return sock->GetSocket();
}

base::CMemSharePtr<CSocketImpl> CCppNetImpl::GetSocket(const Handle& handle) {
	std::unique_lock<std::mutex> lock(_mutex);
	auto iter = _socket_map.find(handle);
	if (iter != _socket_map.end()) {
		return iter->second;
	}
	return nullptr;
}

bool CCppNetImpl::RemoveSocket(const Handle& handle) {
    std::unique_lock<std::mutex> lock(_mutex);
    auto iter = _socket_map.find(handle);
    if (iter != _socket_map.end()) {
        _socket_map.erase(iter);
        return true;
    }
    return false;
}

uint32_t CCppNetImpl::GetThreadNum() {
    return (uint32_t)_thread_vec.size();
}

void CCppNetImpl::_AcceptFunction(base::CMemSharePtr<CAcceptEventHandler>& event, uint32_t err) {
	if (!event) {
        base::LOG_WARN("event is null while accept.");
		return;
	}
	
	Handle handle = event->_client_socket->GetSocket();
	auto socket_ptr = event->_client_socket;
	if (err & EVENT_ERROR_NO) {
		{
			// add socket to map
			std::unique_lock<std::mutex> lock(_mutex);
			_socket_map[handle] = event->_client_socket;
		}
		if (_accept_call_back) {
			_accept_call_back(handle, err);
		}

        base::LOG_DEBUG("get client num : %d", int(_socket_map.size()));
	}
}

void CCppNetImpl::_ReadFunction(base::CMemSharePtr<CEventHandler>& event, uint32_t err) {
	if (!event) {
        base::LOG_WARN("event is null while read.");
		return;
	}
	auto socket_ptr = event->_client_socket.Lock();
	Handle handle = socket_ptr->GetSocket();
	if (err & EVENT_CONNECT && _connection_call_back) {
		err &= ~EVENT_CONNECT;
		_connection_call_back(handle, err);

	} else if (err & EVENT_DISCONNECT && _disconnection_call_back) {
		err &= ~EVENT_DISCONNECT;
		_disconnection_call_back(handle, err);

	} else if (err & EVENT_READ && _read_call_back) {
		err &= ~EVENT_READ;
		_read_call_back(handle, socket_ptr->_read_event->_buffer.Get(), socket_ptr->_read_event->_off_set, err);
		if (err == EVENT_ERROR_CLOSED) {
			std::unique_lock<std::mutex> lock(_mutex);
			_socket_map.erase(socket_ptr->GetSocket());
		}
	}
}

void CCppNetImpl::_WriteFunction(base::CMemSharePtr<CEventHandler>& event, uint32_t err) {
	if (!event) {
        base::LOG_WARN("event is null while write.");
		return;
	}

	auto socket_ptr = event->_client_socket.Lock();
	Handle handle = socket_ptr->GetSocket();
	if (err & EVENT_WRITE && _write_call_back) {
		err &= ~EVENT_WRITE;
		if (event->_buffer->GetCanReadSize() == 0) {
			err |= EVENT_ERROR_DONE;
		}
		_write_call_back(handle, socket_ptr->_read_event->_off_set, err);
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