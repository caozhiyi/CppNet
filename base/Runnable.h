#ifndef HEADER_BASE_RUNNABLE
#define HEADER_BASE_RUNNABLE

#include <thread>     // about thread
#include <atomic>     // for atomic_bool
#include <functional> // for bind
#include <memory>     // for shared_ptr

namespace base {
    class CRunnable
    {
    public:
        CRunnable() : _stop(true) {}
        virtual ~CRunnable() {}

        //base option
        virtual void Start() {
            _stop = false;
            if (!_pthread) {
                _pthread = std::shared_ptr<std::thread>(new std::thread(std::bind(&CRunnable::Run, this)));
            }
        }
        virtual void Stop() {
            _stop = true;
        }
        virtual void Join() {
            if (_pthread) {
                _pthread->join();
            }
        }

        //TO DO
        virtual void Run() = 0;

        bool GetStop() {
            return _stop;
        }

        static void Sleep(int interval) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }

    protected:
        CRunnable(const CRunnable&) = delete;
        CRunnable& operator=(const CRunnable&) = delete;

    protected:
        volatile std::atomic_bool       _stop;
        std::shared_ptr<std::thread>    _pthread;
    };
}
#endif