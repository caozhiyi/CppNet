#ifndef COMMON_LOG_LOG
#define COMMON_LOG_LOG

#include <atomic>
#include <memory>
#include <cstdint>

#include "common/log/log_stream.h"
#include "common/util/singleton.h"

namespace cppnet {
namespace common {

// log level and switch
enum LogLevel: uint8_t {
    kNull  = 0x00, // not print log
    kFatal = 0x01,
    kError = 0x02 | kFatal,
    kWarn  = 0x04 | kError,
    kInfo  = 0x08 | kWarn,
    kDebug = 0x10 | kInfo,
};

enum LogLevelMaskBit : uint8_t {
    kFatalBit = 0x01,
    kErrorBit = 0x02,
    kWarnBit  = 0x04,
    kInfoBit  = 0x08,
    kDebugBit = 0x10,
};

// global log level for fast short-circuit in macros (avoid argument evaluation
// like building std::string when the log will be discarded anyway).
extern std::atomic<uint8_t> g_log_level;

inline bool LogLevelEnabled(uint8_t bit) {
    return (g_log_level.load(std::memory_order_relaxed) & bit) != 0;
}

}  // namespace common
}  // namespace cppnet

// log interface for user
#define LOG_SET(log)         ::cppnet::common::SingletonLogger::Instance().SetLogger(log)
#define LOG_SET_LEVEL(level) ::cppnet::common::SingletonLogger::Instance().SetLevel(level)

// NOTE: level short-circuit at the macro layer prevents evaluation of
// arguments (e.g. FrameType2String(...).c_str()) when the level is disabled,
// which is important for hot paths.
#define LOG_DEBUG(log, ...)  do { if (::cppnet::common::LogLevelEnabled(::cppnet::common::kDebugBit)) \
    ::cppnet::common::SingletonLogger::Instance().Debug(__FILE__, __LINE__, log, ##__VA_ARGS__); } while (0)
#define LOG_INFO(log, ...)   do { if (::cppnet::common::LogLevelEnabled(::cppnet::common::kInfoBit))  \
    ::cppnet::common::SingletonLogger::Instance().Info (__FILE__, __LINE__, log, ##__VA_ARGS__); } while (0)
#define LOG_WARN(log, ...)   do { if (::cppnet::common::LogLevelEnabled(::cppnet::common::kWarnBit))  \
    ::cppnet::common::SingletonLogger::Instance().Warn (__FILE__, __LINE__, log, ##__VA_ARGS__); } while (0)
#define LOG_ERROR(log, ...)  do { if (::cppnet::common::LogLevelEnabled(::cppnet::common::kErrorBit)) \
    ::cppnet::common::SingletonLogger::Instance().Error(__FILE__, __LINE__, log, ##__VA_ARGS__); } while (0)
#define LOG_FATAL(log, ...)  do { if (::cppnet::common::LogLevelEnabled(::cppnet::common::kFatalBit)) \
    ::cppnet::common::SingletonLogger::Instance().Fatal(__FILE__, __LINE__, log, ##__VA_ARGS__); } while (0)

#define LOG_DEBUG_S ::cppnet::common::LogStream(::cppnet::common::SingletonLogger::Instance().GetStreamParam(::cppnet::common::LogLevel::kDebug, __FILE__, __LINE__))
#define LOG_INFO_S  ::cppnet::common::LogStream(::cppnet::common::SingletonLogger::Instance().GetStreamParam(::cppnet::common::LogLevel::kInfo,  __FILE__, __LINE__))
#define LOG_WARN_S  ::cppnet::common::LogStream(::cppnet::common::SingletonLogger::Instance().GetStreamParam(::cppnet::common::LogLevel::kWarn,  __FILE__, __LINE__))
#define LOG_ERROR_S ::cppnet::common::LogStream(::cppnet::common::SingletonLogger::Instance().GetStreamParam(::cppnet::common::LogLevel::kError, __FILE__, __LINE__))
#define LOG_FATAL_S ::cppnet::common::LogStream(::cppnet::common::SingletonLogger::Instance().GetStreamParam(::cppnet::common::LogLevel::kFatal, __FILE__, __LINE__))

namespace cppnet {
namespace common {

// log cache config
static const uint16_t kLogCacheSize = 20;
static const uint16_t kLogBlockSize = 2048;


class Logger;
class BaseLogger;
class SingletonLogger:
    public ::cppnet::Singleton<SingletonLogger> {

public:
    SingletonLogger();
    ~SingletonLogger();

    void SetLogger(std::shared_ptr<Logger> log);

    void SetLevel(LogLevel level);

    // for log print as printf
    void Debug(const char* file, uint32_t line, const char* log...) const;
    void Info(const char* file, uint32_t line, const char* log...) const;
    void Warn(const char* file, uint32_t line, const char* log...) const;
    void Error(const char* file, uint32_t line, const char* log...) const;
    void Fatal(const char* file, uint32_t line, const char* log...) const;

    // for log stream
    LogStreamParam GetStreamParam(LogLevel level, const char* file, uint32_t line);

private:
    std::shared_ptr<BaseLogger> logger_;
};

}  // namespace common

// Compatibility aliases: legacy code references these symbols unqualified
// from inside namespace cppnet. Re-export the symbols declared in this
// header so business code stays untouched. Symbols defined in sibling
// headers (FileLogger / StdoutLogger / LogContext / LogTagGuard) are
// re-exported by their own headers.
using common::Logger;
using common::BaseLogger;
using common::SingletonLogger;
using common::LogStream;
using common::LogStreamParam;
using common::LogLevel;
using common::kNull;
using common::kFatal;
using common::kError;
using common::kWarn;
using common::kInfo;
using common::kDebug;

}

#endif