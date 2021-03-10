#ifndef HEADER_BASE_RUNNABLEALONETASKLISTWITHPOST
#define HEADER_BASE_RUNNABLEALONETASKLISTWITHPOST

#include <map>
#include <mutex>

#include "Runnable.h"
#include "TaskQueue.h"

namespace cppnet {

typedef std::function<void()> Task;
class RunnableAloneTaskListWithPost : public Runnable {
public:
    RunnableAloneTaskListWithPost() {}
    virtual ~RunnableAloneTaskListWithPost() {}

    int GetTaskListSize() {
        return _task_list.Size();
    }

    virtual void Start();
    virtual void Stop();

    //post task
    void Push(const Task&& func) {
        _task_list.Push(func);
    }
    void Push(const Task& func) {
        _task_list.Push(func);
    }

    //TO DO
    virtual void Run();

    std::thread::id GetId()const { return _id; }

    //post task to the thread
    static    bool PostTask(const std::thread::id& thread_id, const Task& func);

private:
    Task _Pop() {
        return std::move(_task_list.Pop());
    }

    RunnableAloneTaskListWithPost(const RunnableAloneTaskListWithPost&) = delete;
    RunnableAloneTaskListWithPost& operator=(const RunnableAloneTaskListWithPost&) = delete;

private:
    TaskQueue<Task>         _task_list;    
    std::thread::id         _id;

    static    std::mutex                                                    _map_mutex;    
    static    std::map<std::thread::id, RunnableAloneTaskListWithPost*>    _runnable_map;
};

std::mutex RunnableAloneTaskListWithPost::_map_mutex;
std::map<std::thread::id, RunnableAloneTaskListWithPost*> RunnableAloneTaskListWithPost::_runnable_map;

void RunnableAloneTaskListWithPost::Start() {
    _stop = false;
    if (!_pthread) {
        _pthread = std::shared_ptr<std::thread>(new std::thread(std::bind(&RunnableAloneTaskListWithPost::Run, this)));
    }

    Push([this]() {
        std::unique_lock<std::mutex> lock(_map_mutex);
        auto iter = _runnable_map.find(std::this_thread::get_id());
        if (iter == _runnable_map.end()) {
            _runnable_map[std::this_thread::get_id()] = this;
            _id = std::this_thread::get_id();
        }
    });
}

void RunnableAloneTaskListWithPost::Stop() {
    Push([this]() {
        {
            std::unique_lock<std::mutex> lock(_map_mutex);
            auto iter = _runnable_map.find(std::this_thread::get_id());
            if (iter != _runnable_map.end()) {
                _runnable_map.erase(iter);
            }
        }
        Push(nullptr);
        _stop = true;
    });
}

bool RunnableAloneTaskListWithPost::PostTask(const std::thread::id& thread_id, const Task& func) {
    std::unique_lock<std::mutex> lock(_map_mutex);
    auto iter = _runnable_map.find(thread_id);
    if (iter != _runnable_map.end()) {
        if (iter->second) {
            iter->second->Push(func);
            return true;
        }
    }
    return false;
}

void  RunnableAloneTaskListWithPost::Run() {
    while (!_stop) {
        auto t = _Pop();
        if (t) {
            t();

        } else {
            break;
        }
    }
}

}
#endif