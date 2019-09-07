#ifndef HEADER_BASE_TASKQUEUE
#define HEADER_BASE_TASKQUEUE

#include <list>
#include <mutex>
#include <condition_variable>

namespace base {

#define INT_MAX 2147483647

    template<typename T>
    class CTaskQueue {
    public:
        CTaskQueue(int size = -1) :_list_size(size > 0 ? size : INT_MAX) {

        }

        ~CTaskQueue() {

        }

        void Push(const T& element) {
            std::unique_lock<std::mutex> lock(_block_mutex);
            _full_notify.wait(_block_mutex, [this]() {return this->_block_queue.size() < this->_list_size; });
            _block_queue.push_front(element);
            _empty_notify.notify_all();
        }

        void PushFront(const T& element) {
            std::unique_lock<std::mutex> lock(_block_mutex);
            _full_notify.wait(_block_mutex, [this]() {return this->_block_queue.size() < this->_list_size; });
            _block_queue.push_front(element);
            _empty_notify.notify_all();
        }

        T Pop() {
            std::unique_lock<std::mutex> lock(_block_mutex);
            _empty_notify.wait(_block_mutex, [this]() {return !this->_block_queue.empty(); });
            T ret = std::move(_block_queue.back());
            _block_queue.pop_back();
            _full_notify.notify_all();
            return std::move(ret);
        }

        void Clear(bool notify = true) {
            std::unique_lock<std::mutex> lock(_block_mutex);
            while (!_block_queue.empty()) {
                _block_queue.pop_front();
                _full_notify.notify_one();
            }
        }

        int Size() {
            std::unique_lock<std::mutex> lock(_block_mutex);
            return _block_queue.size();
        }

    private:
        size_t                             _list_size;
        std::list<T>                       _block_queue;
        std::mutex                         _block_mutex;
        std::condition_variable_any        _empty_notify;
        std::condition_variable_any        _full_notify;
    };
}
#endif