#ifdef __APPLE__
#ifndef NET_EVENT_MAC_EPOLL_ACTION
#define NET_EVENT_MAC_EPOLL_ACTION

#include <vector>
#include <cstdint>
#include <sys/event.h> // for kqueue
#include "../action_interface.h"


namespace cppnet {

// kqueue event interface
class KqueueEventActions: public EventActions {
public:
    KqueueEventActions();
    virtual ~KqueueEventActions();
    virtual bool Init(uint32_t thread_num = 0);
    virtual bool Dealloc();
    // timer event
    virtual bool AddTimerEvent(std::shared_ptr<TimeSolt> t, uint32_t interval, bool always = false);
    virtual bool RemoveTimerEvent(std::shared_ptr<TimeSolt> t);
    // net io event
    virtual bool AddSendEvent(std::shared_ptr<Event>& event);
    virtual bool AddRecvEvent(std::shared_ptr<Event>& event);
    virtual bool AddAcceptEvent(std::shared_ptr<Event>& event);

    virtual bool AddConnection(std::shared_ptr<Event>& event, Address& address);
    virtual bool AddDisconnection(std::shared_ptr<Event>& event);

    virtual bool DelEvent(const uint64_t sock);
    virtual bool DelEvent(std::shared_ptr<Event>& event);
    // io thread process
    virtual void ProcessEvent();
    // post a task to net io thread
    virtual void PostTask(Task& task);
    // weak up net io thread
    virtual void Wakeup();

private:
    void OnEvent(std::vector<struct kevent>& event_vec, int16_t num);
    void OnTask();

protected:
    int32_t             _kqueue_handler;
    uint32_t            _pipe[2];

    std::vector<struct kevent> _change_list;
};

}

#endif
#endif