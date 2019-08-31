#ifndef HEADER_NET_EVENTACTIONS
#define HEADER_NET_EVENTACTIONS

#include <string>

#include "Timer.h"
#include "CppDefine.h"

namespace cppnet {
    class CEventHandler;

    // net io event interface
    class CEventActions
    {
    public:
        CEventActions() {}
        virtual ~CEventActions() {}

        virtual bool Init(uint32_t thread_num = 0) = 0;
        virtual bool Dealloc() = 0;

        // timer event
        virtual uint64_t AddTimerEvent(uint32_t interval, const timer_call_back& call_back, void* param, bool always = false) = 0;
        virtual bool AddTimerEvent(uint32_t interval, base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool RemoveTimerEvent(uint64_t timer_id) = 0;

        // net io event
        virtual bool AddSendEvent(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool AddRecvEvent(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool AddAcceptEvent(base::CMemSharePtr<CAcceptEventHandler>& event) = 0;
#ifndef __linux__
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port, const char* buf, int buf_len) = 0;
#else
        virtual bool AddConnection(base::CMemSharePtr<CEventHandler>& event, const std::string& ip, short port) = 0;
        virtual bool DelEvent(const uint64_t& sock) = 0;
#endif
        virtual bool AddDisconnection(base::CMemSharePtr<CEventHandler>& event) = 0;
        virtual bool DelEvent(base::CMemSharePtr<CEventHandler>& event) = 0;

        // io thread process
        virtual void ProcessEvent() = 0;
        // post a task to net io thread
        virtual void PostTask(std::function<void(void)>&) = 0;
        // weak up net io thread
        virtual void WakeUp() = 0;

        virtual CTimer& Timer() { return _timer; }
    protected:
        CTimer			_timer;
    };
}

#endif