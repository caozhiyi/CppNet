#ifndef HEADER_BASE_RUNNABLEALONETASKLIST
#define HEADER_BASE_RUNNABLEALONETASKLIST

#include "Runnable.h"
#include "TaskQueue.h"

namespace cppnet {

typedef std::function<void()> Task;
    
template<typename T = Task>
class RunnableAloneTaskList : public Runnable
{
public:
    RunnableAloneTaskList() {}
    virtual ~RunnableAloneTaskList() {}

    int GetTaskListSize() {
        return _task_list.Size();
    }

    //post task
    void Push(const T&& t) {
        _task_list.Push(t);
    }
    void Push(const T& t) {
        _task_list.Push(t);
    }
    void PushFront(const T&& t) {
        _task_list.PushFront(t);
    }
    void PushFront(const T& t) {
        _task_list.PushFront(t);
    }

    //TO DO
    virtual void Run() = 0;

protected:
    T _Pop() {
        return std::move(_task_list.Pop());
    }

    RunnableAloneTaskList(const RunnableAloneTaskList&) = delete;
    RunnableAloneTaskList& operator=(const RunnableAloneTaskList&) = delete;

private:
    CTaskQueue<T>            _task_list;            //every thread have a task queue
};

}
#endif