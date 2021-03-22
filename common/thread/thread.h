#ifndef COMMON_THREAD_THREAD
#define COMMON_THREAD_THREAD

#include <thread>     // about thread
#include <atomic>     // for atomic_bool
#include <memory>     // for shared_ptr
#include <functional> // for bind

namespace cppnet {

class Thread {
public:
    Thread(): _stop(true) {}
    virtual ~Thread() {}

    //base option
    virtual void Start() {
        _stop = false;
        if (!_thread) {
            _thread = std::make_shared<std::thread>(std::bind(&Thread::Run, this));
        }
    }

    virtual void Stop() {
        _stop = true;
    }

    virtual void Join() {
        if (_thread) {
            _thread->join();
        }
    }
    //TO DO
    virtual void Run() = 0;

    virtual bool IsStop() {
        return _stop;
    }

protected:
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

protected:
    std::atomic_bool _stop;
    std::shared_ptr<std::thread> _thread;
};

}
#endif