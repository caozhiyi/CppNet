#ifndef NET_EVENT_ACTION_INTERFACE
#define NET_EVENT_ACTION_INTERFACE

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

typedef std::function<void()> Task;

// net io event interface
class EventActions {
public:
    EventActions();
    virtual ~EventActions();
    virtual bool Init(uint32_t thread_num = 0) = 0;
    virtual bool Dealloc() = 0;
    // timer event
    virtual bool AddTimerEvent(std::shared_ptr<TimeSolt> t, uint32_t interval, bool always = false);
    virtual bool RemoveTimerEvent(std::shared_ptr<TimeSolt> t);
    // net io event
    virtual bool AddSendEvent(std::shared_ptr<Event>& event) = 0;
    virtual bool AddRecvEvent(std::shared_ptr<Event>& event) = 0;
    virtual bool AddAcceptEvent(std::shared_ptr<Event>& event) = 0;

    virtual bool AddConnection(std::shared_ptr<Event>& event, Address& addr) = 0;
    virtual bool AddDisconnection(std::shared_ptr<Event>& event) = 0;

    virtual bool DelEvent(const uint64_t sock) = 0;
    virtual bool DelEvent(std::shared_ptr<Event>& event) = 0;
    // io thread process
    virtual void ProcessEvent() = 0;
    // post a task to net io thread
    virtual void PostTask(Task& task) = 0;
    // weak up net io thread
    virtual void Wakeup() = 0;
    virtual std::shared_ptr<Timer> Timer() { return _timer; }

protected:
    bool _run;
    uint64_t _cur_utc_time;
    std::shared_ptr<Timer> _timer;
    std::vector<Task> _task_list;
    std::unordered_set<uint64_t> _listener_map;

};

}

#endif