#ifndef __linux__
#include <winsock2.h>
#include "SocketBase.h"
#include "WinExpendFunc.h"
#include "Log.h"

LPFN_ACCEPTEX				__AcceptEx = nullptr;
LPFN_CONNECTEX				__ConnectEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS	__AcceptExScokAddrs = nullptr;
LPFN_DISCONNECTEX			__DisconnectionEx;

static void* _GetExFunctnion(unsigned int socket, const GUID& which) {
	void* func = nullptr;
	DWORD bytes = 0;
	WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, (LPVOID)&which,
		sizeof(which), &func, sizeof(func), &bytes, NULL, NULL);

	return func;
}

static bool _InitExFunctnion() {
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	__AcceptEx = (LPFN_ACCEPTEX)_GetExFunctnion(socket, WSAID_ACCEPTEX);
	__ConnectEx = (LPFN_CONNECTEX)_GetExFunctnion(socket, WSAID_CONNECTEX);
	__AcceptExScokAddrs = (LPFN_GETACCEPTEXSOCKADDRS)_GetExFunctnion(socket, WSAID_GETACCEPTEXSOCKADDRS);
	__DisconnectionEx = (LPFN_DISCONNECTEX)_GetExFunctnion(socket, WSAID_DISCONNECTEX);
	closesocket(socket);
	if (!__AcceptExScokAddrs || !__ConnectEx || !__AcceptEx || !__DisconnectionEx) {
		return false;
	}
	return true;
}

void SetReusePort(unsigned int sock) {
	int opt = 1;
	int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

bool InitScoket() {
	static WSADATA __wsa_data;
	static bool __has_init = false;
	if (!__has_init && WSAStartup(MAKEWORD(2, 2), &__wsa_data) != 0) {
		LOG_FATAL("init win32 socket lib failed!");
		return false;

	} else {
		__has_init = true;
	}

	static bool _exfuncntion_init = false;
	if (!_exfuncntion_init) {
		if (_InitExFunctnion()) {
			_exfuncntion_init = true;

		} else {
			LOG_FATAL("init expend functions failed!");
			return false;
		}
	}
	return true;
}

void DeallocSocket() {
	WSACleanup();
}

CSocketBase::CSocketBase() : _add_event_actions(false), _invalid(false), _event_actions(nullptr), _pool(new CMemoryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
	_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	SetReusePort(_sock);
	if (_sock == INVALID_SOCKET) {
		LOG_FATAL("init a new socket failed!");
	}
}

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions) : _add_event_actions(false), _invalid(false), _event_actions(event_actions), _pool(new CMemoryPool(1024, 20)) {
	memset(_ip, 0, __addr_str_len);
	_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	SetReusePort(_sock);
	if (_sock == INVALID_SOCKET) {
		LOG_FATAL("init a new socket failed!");
	}
}

CSocketBase::~CSocketBase() {
	closesocket(_sock);
}
#endif