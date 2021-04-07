// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_STRUCTURE_THREAD_SAFE_BLOCK_QUEUE
#define COMMON_STRUCTURE_THREAD_SAFE_BLOCK_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>

namespace cppnet {

template<typename T>
class ThreadSafeBlockQueue {
public:
    ThreadSafeBlockQueue() {}
    ~ThreadSafeBlockQueue() {}

    void Push(const T& element) {
        std::unique_lock<std::mutex> lock(_mutex);
        _queue.push(element);
        _empty_notify.notify_all();
    }

    T Pop() {
        std::unique_lock<std::mutex> lock(_mutex);
        _empty_notify.wait(_mutex, [this]() {return !this->_queue.empty(); });

        auto ret = std::move(_queue.front());
        _queue.pop();
 
        return std::move(ret);
    }

    void Clear() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_queue.empty()) {
            _queue.pop();
        }
    }

    uint32_t Size() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.size();
    }

    bool Empty() {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.empty();
    }

private:
    std::mutex    _mutex;
    std::queue<T> _queue;
    std::condition_variable_any _empty_notify;
};

}
#endif