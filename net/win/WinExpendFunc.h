#ifndef __linux__

#ifndef HEADER_WINEXPENDFUNC
#define HEADER_WINEXPENDFUNC

#include <winsock2.h>
#include <MSWSock.h>

extern LPFN_ACCEPTEX				__AcceptEx;
extern LPFN_CONNECTEX				__ConnectEx;
extern LPFN_GETACCEPTEXSOCKADDRS	__AcceptExScokAddrs;
extern LPFN_DISCONNECTEX			__DisconnectionEx;
extern void SetReusePort(unsigned int sock);

#endif

#endif // __linux__
