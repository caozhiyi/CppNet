// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef QUIC_COMMON_LOG_FILE_LOGGER
#define QUIC_COMMON_LOG_FILE_LOGGER

#include <mutex>
#include <queue>
#include <fstream>

#include "logger_interface.h"
#include "common/thread/thread_with_queue.h"

namespace cppnet {

enum FileLoggerSpiltUnit {
    FLSU_DAY  = 1,
    FLSU_HOUR = 2,
};

class FileLogger: 
    public Logger, 
    public ThreadWithQueue<std::shared_ptr<Log>> {

public:
    FileLogger(const std::string& file, 
        FileLoggerSpiltUnit unit = FLSU_DAY, 
        uint16_t max_store_days = 3,
        uint16_t time_offset = 5);

    ~FileLogger();

    void Run();
    void Stop();

    void Debug(std::shared_ptr<Log>& log);
    void Info(std::shared_ptr<Log>& log);
    void Warn(std::shared_ptr<Log>& log);
    void Error(std::shared_ptr<Log>& log);
    void Fatal(std::shared_ptr<Log>& log);

    void SetFileName(const std::string& name) { _file_name = name; }
    std::string GetFileName() { return _file_name; }

    void SetMaxStoreDays(uint16_t max);
    uint16_t GetMAxStorDays() { return _max_file_num; }

private:
    void CheckTime(char* log);
    void CheckExpireFiles();

private:
    enum : uint8_t {
        __file_logger_time_buf_size = sizeof("xxxx-xx-xx:xx")
    };
    std::string   _file_name;
    std::fstream  _stream;

    // for time check
    uint16_t _time_offset;
    uint8_t  _time_buf_len;
    FileLoggerSpiltUnit _spilt_unit;
    char     _time[__file_logger_time_buf_size];

    // for log file delete
    uint16_t _max_file_num;
    std::queue<std::string> _history_file_names;
};

} // namespace cppnet

#endif
