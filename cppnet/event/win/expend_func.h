// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_EVENT_WIN_EXPEND_FUNC
#define CPPNET_EVENT_WIN_EXPEND_FUNC

#include <winsock2.h>
#include <mswsock.h>
#pragma comment(lib,"mswsock.lib")
#pragma comment(lib,"ws2_32.lib")

#include "common/util/singleton.h"

namespace cppnet {

class WinExpendFunc:
    public Singleton<WinExpendFunc> {

public:
    WinExpendFunc();
    ~WinExpendFunc();
    
    void Init() {}
public:
    
    LPFN_ACCEPTEX             _AcceptEx;
    LPFN_CONNECTEX            _ConnectEx;
    LPFN_GETACCEPTEXSOCKADDRS _AcceptExScokAddrs;
    LPFN_DISCONNECTEX         _DisconnectionEx;
 
};

#define WinSockInit        WinExpendFunc::Instance().Init
#define AcceptEx           WinExpendFunc::Instance()._AcceptEx
#define ConnectEx          WinExpendFunc::Instance()._ConnectEx
#define AcceptExScokAddrs  WinExpendFunc::Instance()._AcceptExScokAddrs
#define DisconnectionEx    WinExpendFunc::Instance()._DisconnectionEx

}

#endif
