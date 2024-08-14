// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <cstdio>
#include <cstring>
#include "common/util/time.h"
#include "common/log/file_logger.h"

namespace cppnet {

FileLogger::FileLogger(const std::string& file, 
    FileLoggerSpiltUnit unit, 
    uint16_t max_store_days,
    uint16_t time_offset):
    _file_name(file),
    _time_offset(time_offset),
    _spilt_unit(unit) {

    if (unit == FLSU_HOUR) {
        _time_buf_len = 13; // xxxx-xx-xx:xx
        _max_file_num = max_store_days * 24;

    } else {
        _time_buf_len = 10; // xxxx-xx-xx
        _max_file_num = max_store_days;
    }

    memset(_time, 0, __file_logger_time_buf_size);

    Start();
}

FileLogger::~FileLogger() {
    Stop();
    Join();
    _stream.close();
}

void FileLogger::Run() {
     while (!_stop) {
        auto log = Pop();
        if (log) {
            CheckTime(log->_log);
            if (_stream.is_open()) {
                _stream.write(log->_log, log->_len);
                _stream.put('\n');
                _stream.flush();
            }

        } else {
            break;
        }
    }
}

void FileLogger::Stop() {
    _stop = true;
    Push(nullptr);
}

void FileLogger::Debug(std::shared_ptr<Log>& log) {
    Push(log);
    Logger::Debug(log);
}

void FileLogger::Info(std::shared_ptr<Log>& log) {
    Push(log);
    Logger::Info(log);
}

void FileLogger::Warn(std::shared_ptr<Log>& log) {
    Push(log);
    Logger::Warn(log);
}

void FileLogger::Error(std::shared_ptr<Log>& log) {
    Push(log);
    Logger::Error(log);
}

void FileLogger::Fatal(std::shared_ptr<Log>& log) {
    Push(log);
    Logger::Fatal(log);
}

void FileLogger::SetMaxStoreDays(uint16_t max) {
    if (_spilt_unit == FLSU_HOUR) {
        _time_buf_len = 13; // xxxx-xx-xx:xx
        _max_file_num = max * 24;

    } else {
        _time_buf_len = 10; // xxxx-xx-xx
        _max_file_num = max;
    }

    CheckExpireFiles();
}

void FileLogger::CheckTime(char* log) {
    if (strncmp(_time, log + _time_offset, _time_buf_len) == 0) {
        return;
    }

    if (_stream.is_open()) {
        _stream.close();
    }
    
    // get new time and file name
    memcpy(_time, log + _time_offset, _time_buf_len);
    std::string file_name(_file_name);
    file_name.append(".");
    file_name.append(_time, _time_buf_len);
    file_name.append(".log");

    _history_file_names.push(file_name);
    CheckExpireFiles();

    // open new log file
    _stream.open(file_name.c_str(), std::ios::app | std::ios::out);
}

void FileLogger::CheckExpireFiles() {
    // delete expire files
    while (_history_file_names.size() > _max_file_num) {
        std::string del_file = _history_file_names.front();
        _history_file_names.pop();
        std::remove(del_file.c_str());
    }
}

} // namespace cppnet
