#ifndef COMMON_LOG_IF_LOG
#define COMMON_LOG_IF_LOG

#include <atomic>
#include <memory>
#include <cstdint>

namespace cppnet {
namespace common {

/**
 * @brief Log message structure
 */
struct Log {
    char*    log_;   ///< Log message content
    uint32_t len_;   ///< Length of log message
};

/**
 * @brief Logger base class for custom logging implementations
 *
 * Inherit this class to implement custom log output destinations.
 * Supports logger chaining for multiple output targets.
 *
 * Concurrency model (intentionally lock-free on the hot path):
 *   - The downstream logger in the chain (logger_) is a write-once field.
 *     The first SetLogger() call wins; subsequent calls are silently
 *     ignored. This matches actual usage (logger is configured once at
 *     process start) and lets Debug/Info/... read the pointer with a
 *     single relaxed atomic load -- no mutex on every log line.
 *   - owner_ keeps the chosen downstream alive for the remainder of this
 *     Logger's lifetime, so the raw pointer obtained from logger_next_ is
 *     safe to dereference without reference-count manipulation.
 */
class Logger {
public:
    Logger(): logger_next_(nullptr) {}
    virtual ~Logger() {}

    /**
     * @brief Set the next logger in the chain.
     *
     * Write-once: only the first call installs the downstream; later calls
     * are no-ops. This avoids any locking on the log-write path and makes
     * concurrent SetLogger races safe (no use-after-free of a replaced
     * logger that other threads are still dispatching into).
     *
     * @param logger Next logger instance.
     * @return true if this call installed the logger; false if a logger
     *         was already set.
     */
    bool SetLogger(std::shared_ptr<Logger> logger) {
        Logger* expected = nullptr;
        if (!logger_next_.compare_exchange_strong(expected, logger.get(),
                                                  std::memory_order_release,
                                                  std::memory_order_relaxed)) {
            // Already set; ignore.
            return false;
        }
        // Keep the downstream alive for our lifetime so the raw pointer
        // stored in logger_next_ remains valid until ~Logger().
        owner_ = std::move(logger);
        return true;
    }

    /**
     * @brief Get the next logger in the chain.
     *
     * Returns the strong reference (owner_). Callers that only forward a
     * single log line should prefer GetNextLoggerRaw() to avoid touching
     * the shared_ptr refcount.
     */
    std::shared_ptr<Logger> GetLogger() const { return owner_; }

    /**
     * @brief Log a debug message
     */
    virtual void Debug(std::shared_ptr<Log>& log) { if (auto next = GetNextLoggerRaw()) next->Debug(log); }

    /**
     * @brief Log an info message
     */
    virtual void Info(std::shared_ptr<Log>& log) { if (auto next = GetNextLoggerRaw()) next->Info(log); }

    /**
     * @brief Log a warning message
     */
    virtual void Warn(std::shared_ptr<Log>& log) { if (auto next = GetNextLoggerRaw()) next->Warn(log); }

    /**
     * @brief Log an error message
     */
    virtual void Error(std::shared_ptr<Log>& log) { if (auto next = GetNextLoggerRaw()) next->Error(log); }

    /**
     * @brief Log a fatal message
     */
    virtual void Fatal(std::shared_ptr<Log>& log) { if (auto next = GetNextLoggerRaw()) next->Fatal(log); }

protected:
    /**
     * @brief Lock-free read of the downstream logger pointer.
     *
     * Safe because:
     *   1. logger_next_ is write-once (see SetLogger).
     *   2. owner_ holds a strong reference for this Logger's full lifetime,
     *      so the pointee is still alive whenever this Logger is alive.
     *
     * acquire pairs with the release in SetLogger so callers see a fully
     * constructed downstream object.
     */
    Logger* GetNextLoggerRaw() const {
        return logger_next_.load(std::memory_order_acquire);
    }

    // Cached raw pointer for hot-path reads (no shared_ptr atomic ops).
    std::atomic<Logger*> logger_next_;
    // Holds ownership of the downstream so logger_next_ stays valid.
    // Only written once, by the SetLogger call that wins the CAS, and read
    // by GetLogger() / destruction. No further synchronization needed.
    std::shared_ptr<Logger> owner_;
};

}  // namespace common
}  // namespace cppnet

#endif
