#ifndef NET_EVENT_EVENT_INTERFACE
#define NET_EVENT_EVENT_INTERFACE

#include <memory>
#include <atomic>

#include "CppDefine.h"

namespace cppnet {
    enum EventType {
        ET_READ             = 0x001,        // read event
        ET_WRITE            = 0x002,        // write event
        ET_ACCEPT           = 0x004,        // accept event
        ET_TIMER            = 0x008,        // timer event
        ET_CONNECT          = 0x010,        // connect event
        ET_DISCONNECT       = 0x020,        // disconnect event
    };

    enum IntervalErrorCode {
        IEC_CONNECT_BREAK   = 0x080,        // connect break
        IEC_CONNECT_FAILED  = 0x100,        // connect faild
        IEC_CONNECT_CLOSE   = 0x200,        // connect close
        IEC_TIME_OUT        = 0x400,        // time out
        IEC_MAX             = 0x800         // max event flag define
    };

    class Event {
    public:
        Event(): _event_type(0) {}
        virtual ~Event() {}

        void AddType(EventType type) { _event_type |= type; }
        uint16_t GetType() { return _event_type; }
        void ClearType() { _event_type = 0; }
    private:
        uint16_t _event_type;
    };
}

#endif