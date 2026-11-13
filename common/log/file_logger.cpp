#include <cstdio>
#include <cstring>
#include <filesystem>
#include <system_error>

#include "common/log/file_logger.h"

namespace cppnet {
namespace common {

FileLogger::FileLogger(const std::string& file, 
    FileLoggerSpiltUnit unit, 
    uint16_t max_store_days,
    uint16_t time_offset):
    file_name_(file),
    time_offset_(time_offset),
    spilt_unit_(unit) {

    if (unit == FileLoggerSpiltUnit::kHour) {
        time_buf_len_ = 13; // xxxx-xx-xx:xx
        max_file_num_ = max_store_days * 24;

    } else {
        time_buf_len_ = 10; // xxxx-xx-xx
        max_file_num_ = max_store_days;
    }

    memset(time_, 0, kFileLoggerTimeBufSize);

    Start();
}

FileLogger::~FileLogger() {
    Stop();
    Join();
    stream_.close();
}

void FileLogger::Run() {
     while (!_stop) {
        auto log = Pop();
        if (log) {
            CheckTime(log->log_);
            if (stream_.is_open()) {
                stream_.write(log->log_, log->len_);
                stream_.put('\n');
                stream_.flush();
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
    if (spilt_unit_ == FileLoggerSpiltUnit::kHour) {
        time_buf_len_ = 13; // xxxx-xx-xx:xx
        max_file_num_ = max * 24;

    } else {
        time_buf_len_ = 10; // xxxx-xx-xx
        max_file_num_ = max;
    }

    CheckExpireFiles();
}

void FileLogger::CheckTime(char* log) {
    if (strncmp(time_, log + time_offset_, time_buf_len_) == 0) {
        return;
    }

    if (stream_.is_open()) {
        stream_.close();
    }
    
    // get new time and file name
    memcpy(time_, log + time_offset_, time_buf_len_);
    std::string file_name(file_name_);
    file_name.append(".");
    file_name.append(time_, time_buf_len_);
    file_name.append(".log");

    history_file_names_.push(file_name);
    CheckExpireFiles();

    // Ensure parent directory exists. std::ofstream::open does not create
    // missing directories, so without this every write would silently fail
    // when the configured log path (e.g. "./logs") has not been created yet.
    std::filesystem::path log_path(file_name);
    if (log_path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(log_path.parent_path(), ec);
        if (ec) {
            // Fall back to stderr — we cannot log via ourselves here.
            std::fprintf(stderr,
                "[FileLogger] failed to create log directory \"%s\": %s\n",
                log_path.parent_path().string().c_str(), ec.message().c_str());
        }
    }

    // open new log file
    stream_.open(file_name.c_str(), std::ios::app | std::ios::out);
    if (!stream_.is_open()) {
        std::fprintf(stderr,
            "[FileLogger] failed to open log file \"%s\"\n", file_name.c_str());
    }
}

void FileLogger::CheckExpireFiles() {
    // delete expire files
    while (history_file_names_.size() > max_file_num_) {
        std::string del_file = history_file_names_.front();
        history_file_names_.pop();
        std::remove(del_file.c_str());
    }
}

}  // namespace common
}  // namespace cppnet