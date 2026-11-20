#ifndef COMMON_LOG_STDOUT_LOGGER
#define COMMON_LOG_STDOUT_LOGGER

#include <mutex>
#include "common/log/if_logger.h"

namespace cppnet {
namespace common {

class StdoutLogger:
    public Logger {

public:
    StdoutLogger();
    ~StdoutLogger();

    void Debug(std::shared_ptr<Log>& log);
    void Info(std::shared_ptr<Log>& log);
    void Warn(std::shared_ptr<Log>& log);
    void Error(std::shared_ptr<Log>& log);
    void Fatal(std::shared_ptr<Log>& log);

private:
    std::mutex mutex_;
};

}  // namespace common

// Compatibility alias: legacy code uses cppnet::StdoutLogger directly.
using common::StdoutLogger;

}  // namespace cppnet

#endif