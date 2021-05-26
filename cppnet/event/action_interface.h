// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

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
    virtual bool AddSendEvent(Event* event) = 0;
    virtual bool AddRecvEvent(Event* event) = 0;
    virtual bool AddAcceptEvent(Event* event) = 0;
    virtual bool AddConnection(Event* event, Address& addr) = 0;
    virtual bool AddDisconnection(Event* event) = 0;

    virtual bool DelEvent(Event* event) = 0;
    // io thread process
    virtual void ProcessEvent(int32_t wait_ms) = 0;
    // weak up net io thread
    virtual void Wakeup() = 0;
};

std::shared_ptr<EventActions> MakeEventActions();

}

#endif