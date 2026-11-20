#ifndef COMMON_LOG_BASE_LOGGER
#define COMMON_LOG_BASE_LOGGER

#include <stdarg.h>
#include <atomic>
#include <cstdint>
#include <memory>

#include "common/log/log.h"
#include "common/log/log_stream.h"
#include "common/structure/thread_safe_queue.h"

namespace cppnet {
class Alloter;  // defined in cppnet:: alloter

namespace common {

// basic management class of log printing
struct Log;
class Logger;
class BaseLogger {
public:
    BaseLogger(uint16_t cache_size, uint16_t block_size);
    ~BaseLogger();

    // Install the underlying logger chain. Write-once: the first successful
    // call wins, subsequent calls are silently ignored. This matches the
    // intended lifetime model (logger is configured exactly once at process
    // startup) and lets every LOG_* call read the logger pointer with a
    // single relaxed-acquire atomic load -- no mutex, no shared_ptr
    // reference-count traffic on the hot path.
    //
    // Rationale (load_tester crash): the previous implementation re-assigned
    // logger_ on every QuicClient::Init(), which under concurrent Init()
    // calls let the old FileLogger be destroyed while other threads were
    // still pushing into its queue, causing std::mutex::lock() to throw
    // system_error from inside condition_variable_any::notify_all().
    // Making the install write-once eliminates that race entirely.
    //
    // @return true if this call installed the logger; false if a logger was
    //         already set.
    bool SetLogger(std::shared_ptr<Logger> log);

    // Returns the strong reference owning the installed logger (or nullptr
    // if none has been set). Intended for diagnostic / shutdown use, not
    // the hot path.
    std::shared_ptr<Logger> GetLogger() const { return logger_owner_; }

    void SetLevel(LogLevel level);

    void Debug(const char* file, uint32_t line, const char* content, va_list list);
    void Info(const char* file, uint32_t line, const char* content, va_list list);
    void Warn(const char* file, uint32_t line, const char* content, va_list list);
    void Error(const char* file, uint32_t line, const char* content, va_list list);
    void Fatal(const char* file, uint32_t line, const char* content, va_list list);

    LogStreamParam GetStreamParam(LogLevel level, const char* file, uint32_t line);

private:
    std::shared_ptr<Log> GetLog();
    void FreeLog(Log* log);
    Log* NewLog();

    // Lock-free read used by all log dispatch paths. Safe because logger_
    // is write-once and logger_owner_ keeps the pointee alive for the
    // remainder of this BaseLogger's lifetime.
    Logger* GetLoggerRaw() const {
        return logger_.load(std::memory_order_acquire);
    }

protected:
    // level_ is read on every LOG_* call (potentially from any thread that
    // ever logs) and written by SetLevel() — which is invoked from each
    // QuicClient::Init() / QuicServer::Init() and therefore can happen
    // concurrently with active log dispatch from worker threads. TSan
    // confirmed the race (base_logger.cpp:141 read vs base_logger.cpp:93
    // write). Make it atomic with relaxed ordering: the level mask check
    // is a self-contained predicate, no other state depends on it being
    // observed in lock-step with anything else.
    std::atomic<uint16_t> level_;
    uint16_t cache_size_;
    uint16_t block_size_;

    std::shared_ptr<::cppnet::Alloter> allocter_;
    ThreadSafeQueue<Log*> cache_queue_;

    // Hot-path-friendly raw pointer to the installed logger. Written once
    // by the SetLogger() call that wins the CAS; read with acquire in
    // GetLoggerRaw().
    std::atomic<Logger*> logger_;
    // Owns the installed logger so the raw pointer above stays valid for
    // the entire lifetime of this BaseLogger. Written once, then read-only.
    std::shared_ptr<Logger> logger_owner_;
};

}  // namespace common
}  // namespace cppnet

#endif
