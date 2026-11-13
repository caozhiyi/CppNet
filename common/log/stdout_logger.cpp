#include <iostream>
#include "common/log/stdout_logger.h"

namespace cppnet {
namespace common {

StdoutLogger::StdoutLogger() {

}

StdoutLogger::~StdoutLogger() {

}

void StdoutLogger::Debug(std::shared_ptr<Log>& log) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::cout<< log->log_ << std::endl;
    }
    Logger::Debug(log);
}

void StdoutLogger::Info(std::shared_ptr<Log>& log) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::cout<< log->log_ << std::endl;
    }
    Logger::Info(log);
}

void StdoutLogger::Warn(std::shared_ptr<Log>& log) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::cout<< log->log_ << std::endl;
    }
    Logger::Warn(log);
}

void StdoutLogger::Error(std::shared_ptr<Log>& log) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::cerr<< log->log_ << std::endl;
    }
    Logger::Error(log);
}

void StdoutLogger::Fatal(std::shared_ptr<Log>& log) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        std::cerr<< log->log_ << std::endl;
    }
    Logger::Fatal(log);
}

}  // namespace common
}  // namespace cppnet