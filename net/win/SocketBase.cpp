#ifndef __linux__

#include <winsock2.h>

#include "Log.h"
#include "CNConfig.h"
#include "SocketBase.h"
#include "WinExpendFunc.h"

using namespace cppnet;
LPFN_ACCEPTEX                cppnet::__AcceptEx          = nullptr;
LPFN_CONNECTEX               cppnet::__ConnectEx         = nullptr;
LPFN_GETACCEPTEXSOCKADDRS    cppnet::__AcceptExScokAddrs = nullptr;
LPFN_DISCONNECTEX            cppnet::__DisconnectionEx   = nullptr;

static void* _GetExFunctnion(const uint64_t& socket, const GUID& which) {
    void* func = nullptr;
    DWORD bytes = 0;
    WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, (LPVOID)&which,
        sizeof(which), &func, sizeof(func), &bytes, NULL, NULL);

    return func;
}

static bool _InitExFunctnion() {
    SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    __AcceptEx = (LPFN_ACCEPTEX)_GetExFunctnion((uint64_t)socket, WSAID_ACCEPTEX);
    __ConnectEx = (LPFN_CONNECTEX)_GetExFunctnion((uint64_t)socket, WSAID_CONNECTEX);
    __AcceptExScokAddrs = (LPFN_GETACCEPTEXSOCKADDRS)_GetExFunctnion((uint64_t)socket, WSAID_GETACCEPTEXSOCKADDRS);
    __DisconnectionEx = (LPFN_DISCONNECTEX)_GetExFunctnion((uint64_t)socket, WSAID_DISCONNECTEX);
    closesocket(socket);
    if (!__AcceptExScokAddrs || !__ConnectEx || !__AcceptEx || !__DisconnectionEx) {
        base::LOG_FATAL("get expand function failed!");
        return false;
    }
    return true;
}

void cppnet::SetReusePort(const uint64_t& sock) {
    int opt = 1;
    int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
}

bool InitScoket() {
    static WSADATA __wsa_data;
    static bool __has_init = false;
    if (!__has_init && WSAStartup(MAKEWORD(2, 2), &__wsa_data) != 0) {
        base::LOG_FATAL("init win32 socket lib failed!");
        return false;

    } else {
        __has_init = true;
    }

    static bool _exfuncntion_init = false;
    if (!_exfuncntion_init) {
        if (_InitExFunctnion()) {
            _exfuncntion_init = true;

        } else {
            base::LOG_FATAL("init expend functions failed!");
            return false;
        }
    }
    return true;
}

void DeallocSocket() {
    WSACleanup();
}

CSocketBase::CSocketBase() : _add_event_actions(false), _event_actions(nullptr), _pool(new base::CMemoryPool(__mem_block_size, __mem_block_add_step)) {
    memset(_ip, 0, __addr_str_len);
    _sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    SetReusePort(_sock);
    if (_sock == INVALID_SOCKET) {
        base::LOG_FATAL("init a new socket failed!");
    }
}

CSocketBase::CSocketBase(std::shared_ptr<CEventActions>& event_actions) : _add_event_actions(false), _event_actions(event_actions), _pool(new base::CMemoryPool(__mem_block_size, __mem_block_add_step)) {
    memset(_ip, 0, __addr_str_len);
    _sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    SetReusePort(_sock);
    if (_sock == INVALID_SOCKET) {
        base::LOG_FATAL("init a new socket failed!");
    }
}

CSocketBase::~CSocketBase() {
    closesocket(_sock);
}
#endif