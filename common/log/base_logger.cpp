// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "base_logger.h"
#include "logger_interface.h"
#include "common/util/time.h"
#include "common/alloter/normal_alloter.h"

namespace cppnet {

enum LogLevelMask {
    LLM_FATAL        = 0x01,
    LLM_ERROR        = 0x02,
    LLM_WARN         = 0x04,
    LLM_INFO         = 0x08,
    LLM_DEBUG        = 0x10,
};

static uint32_t FormatLog(const char* file, uint32_t line, const char* level, char* buf, uint32_t len) {
    // format level
    uint32_t curlen = snprintf(buf, len, "[%s|", level);

    // format time
    uint32_t size = __format_time_buf_size;
    GetFormatTime(buf + curlen, size);
    curlen += size;

    // format other info
    curlen += snprintf(buf + curlen, len - curlen, "|%s:%d] ", file, line);

    return curlen;
}

static uint32_t FormatLog(const char* file, uint32_t line, const char* level, const char* content, va_list list, char* buf, uint32_t len) {
    uint32_t curlen = 0;

    // format level time and file name
    curlen += FormatLog(file, line, level, buf, len);

    curlen += vsnprintf(buf + curlen, len - curlen, content, list);

    return curlen;
}

BaseLogger::BaseLogger(uint16_t cache_size, uint16_t block_size):
    _level(LL_INFO),
    _cache_size(cache_size),
    _block_size(block_size) {

    _allocter = MakeNormalAlloterPtr();
}

BaseLogger::~BaseLogger() {
    SetLevel(LL_NULL);
}

void BaseLogger::SetLevel(LogLevel level) { 
    _level = level; 
    if (_level > LL_NULL && _cache_queue.Empty()) {
        for (uint16_t i = 0; i < _cache_size; i++) {
            _cache_queue.Push(NewLog());
        }

    } else if (_level == LL_NULL) {
        size_t size = _cache_queue.Size();
        Log* log = nullptr;
        void* del = nullptr;
        for (size_t i = 0; i < size; i++) {
            if (_cache_queue.Pop(log)) {
                del = (void*)log;
                _allocter->Free(del);
            }
        }
    }
}

void BaseLogger::Debug(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(_level & LLM_DEBUG)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->_len = FormatLog(file, line, "DEB", content, list, log->_log, log->_len);

    if (_logger) {
        _logger->Debug(log);
    }
}

void BaseLogger::Info(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(_level & LLM_INFO)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->_len = FormatLog(file, line, "INF", content, list, log->_log, log->_len);

    if (_logger) {
        _logger->Info(log);
    }
}

void BaseLogger::Warn(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(_level & LLM_WARN)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->_len = FormatLog(file, line, "WAR", content, list, log->_log, log->_len);

    if (_logger) {
        _logger->Warn(log);
    }
}

void BaseLogger::Error(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(_level & LLM_ERROR)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->_len = FormatLog(file, line, "ERR", content, list, log->_log, log->_len);

    if (_logger) {
        _logger->Error(log);
    }
}

void BaseLogger::Fatal(const char* file, uint32_t line, const char* content, va_list list) {
    if (!(_level & LLM_FATAL)) {
        return;
    }

    std::shared_ptr<Log> log = GetLog();
    log->_len = FormatLog(file, line, "FAT", content, list, log->_log, log->_len);

    if (_logger) {
        _logger->Fatal(log);
    }
}

LogStreamParam BaseLogger::GetStreamParam(LogLevel level, const char* file, uint32_t line) {
    // check log level can print
    if (level > _level) {
        return std::make_pair(nullptr, nullptr);
    }

    std::shared_ptr<Log> log = GetLog();
    std::function<void(std::shared_ptr<Log>)> cb;
    switch (level)
    {
    case LL_NULL:
        break;
    case LL_FATAL:
        cb = [this](std::shared_ptr<Log> l) { _logger->Fatal(l); };
        log->_len = FormatLog(file, line, "FAT", log->_log, log->_len);
        break;
    case LL_ERROR:
        cb = [this](std::shared_ptr<Log> l) { _logger->Error(l); };
        log->_len = FormatLog(file, line, "ERR", log->_log, log->_len);
        break;
    case LL_WARN:
        cb = [this](std::shared_ptr<Log> l) { _logger->Warn(l); };
        log->_len = FormatLog(file, line, "WAR", log->_log, log->_len);
        break;
    case LL_INFO:
        cb = [this](std::shared_ptr<Log> l) { _logger->Info(l); };
        log->_len = FormatLog(file, line, "INF", log->_log, log->_len);
        break;
    case LL_DEBUG:
        cb = [this](std::shared_ptr<Log> l) { _logger->Debug(l); };
        log->_len = FormatLog(file, line, "DEB", log->_log, log->_len);
        break;
    default:
        return std::make_pair(nullptr, nullptr);
    }

    return std::make_pair(log, cb);
}

std::shared_ptr<Log> BaseLogger::GetLog() {
    Log* log = nullptr;
    if (_cache_queue.Pop(log)) {

    } else {
        log = NewLog();
    }

    return std::shared_ptr<Log>(log, [this](Log* &l) { FreeLog(l); });
}

void BaseLogger::FreeLog(Log* log) {
    if (_cache_queue.Size() > _cache_size) {
        void* del = (void*)log;
        _allocter->Free(del);

    } else {
        log->_len = _block_size;
        _cache_queue.Push(log);
    }
}

Log* BaseLogger::NewLog() {
    Log* item = (Log*)_allocter->MallocAlign(_block_size + sizeof(Log));

    item->_log = (char*)item + sizeof(Log);
    item->_len = _block_size;

    return item;
}

}