#include <stdarg.h>

#include "log.h"
#include "base_logger.h"

namespace cppnet {

SingletonLogger::SingletonLogger() {
    _logger = std::make_shared<BaseLogger>(__log_cache_size, __log_block_size);
}

SingletonLogger::~SingletonLogger() {

}

void SingletonLogger::SetLogger(std::shared_ptr<Logger> log) {
    _logger->SetLogger(log);
}

void SingletonLogger::SetLevel(LogLevel level){
    _logger->SetLevel(level);
}

void SingletonLogger::Debug(const char* file, uint32_t line, const char* log...){
    va_list list;
    va_start(list, log);
    _logger->Debug(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Info(const char* file, uint32_t line, const char* log...){
    va_list list;
    va_start(list, log);
    _logger->Info(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Warn(const char* file, uint32_t line, const char* log...){
    va_list list;
    va_start(list, log);
    _logger->Warn(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Error(const char* file, uint32_t line, const char* log...){
    va_list list;
    va_start(list, log);
    _logger->Error(file, line, log, list);
    va_end(list);
}

void SingletonLogger::Fatal(const char* file, uint32_t line, const char* log...){
    va_list list;
    va_start(list, log);
    _logger->Fatal(file, line, log, list);
    va_end(list);
}


}