#ifndef CPPNET_EVENT_ACTION_INTERFACE
#define CPPNET_EVENT_ACTION_INTERFACE

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <functional>
#include <unordered_set>

namespace cppnet {

class Timer;
class Event;
class Address;
class TimeSolt;

// net io event interface
class EventActions {
public:
    EventActions() {}
    virtual ~EventActions() {}

    virtual bool Init(uint32_t thread_num = 0) = 0;
    virtual bool Dealloc() = 0;

    // net io event
    virtual bool AddSendEvent(std::shared_ptr<Event>& event) = 0;
    virtual bool AddRecvEvent(std::shared_ptr<Event>& event) = 0;
    virtual bool AddAcceptEvent(std::shared_ptr<Event>& event) = 0;

    virtual bool AddConnection(std::shared_ptr<Event>& event, Address& addr) = 0;
    virtual bool AddDisconnection(std::shared_ptr<Event>& event) = 0;

    virtual bool DelEvent(std::shared_ptr<Event>& event) = 0;
    // io thread process
    virtual void ProcessEvent(int32_t wait_ms) = 0;
    // weak up net io thread
    virtual void Wakeup() = 0;
};

}

#endif