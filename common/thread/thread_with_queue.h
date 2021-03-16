#ifndef COMMON_THREAD_THREAD_WITH_QUEUE
#define COMMON_THREAD_THREAD_WITH_QUEUE

#include "thread.h"
#include "common/structure/thread_safe_block_queue.h"

namespace cppnet {
    
template<typename T>
class ThreadWithQueue: public Thread {
public:
    ThreadWithQueue() {}
    virtual ~ThreadWithQueue() {}

    uint32_t GetQueueSize() {
        return _queue.Size();
    }

    void Push(const T& t) {
        _queue.Push(t);
    }

    T Pop() {
        return std::move(_queue.Pop());
    }

    //TO DO
    virtual void Run() = 0;

protected:
    ThreadWithQueue(const ThreadWithQueue&) = delete;
    ThreadWithQueue& operator=(const ThreadWithQueue&) = delete;

private:
    ThreadSafeBlockQueue<T> _queue;
};

}
#endif