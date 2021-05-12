// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "log.h"
#include "log_stream.h"
#include "logger_interface.h"

namespace cppnet {

#define CHECK_CONTINUE()  do{ if (!_log || _log->_len >= __log_block_size) { return *this; }  } while(0);

LogStream::LogStream(const LogStreamParam& param):
    _log(param.first),
    _call_back(param.second) {

}

LogStream::~LogStream() {
    if (_log && _call_back) {
        _call_back(_log);
    }
}

LogStream& LogStream::operator<<(bool v) {
    CHECK_CONTINUE()

    char c = v ? '1' : '0';
    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%c", c);
    return *this;
}

LogStream& LogStream::operator<<(int8_t v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%d", v);
    return *this;
}

LogStream& LogStream::operator<<(uint8_t v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%u", v);
    return *this;
}

LogStream& LogStream::operator<<(int16_t v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%d", v);
    return *this;
}

LogStream& LogStream::operator<<(uint16_t v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%u", v);
    return *this;
}

LogStream& LogStream::operator<<(int32_t v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%d", v);
    return *this;
}

LogStream& LogStream::operator<<(uint32_t v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%u", v);
    return *this;
}

LogStream& LogStream::operator<<(int64_t v) {
    CHECK_CONTINUE()
#ifdef __win__
    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%I64d", v);
#else
    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%ld", v);
#endif
    return *this;
}

LogStream& LogStream::operator<<(uint64_t v) {
    CHECK_CONTINUE()
#ifdef __win__
    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%I64u", v);
#else
    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%lu", v);
#endif
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%.10lf", v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%.20lf", v);
    return *this;
}


LogStream& LogStream::operator<<(const std::string& v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%s", v.c_str());
    return *this;
}

LogStream& LogStream::operator<<(const char* v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%s", v);
    return *this;
}

LogStream& LogStream::operator<<(char v) {
    CHECK_CONTINUE()

    _log->_len += snprintf(_log->_log + _log->_len, __log_block_size - _log->_len, "%c", v);
    return *this;
}

}