#include "common/log/base_logger.h"
#include "common/alloter/alloter_interface.h"
#include "common/alloter/normal_alloter.h"
#include "common/log/if_logger.h"
#include "common/log/log_context.h"
#include "common/util/time.h"

namespace cppnet {
namespace common {

enum LogLevelMask {
    kFatalMask = 0x01,
    kErrorMask = 0x02,
    kWarnMask = 0x04,
    kInfoMask = 0x08,
    kDebugMask = 0x10,
};

static uint32_t FormatLog(const char* file, uint32_t line, const char* level, char* buf, uint32_t len) {
    // format level
    uint32_t curlen = snprintf(buf, len, "[%s|", level);

    // format time
    uint32_t size = kFormatTimeBufSize;
    GetFormatTime(buf + curlen, size);
    curlen += size;

    // format context tag if exists
    const char* context_tag = LogContext::GetTag();
    if (context_tag && *context_tag) {
        curlen += snprintf(buf + curlen, len - curlen, "|%s|%s:%d] ", context_tag, file, line);
    } else {
        // format other info
        curlen += snprintf(buf + curlen, len - curlen, "|%s:%d] ", file, line);
    }

    return curlen;
}

static uint32_t FormatLog(
    const char* file, uint32_t line, const char* level, const char* content, va_list list, char* buf, uint32_t len) {
    uint32_t curlen = 0;

    // format level time and file name
    curlen += FormatLog(file, line, level, buf, len);

    int ret = vsnprintf(buf + curlen, len - curlen, content, list);
    // vsnprintf returns the number of chars that would have been written (excluding null terminator).
    // If ret < 0 (encoding error) or ret >= remaining space, clamp to available space.
    if (ret < 0) {
        ret = 0;
    } else if (static_cast<uint32_t>(ret) > len - curlen) {
        ret = static_cast<int>(len - curlen);
    }
    curlen += static_cast<uint32_t>(ret);

    return curlen;
}

BaseLogger::BaseLogger(uint16_t cache_size, uint16_t block_size):
    level_(static_cast<uint16_t>(LogLevel::kInfo)),
    cache_size_(cache_size),
    block_size_(block_size),
    logger_(nullptr) {
    allocter_ = MakeNormalAlloterPtr();
}

BaseLogger::~BaseLogger() {
    SetLevel(LogLevel::kNull);
}

bool BaseLogger::SetLogger(std::shared_ptr<Logger> log) {
    // Write-once: only the first call installs the logger. Subsequent
    // calls (e.g. additional QuicClient::Init() invocations from the
    // load_tester) are deliberate no-ops -- this avoids the
    // use-after-free that arose from concurrent replacement of the
    // singleton's logger while other threads were still dispatching
    // into the previous one.
    Logger* expected = nullptr;
    if (!logger_.compare_exchange_strong(expected, log.get(),
                                         std::memory_order_release,
                                         std::memory_order_relaxed)) {
        return false;
    }
    // The CAS winner takes ownership for the rest of this BaseLogger's
    // lifetime. From this point on logger_ is read-only, so the hot path
    // can dereference it without any synchronization beyond the acquire
    // load in GetLoggerRaw().
    logger_owner_ = std::move(log);
    return true;
}

void BaseLogger::SetLevel(LogLevel level) {
    // Snapshot the previous level under release ordering so any LOG_* call
    // that observes the new value with acquire (level_.load below) also
    // observes a fully-constructed cache_queue_ on the kNull -> active
    // transition. The cache_queue_ ops below are themselves thread-safe.
    const uint16_t new_level = static_cast<uint16_t>(level);
    level_.store(new_level, std::memory_order_release);
    if (new_level > static_cast<uint16_t>(LogLevel::kNull) && cache_queue_.Empty()) {
        for (uint16_t i = 0; i < cache_size_; i++) {
            cache_queue_.Push(NewLog());
        }

    } else if (new_level == static_cast<uint16_t>(LogLevel::kNull)) {
        size_t size = cache_queue_.Size();
        Log* log = nullptr;
        void* del = nullptr;
        for (size_t i = 0; i < size; i++) {
            if (cache_queue_.Pop(log)) {
                del = (void*)log;
                allocter_->Free(del);
            }
        }
    }
}

void BaseLogger::Debug(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(level_.load(std::memory_order_acquire) & kDebugMask)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->len_ = FormatLog(file, line, "DEB", content, list, log->log_, log->len_);

    // Lock-free read; logger_ is write-once and its target is kept alive
    // by logger_owner_ for our entire lifetime.
    if (Logger* logger = GetLoggerRaw()) {
        logger->Debug(log);
    }
}

void BaseLogger::Info(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(level_.load(std::memory_order_acquire) & kInfoMask)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->len_ = FormatLog(file, line, "INF", content, list, log->log_, log->len_);

    if (Logger* logger = GetLoggerRaw()) {
        logger->Info(log);
    }
}

void BaseLogger::Warn(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(level_.load(std::memory_order_acquire) & kWarnMask)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->len_ = FormatLog(file, line, "WAR", content, list, log->log_, log->len_);

    if (Logger* logger = GetLoggerRaw()) {
        logger->Warn(log);
    }
}

void BaseLogger::Error(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(level_.load(std::memory_order_acquire) & kErrorMask)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->len_ = FormatLog(file, line, "ERR", content, list, log->log_, log->len_);

    if (Logger* logger = GetLoggerRaw()) {
        logger->Error(log);
    }
}

void BaseLogger::Fatal(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(level_.load(std::memory_order_acquire) & kFatalMask)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->len_ = FormatLog(file, line, "FAT", content, list, log->log_, log->len_);

    if (Logger* logger = GetLoggerRaw()) {
        logger->Fatal(log);
    }
}

LogStreamParam BaseLogger::GetStreamParam(LogLevel level, const char* file, uint32_t line) {
    // check log level can print
    if (static_cast<uint16_t>(level) > level_.load(std::memory_order_acquire)) {
        return std::make_pair(nullptr, nullptr);
    }

    std::shared_ptr<Log> log = GetLog();
    std::function<void(std::shared_ptr<Log>)> cb;
    // Capture the raw pointer; logger_ is write-once and kept alive by
    // logger_owner_ for the lifetime of this BaseLogger, so the lambda's
    // dereference is safe as long as the singleton is alive (which is the
    // entire process under common::Singleton).
    Logger* logger = GetLoggerRaw();
    switch (level) {
        case LogLevel::kNull:
            break;
        case LogLevel::kFatal:
            cb = [logger](std::shared_ptr<Log> l) { if (logger) logger->Fatal(l); };
            log->len_ = FormatLog(file, line, "FAT", log->log_, log->len_);
            break;
        case LogLevel::kError:
            cb = [logger](std::shared_ptr<Log> l) { if (logger) logger->Error(l); };
            log->len_ = FormatLog(file, line, "ERR", log->log_, log->len_);
            break;
        case LogLevel::kWarn:
            cb = [logger](std::shared_ptr<Log> l) { if (logger) logger->Warn(l); };
            log->len_ = FormatLog(file, line, "WAR", log->log_, log->len_);
            break;
        case LogLevel::kInfo:
            cb = [logger](std::shared_ptr<Log> l) { if (logger) logger->Info(l); };
            log->len_ = FormatLog(file, line, "INF", log->log_, log->len_);
            break;
        case LogLevel::kDebug:
            cb = [logger](std::shared_ptr<Log> l) { if (logger) logger->Debug(l); };
            log->len_ = FormatLog(file, line, "DEB", log->log_, log->len_);
            break;
        default:
            return std::make_pair(nullptr, nullptr);
    }

    return std::make_pair(log, cb);
}

std::shared_ptr<Log> BaseLogger::GetLog() {
    Log* log = nullptr;
    if (cache_queue_.Pop(log)) {
    } else {
        log = NewLog();
    }

    return std::shared_ptr<Log>(log, [this](Log* l) { FreeLog(l); });
}

void BaseLogger::FreeLog(Log* log) {
    if (cache_queue_.Size() > cache_size_) {
        void* del = (void*)log;
        allocter_->Free(del);

    } else {
        log->len_ = block_size_;
        cache_queue_.Push(log);
    }
}

Log* BaseLogger::NewLog() {
    Log* item = (Log*)allocter_->MallocAlign(block_size_ + sizeof(Log));

    item->log_ = (char*)item + sizeof(Log);
    item->len_ = block_size_;

    return item;
}

}  // namespace common
}  // namespace cppnet