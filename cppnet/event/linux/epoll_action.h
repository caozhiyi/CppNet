#ifdef __linux__
#ifndef NET_EVENT_LINUX_EPOLL_ACTION
#define NET_EVENT_LINUX_EPOLL_ACTION

#include <mutex>
#include <sys/epoll.h>
#include "../action_interface.h"

namespace cppnet {

// epoll event interface
class EpollEventActions: public EventActions {
public:
    EpollEventActions();
    virtual ~EpollEventActions();
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
    void OnEvent(std::vector<epoll_event>& event_vec, int16_t num);
    void OnTask();
    bool AddEvent(epoll_event* ev, int32_t event_flag, uint64_t sock, bool in_actions);

protected:
    std::mutex  _mutex;
    int32_t     _epoll_handler;
    uint32_t    _pipe[2];
    epoll_event _pipe_content;

};

}

#endif
#endif