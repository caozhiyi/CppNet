#include "dispatcher.h"
#include "cppnet/cppnet_base.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/timer_event.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/mac/kqueue_action.h"
#include "cppnet/event/linux/epoll_action.h"

#include "common/util/time.h"
#include "common/timer/timer.h"
#include "common/alloter/pool_alloter.h"

namespace cppnet {

thread_local std::unordered_map<uint64_t, std::shared_ptr<TimerEvent>> Dispatcher::__all_timer_event_map;

std::mutex Dispatcher::_wait_destroy_map_mutex;
std::unordered_map<std::thread::id, std::shared_ptr<std::thread>> Dispatcher::_wait_destroy_thread_map;

Dispatcher::Dispatcher(std::shared_ptr<CppNetBase> base):
    _cur_utc_time(0),
    _timer_id_creater(0),
    _cppnet_base(base) {

    _timer = MakeTimer1Min();
#ifdef __APPLE__
    _event_actions = std::make_shared<KqueueEventActions>();
#elif __linux__
    _event_actions = std::make_shared<EpollEventActions>();
#endif
    _event_actions->Init();
    // start thread
    Start();
}

Dispatcher::~Dispatcher() {
    if (std::this_thread::get_id() != _local_thread_id) {
        Stop();
        Join();

    } else {
        Stop();

        std::unique_lock<std::mutex> lock(_wait_destroy_map_mutex);
        _wait_destroy_thread_map[std::this_thread::get_id()] = _thread;
        Join();
    }
}

void Dispatcher::Run() {
    _local_thread_id = std::this_thread::get_id();
    _cur_utc_time = UTCTimeMsec();
    int32_t wait_time = 0;
    uint64_t cur_time = 0;

    while (!_stop) {
        cur_time = UTCTimeMsec();
        _timer->TimerRun(cur_time - _cur_utc_time);
        _cur_utc_time = cur_time;

        wait_time = _timer->MinTime();

        _event_actions->ProcessEvent(wait_time);

        DoTask();
    }
}

void Dispatcher::Stop() {
    _stop = true;
    _event_actions->Wakeup();
}

void Dispatcher::Listen(uint64_t sock, const std::string& ip, uint16_t port) {
    if (std::this_thread::get_id() == _local_thread_id) {
        auto connect_sock = std::make_shared<ConnectSocket>();
        connect_sock->SetEventActions(_event_actions);
        connect_sock->SetCppNetBase(_cppnet_base.lock());
        connect_sock->SetSocket(sock);
        connect_sock->Bind(ip, port);
        connect_sock->Listen();

    } else {
        auto task = [sock, ip, port, this]() {
            auto connect_sock = std::make_shared<ConnectSocket>();
            connect_sock->SetEventActions(_event_actions);
            connect_sock->SetCppNetBase(_cppnet_base.lock());
            connect_sock->SetSocket(sock);
            connect_sock->Bind(ip, port);
            connect_sock->Listen();
        };
        PostTask(task);
    }
}

void Dispatcher::Connect(const std::string& ip, uint16_t port) {
    if (std::this_thread::get_id() == _local_thread_id) {
        auto alloter = std::make_shared<AlloterWrap>(MakePoolAlloterPtr());
        auto sock = std::make_shared<RWSocket>(alloter);

        sock->SetEventActions(_event_actions);
        sock->SetCppNetBase(_cppnet_base.lock());
        sock->Connect(ip, port);
    
    } else {
        auto task = [ip, port, this]() {
            auto alloter = std::make_shared<AlloterWrap>(MakePoolAlloterPtr());
            auto sock = std::make_shared<RWSocket>(alloter);

            sock->SetEventActions(_event_actions);
            sock->SetCppNetBase(_cppnet_base.lock());
            sock->Connect(ip, port);
        };
        PostTask(task);
    }
}

void Dispatcher::PostTask(const Task& task) {
    {
        std::unique_lock<std::mutex> lock(_task_list_mutex);
        _task_list.push_back(task);
    }
    _event_actions->Wakeup();
}

uint32_t Dispatcher::AddTimer(const user_timer_call_back& cb, void* param, uint32_t interval, bool always) {
    std::shared_ptr<TimerEvent> event = std::make_shared<TimerEvent>();
    event->AddType(ET_USER_TIMER);
    event->SetTimerCallBack(cb, param);

    uint32_t timer_id = MakeTimerID();

    if (std::this_thread::get_id() == _local_thread_id) {
        _timer->AddTimer(event, interval, always);
        __all_timer_event_map[timer_id] = event;
        _event_actions->Wakeup();
        
    } else {
        auto task = [event, timer_id, interval, always, this]() {
            _timer->AddTimer(event, interval, always);
            __all_timer_event_map[timer_id] = event;
        };
        PostTask(task);
    }
    return timer_id;
}

uint32_t Dispatcher::AddTimer(std::shared_ptr<RWSocket> sock, uint32_t interval, bool always) {
    std::shared_ptr<TimerEvent> event = std::make_shared<TimerEvent>();
    event->AddType(ET_TIMER);
    event->SetSocket(sock);

    uint32_t timer_id = MakeTimerID();

    if (std::this_thread::get_id() == _local_thread_id) {
        _timer->AddTimer(event, interval, always);
        __all_timer_event_map[timer_id] = event;
        _event_actions->Wakeup();
        
    } else {
        auto task = [event, timer_id, interval, always, this]() {
            _timer->AddTimer(event, interval, always);
            __all_timer_event_map[timer_id] = event;
        };
        PostTask(task);
    }
    return timer_id;
}

void Dispatcher::StopTimer(uint64_t timer_id) {
    if (std::this_thread::get_id() == _local_thread_id) {
        auto iter = __all_timer_event_map.find(timer_id);
        if (iter == __all_timer_event_map.end()) {
            return;
        }
        
        _timer->RmTimer(iter->second);
        __all_timer_event_map.erase(iter);

    } else {
        auto task = [timer_id, this]() {
            auto iter = __all_timer_event_map.find(timer_id);
            if (iter == __all_timer_event_map.end()) {
                return;
            }

            _timer->RmTimer(iter->second);
            __all_timer_event_map.erase(iter);
        };
        PostTask(task);
    }
}

void Dispatcher::DoTask() {
    std::vector<Task> func_vec;
    {
        std::unique_lock<std::mutex> lock(_task_list_mutex);
        func_vec.swap(_task_list);
    }

    for (size_t i = 0; i < func_vec.size(); ++i) {
        func_vec[i]();
    }
}

uint64_t Dispatcher::MakeTimerID() {
    std::unique_lock<std::mutex> lock(_timer_id_mutex);
    return ++_timer_id_creater;
}

}
