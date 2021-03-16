#ifndef COMMON_STRUCTURE_THREAD_SAFE_QUEUE
#define COMMON_STRUCTURE_THREAD_SAFE_QUEUE

#include <mutex>
#include <queue>

namespace cppnet {

template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() {}
    ~ThreadSafeQueue() {}

    void Push(const T& element) {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.push(element);
    }

    bool Pop(T& value) {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            return false;
        }
        value = std::move(_queue.front());
        _queue.pop();
        return true;
    }

    void Clear() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_queue.empty()) {
            _queue.pop();
        }
    }

    size_t Size() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.size();
    }

    bool Empty() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.empty();
    }

private:
    std::queue<T>        _queue;
    std::mutex           _mutex;
};

}

#endif