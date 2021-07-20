// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi>

#ifndef COMMON_LOG_BASE_LOGGER_H_
#define COMMON_LOG_BASE_LOGGER_H_

#include <memory>
#include <cstdint>
#include <cstdarg>

#include "common/log/log.h"
#include "common/log/log_stream.h"
#include "common/structure/thread_safe_queue.h"

namespace cppnet {

// basic management class of log printing
struct Log;
class Logger;
class Alloter;
class BaseLogger {
 public:
  BaseLogger(uint16_t cache_size, uint16_t block_size);
  ~BaseLogger();

  void SetLogger(std::shared_ptr<Logger> log) { _logger = log; }

  void SetLevel(LogLevel level);

  void Debug(const char* file, uint32_t line,
    const char* content, va_list list);
  void Info(const char* file, uint32_t line,
    const char* content, va_list list);
  void Warn(const char* file, uint32_t line,
    const char* content, va_list list);
  void Error(const char* file, uint32_t line,
    const char* content, va_list list);
  void Fatal(const char* file, uint32_t line,
    const char* content, va_list list);

  LogStreamParam GetStreamParam(LogLevel level,
    const char* file, uint32_t line);

 private:
  std::shared_ptr<Log> GetLog();
  void FreeLog(Log* log);
  Log* NewLog();

 protected:
  uint16_t _level;
  uint16_t _cache_size;
  uint16_t _block_size;

  std::shared_ptr<Alloter> _allocter;
  ThreadSafeQueue<Log*>    _cache_queue;
  std::shared_ptr<Logger>  _logger;
};

}  // namespace cppnet

#endif  // COMMON_LOG_BASE_LOGGER_H_
