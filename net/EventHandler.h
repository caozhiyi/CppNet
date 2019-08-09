#ifndef HEADER_NET_EVENTHANDLER
#define HEADER_NET_EVENTHANDLER

#include <memory>
#include <atomic>
#include "PoolSharedPtr.h"
#include "AcceptSocket.h"
#include "SocketImpl.h"
#include "CppDefine.h"

#define INVALID_TIMER   -1

namespace cppnet {
    enum EVENT_FLAG {
        EVENT_READ          = 0x0001,		//read event
        EVENT_WRITE         = 0x0002,		//write event
        EVENT_ACCEPT        = 0x0004,		//accept event
        EVENT_TIMER         = 0x0008,		//timer event
        EVENT_CONNECT       = 0x0010,		//connect event
        EVENT_DISCONNECT    = 0x0020,		//disconnect event

        EVENT_TIMER_ALWAYS  = 0x0040,       //timer always check
    };

    class Cevent {
    public:
        void*						_data = nullptr;
        int							_event_flag_set = 0;
    };

    struct CTimerEvent {
        int							_event_flag;
        uint64_t                    _timer_id;
        unsigned int                _interval;
        void*                       _timer_param;
        std::function<void(void*)>  _timer_call_back;   // only timer event
        base::CMemWeakPtr<CEventHandler>  _event;
    };

    class CBuffer;
    class CEventHandler : public Cevent {
    public:
        base::CMemSharePtr<base::CBuffer>	_buffer;
        base::CMemWeakPtr<CSocketImpl>		_client_socket;
        int							        _off_set;				//read or write size

        std::function<void(base::CMemSharePtr<CEventHandler>&, int error)>	_call_back;
    };

    class CAcceptEventHandler : public Cevent {
    public:
        base::CMemSharePtr<CSocketImpl>		_client_socket;

        base::CMemSharePtr<CAcceptSocket>	_accept_socket = nullptr;
        std::function<void(base::CMemSharePtr<CAcceptEventHandler>&, int error)>	_call_back;
    };
}

#endif