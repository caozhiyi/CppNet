#include <random>

#include "NetObject.h"
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

CNetObject::CNetObject() {

}

CNetObject::~CNetObject() {

}

void CNetObject::Init(int thread_num) {
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

void CNetObject::Dealloc() {
	for (auto iter = _actions_map.begin(); iter != _actions_map.end(); ++iter) {
		iter->second->Dealloc();
	}
	_actions_map.clear();
#ifndef __linux__
	DeallocSocket();
#endif // __linux__
}

void CNetObject::MainLoop() {
	for (;;) {
		CRunnable::Sleep(50000);
	}
}

void CNetObject::Join() {
	for (size_t i = 0; i < _thread_vec.size(); ++i) {
		_thread_vec[i]->join();
	}
}

void CNetObject::SetReadCallback(const call_back& func) {
	_read_call_back = func;
}

void CNetObject::SetWriteCallback(const call_back& func) {
	_write_call_back = func;
}

void CNetObject::SetDisconnectionCallback(const call_back& func) {
	_disconnection_call_back = func;
}

void CNetObject::SetAcceptCallback(const call_back& func) {
	_accept_call_back = func;
}

bool CNetObject::ListenAndAccept(int port, std::string ip) {
	if (!_accept_call_back) {
		LOG_WARN("accept call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return false;
	}

	if (!_read_call_back) {
		LOG_WARN("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return false;
	}

	if (_actions_map.size() <= 0) {
		LOG_WARN("CNetObject obj is not inited!, port : %d, ip : %s ", port, ip.c_str());
		return false;
	}

	for (auto iter = _actions_map.begin(); iter != _actions_map.end(); ++iter) {
		CMemSharePtr<CAcceptSocket> accept_socket = MakeNewSharedPtr<CAcceptSocket>(&_pool, _actions_map.begin()->second);
		if (!accept_socket->Bind(port, ip)) {
			return false;
		}

		if (!accept_socket->Listen(100)) {
			return false;
		}

		accept_socket->SetAcceptCallBack(std::bind(&CNetObject::_AcceptFunction, this, std::placeholders::_1, std::placeholders::_2));
		accept_socket->SetReadCallBack(std::bind(&CNetObject::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
		
		accept_socket->SyncAccept();
		_accept_socket[accept_socket->GetSocket()] = accept_socket;
#ifndef __linux__
		break;
#endif
	}
	return true;
}

void CNetObject::SetConnectionCallback(const call_back& func) {
	_connection_call_back = func;
}

CMemSharePtr<CSocket> CNetObject::Connection(int port, std::string ip, char* buf, int buf_len) {
	if (!_connection_call_back) {
		LOG_WARN("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}
	if (!_write_call_back) {
		LOG_WARN("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}

	auto actions = RandomGetActions();
	CMemSharePtr<CSocket> sock = MakeNewSharedPtr<CSocket>(&_pool, actions);
	sock->SetWriteCallBack(std::bind(&CNetObject::_WriteFunction, this, std::placeholders::_1, std::placeholders::_2));

#ifndef __linux__
	sock->SetReadCallBack(std::bind(&CNetObject::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
	sock->SyncConnection(ip, port, buf, buf_len);
#else
	auto func = [buf, buf_len, sock, this](CMemSharePtr<CEventHandler>& event, int err) {
		if (err = EVENT_ERROR_NO) {
			sock->SyncWrite(buf, buf_len);
		}
		sock->SetReadCallBack(std::bind(&CNetObject::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
	};
	sock->SetReadCallBack(func);
#endif
	return sock;
}

CMemSharePtr<CSocket> CNetObject::Connection(int port, std::string ip) {
	if (!_connection_call_back) {
		LOG_WARN("connection call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}
	if (!_write_call_back) {
		LOG_WARN("read call back function is null!, port : %d, ip : %s ", port, ip.c_str());
		return nullptr;
	}

	auto actions = RandomGetActions();
	CMemSharePtr<CSocket> sock = MakeNewSharedPtr<CSocket>(&_pool, actions);
	sock->SetWriteCallBack(std::bind(&CNetObject::_WriteFunction, this, std::placeholders::_1, std::placeholders::_2));
	sock->SetReadCallBack(std::bind(&CNetObject::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));

#ifndef __linux__
	sock->SyncConnection(ip, port, "", 0);
#else
	sock->SyncConnection(ip, port);
#endif
	return sock;
}

void CNetObject::_AcceptFunction(CMemSharePtr<CAcceptEventHandler>& event, int err) {
	if (!event) {
		return;
	}
	
	auto socket_ptr = event->_client_socket;
	if (err & EVENT_ERROR_NO) {
		socket_ptr->SetReadCallBack(std::bind(&CNetObject::_ReadFunction, this, std::placeholders::_1, std::placeholders::_2));
		socket_ptr->SetWriteCallBack(std::bind(&CNetObject::_WriteFunction, this, std::placeholders::_1, std::placeholders::_2));
		if (_accept_call_back) {
			_accept_call_back(socket_ptr, err);
		}
		static int num = 0;
		num++;
		LOG_ERROR("get client num : %d", num);

		std::unique_lock<std::mutex> lock(_mutex);
		_socket_map[event->_client_socket->GetSocket()] = event->_client_socket;
	}
}

void CNetObject::_ReadFunction(CMemSharePtr<CEventHandler>& event, int err) {
	if (!event) {
		return;
	}
	auto socket_ptr = event->_client_socket.Lock();
	if (err & EVENT_READ && _read_call_back) {
		err &= ~EVENT_READ;
		_read_call_back(socket_ptr, err);
		if (err == EVENT_ERROR_CLOSED) {
			std::unique_lock<std::mutex> lock(_mutex);
			_socket_map.erase(socket_ptr->GetSocket());
		}

	} else if (err & EVENT_CONNECT && _connection_call_back) {
		err &= ~EVENT_CONNECT;
		_connection_call_back(socket_ptr, err);
	} else if (err & EVENT_DISCONNECT && _disconnection_call_back) {
		err &= ~EVENT_DISCONNECT;
		_disconnection_call_back(socket_ptr, err);
	}
}

void CNetObject::_WriteFunction(CMemSharePtr<CEventHandler>& event, int err) {
	if (!event) {
		return;
	}
	auto socket_ptr = event->_client_socket.Lock();
	if (err & EVENT_WRITE && _write_call_back) {
		err &= ~EVENT_WRITE;
		_write_call_back(socket_ptr, err);
		if (err == EVENT_ERROR_CLOSED) {
			std::unique_lock<std::mutex> lock(_mutex);
			_socket_map.erase(socket_ptr->GetSocket());
		}
	}
}

std::shared_ptr<CEventActions>& CNetObject::RandomGetActions() {
	std::random_device rd;
	std::mt19937 mt(rd());
	int index = mt() % int(_actions_map.size());
	auto iter = _actions_map.begin();
	for (int i = 0; i < index; i++) {
		iter++;
	}
	return iter->second;
}