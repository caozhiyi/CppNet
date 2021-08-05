// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_DISPATCHER
#define CPPNET_DISPATCHER

#include <mutex>
#include <memory>
#include <thread>
#include <functional>
#include <unordered_map>
#include "include/cppnet_type.h"
#include "foundation/thread/thread_with_queue.h"

namespace cppnet {

typedef std::function<void()> Task;

class Timer;
class RWSocket;
class TimerEvent;
class CppNetBase;
class EventActions;

class Dispatcher: 
    public Thread,
    public std::enable_shared_from_this<Dispatcher> {

public:
    Dispatcher(std::shared_ptr<CppNetBase> base, uint32_t thread_num, uint32_t base_id);
    Dispatcher(std::shared_ptr<CppNetBase> base, uint32_t base_id = 0);
    ~Dispatcher();

    void Run();

    void Stop();

    void Listen(uint64_t sock, const std::string& ip, uint16_t port);

    void Connect(const std::string& ip, uint16_t port);

    void PostTask(const Task& task);

    uint32_t AddTimer(const user_timer_call_back& cb, void* param, uint32_t interval, bool always = false);
    uint32_t AddTimer(std::shared_ptr<RWSocket> sock, uint32_t interval, bool always = false);
    void StopTimer(uint32_t timer_id);

    std::thread::id GetThreadID() { return _local_thread_id; }

private:
    void DoTask();
    uint32_t MakeTimerID();

private:
    uint64_t _cur_utc_time;

    std::mutex _timer_id_mutex;
    uint32_t _timer_id_creater;

    std::mutex _task_list_mutex;
    std::vector<Task> _task_list;

    std::thread::id _local_thread_id;
    std::shared_ptr<Timer> _timer;
    std::shared_ptr<EventActions> _event_actions;

    std::weak_ptr<CppNetBase> _cppnet_base;

    static thread_local std::unordered_map<uint64_t, std::shared_ptr<TimerEvent>> __all_timer_event_map;
};

}

#endif