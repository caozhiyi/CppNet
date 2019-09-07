#ifndef HEADER_BASE_TSQUEUE
#define HEADER_BASE_TSQUEUE

#include <mutex>
#include <queue>

namespace base {

    // thread safe queue
    template<typename T>
    class CTSQueue {
    public:
        CTSQueue() {

        }

        ~CTSQueue() {

        }

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

    private:
        std::queue<T>        _queue;
        std::mutex           _mutex;
    };
}

#endif