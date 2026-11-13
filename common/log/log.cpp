#include <stdarg.h>

#include "common/log/log.h"
#include "common/log/base_logger.h"

namespace cppnet {
namespace common {

// Global log level mirror used by macros for short-circuit.
// Default to kInfo so warnings/errors are visible before SetLevel is called.
std::atomic<uint8_t> g_log_level{static_cast<uint8_t>(LogLevel::kInfo)};

SingletonLogger::SingletonLogger() {
    logger_ = std::make_shared<BaseLogger>(kLogCacheSize, kLogBlockSize);
}

SingletonLogger::~SingletonLogger() {

}

void SingletonLogger::SetLogger(std::shared_ptr<Logger> log) {
    logger_->SetLogger(log);
}

void SingletonLogger::SetLevel(LogLevel level){
    logger_->SetLevel(level);
    g_log_level.store(static_cast<uint8_t>(level), std::memory_order_relaxed);
}

void SingletonLogger::Debug(const char* file, uint32_t line, const char* log...) const {
    va_list list;
    va_start(list, log);
    logger_->Debug(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Info(const char* file, uint32_t line, const char* log...) const {
    va_list list;
    va_start(list, log);
    logger_->Info(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Warn(const char* file, uint32_t line, const char* log...) const {
    va_list list;
    va_start(list, log);
    logger_->Warn(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Error(const char* file, uint32_t line, const char* log...) const {
    va_list list;
    va_start(list, log);
    logger_->Error(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Fatal(const char* file, uint32_t line, const char* log...) const {
    va_list list;
    va_start(list, log);
    logger_->Fatal(file, line, log, list);
    va_end(list);
}

LogStreamParam SingletonLogger::GetStreamParam(LogLevel level, const char* file, uint32_t line) {
    return logger_->GetStreamParam(level, file, line);
}

}  // namespace common
}  // namespace cppnet