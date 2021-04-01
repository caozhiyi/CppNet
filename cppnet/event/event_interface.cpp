#include "event_interface.h"

namespace cppnet {

char* TypeString(EventType type) {
    switch (type)
    {
    case ET_READ:
        return "read";
        break;
    case ET_WRITE:
        return "write";
        break;
    case ET_ACCEPT:
        return "accept";
        break;
    case ET_TIMER:
        return "timer";
        break;
    case ET_USER_TIMER:
        return "user_timer";
        break;
    case ET_CONNECT:
        return "connect";
        break;
    case ET_DISCONNECT:
        return "disconnect";
        break;
    case ET_INACTIONS:
        return "inactions";
        break;
    default:
        return "unknow";
        break;
    }
}

}
