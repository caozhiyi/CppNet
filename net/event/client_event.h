#ifndef NET_EVENT_CLIENT_EVENT
#define NET_EVENT_CLIENT_EVENT

#include <memory>

#include "event_interface.h"

namespace cppnet {

class Buffer;
class Socket;
class ClientEvent: public Event {
public:
    ClientEvent(std::shared_ptr<Buffer> buffer);
    ~ClientEvent();
    
private:
    std::shared_ptr<Buffer> _buffer;
    std::shared_ptr<Socket> _socket;
};

}

#endif