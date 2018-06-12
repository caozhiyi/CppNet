#ifndef linux

#ifndef HEADER_WINEXPENDFUNC
#define HEADER_WINEXPENDFUNC

#include <winsock2.h>
#include <MSWSock.h>

extern LPFN_ACCEPTEX				__AcceptEx;
extern LPFN_CONNECTEX				__ConnectEx;
extern LPFN_GETACCEPTEXSOCKADDRS	__AcceptExScokAddrs;
extern LPFN_DISCONNECTEX			__DisconnectionEx;
#endif

#endif // linux