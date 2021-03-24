#ifndef CPPNET_EVENT_WIN_EXPEND_FUNC
#define CPPNET_EVENT_WIN_EXPEND_FUNC

#include <MSWSock.h>
#include <winsock2.h>

#include "common/util/singleton.h"

namespace cppnet {

class WinExpendFunc:
    public Singleton<WinExpendFunc> {

public:
    WinExpendFunc();
    ~WinExpendFunc();

public:
    LPFN_ACCEPTEX             _AcceptEx;
    LPFN_CONNECTEX            _ConnectEx;
    LPFN_GETACCEPTEXSOCKADDRS _AcceptExScokAddrs;
    LPFN_DISCONNECTEX         _DisconnectionEx;
};

typedef WinExpendFunc::Instance()._AcceptEx          AcceptEx;
typedef WinExpendFunc::Instance()._ConnectEx         ConnectEx;
typedef WinExpendFunc::Instance()._AcceptExScokAddrs AcceptExScokAddrs;
typedef WinExpendFunc::Instance()._DisconnectionEx   DisconnectionEx;


}

#endif
