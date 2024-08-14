// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef QUIC_COMMON_LOG_STDOUT_LOGGER
#define QUIC_COMMON_LOG_STDOUT_LOGGER

#include <mutex>
#include "common/log/logger_interface.h"

namespace cppnet {

class StdoutLogger: 
    public Logger {

public:
    StdoutLogger() = default;
    ~StdoutLogger() = default;

    void Debug(std::shared_ptr<Log>& log);
    void Info(std::shared_ptr<Log>& log);
    void Warn(std::shared_ptr<Log>& log);
    void Error(std::shared_ptr<Log>& log);
    void Fatal(std::shared_ptr<Log>& log);

private:
    std::mutex _mutex;
};

} // namespace cppnet

#endif