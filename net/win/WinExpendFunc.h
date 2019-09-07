#ifndef __linux__

#ifndef HEADER_NET_WIN_WINEXPENDFUNC
#define HEADER_NET_WIN_WINEXPENDFUNC

#include <winsock2.h>
#include <MSWSock.h>

namespace cppnet {
    extern LPFN_ACCEPTEX                __AcceptEx;
    extern LPFN_CONNECTEX               __ConnectEx;
    extern LPFN_GETACCEPTEXSOCKADDRS    __AcceptExScokAddrs;
    extern LPFN_DISCONNECTEX            __DisconnectionEx;
    extern void SetReusePort(const uint64_t& sock);
}

#endif

#endif // __linux__
