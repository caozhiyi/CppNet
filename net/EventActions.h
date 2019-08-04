#ifndef HEADER_NET_EVENTACTIONS
#define HEADER_NET_EVENTACTIONS
#include <string>
#include "Timer.h"

namespace cppnet {
    class CEventHandler;

    // net io event interface
    class CEventActions
    {
    public:
        CEventActions() {}
        virtual ~CEventActions() {}

        //param is net io thread num, default cpu number
        virtual bool Init() = 0;
        virtual bool Dealloc() = 0;

        virtual uint64_t AddTimerEvent(unsigned int interval, const std::function<void(void*)>& call_back, void* param, bool always = false) = 0;
        virtual bool AddTimerEvent(unsigned int interval, base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool RemoveTimerEvent(uint64_t timer_id) = 0;
        virtual bool AddSendEvent(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool AddRecvEvent(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event) = 0;
#ifndef __linux__
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, char* buf, int buf_len) = 0;
#else
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) = 0;
#endif
        virtual bool AddDisconnection(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool DelEvent(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual void ProcessEvent() = 0;

        virtual void PostTask(std::function<void(void)>&) = 0;
        virtual void WakeUp() = 0;

        virtual CTimer& Timer() { return _timer; }
    protected:
        CTimer			_timer;
        int				_need_init;
        std::string		_name;
    };
}

#endif