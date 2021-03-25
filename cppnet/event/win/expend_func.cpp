#include "expend_func.h"
#include "common/log/log.h"

namespace cppnet {

static void* GetExFunctnion(const uint64_t& socket, const GUID& which) {
    void* func = nullptr;
    DWORD bytes = 0;
    WSAIoctl((SOCKET)socket, SIO_GET_EXTENSION_FUNCTION_POINTER, (LPVOID)&which,
        sizeof(which), &func, sizeof(func), &bytes, NULL, NULL);

    return func;
}

WinExpendFunc::WinExpendFunc():
    _AcceptExScokAddrs(nullptr),
    _ConnectEx(),
    _AcceptEx(),
    _DisconnectionEx() {

    static WSADATA __wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &__wsa_data) != 0) {
        LOG_FATAL("init win32 socket lib failed!");
        return;
    }

    SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    _AcceptEx = (LPFN_ACCEPTEX)GetExFunctnion((uint64_t)socket, WSAID_ACCEPTEX);
    _ConnectEx = (LPFN_CONNECTEX)GetExFunctnion((uint64_t)socket, WSAID_CONNECTEX);
    _AcceptExScokAddrs = (LPFN_GETACCEPTEXSOCKADDRS)GetExFunctnion((uint64_t)socket, WSAID_GETACCEPTEXSOCKADDRS);
    _DisconnectionEx = (LPFN_DISCONNECTEX)GetExFunctnion((uint64_t)socket, WSAID_DISCONNECTEX);
    
    closesocket(socket);

    if (!_AcceptExScokAddrs || !_ConnectEx || !_AcceptEx || !_DisconnectionEx) {
        throw "get expand function failed!";
    }
}

WinExpendFunc::~WinExpendFunc() {
    WSACleanup();
}

}
