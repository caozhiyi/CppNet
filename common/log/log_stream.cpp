#include "common/log/log.h"
#include "common/log/if_logger.h"
#include "common/log/log_stream.h"


namespace cppnet {
namespace common {

#define CHECK_CONTINUE()  do{ if (!log_ || log_->len_ >= kLogBlockSize) { return *this; }  } while(0);

LogStream::LogStream(const LogStreamParam& param):
    log_(param.first),
    call_back_(param.second) {

}

LogStream::~LogStream() {
    if (log_ && call_back_) {
        call_back_(log_);
    }
}

LogStream& LogStream::operator<<(bool v) {
    CHECK_CONTINUE()

    char c = v ? '1' : '0';
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%c", c);
    return *this;
}

LogStream& LogStream::operator<<(int8_t v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%d", v);
    return *this;
}

LogStream& LogStream::operator<<(uint8_t v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%u", v);
    return *this;
}

LogStream& LogStream::operator<<(int16_t v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%d", v);
    return *this;
}

LogStream& LogStream::operator<<(uint16_t v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%u", v);
    return *this;
}

LogStream& LogStream::operator<<(int32_t v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%d", v);
    return *this;
}

LogStream& LogStream::operator<<(uint32_t v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%u", v);
    return *this;
}

LogStream& LogStream::operator<<(int64_t v) {
    CHECK_CONTINUE()
#ifdef __win__
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%I64d", v);
#elif __APPLE__
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%lld", v);
#else
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%ld", v);
#endif
    return *this;
}

LogStream& LogStream::operator<<(uint64_t v) {
    CHECK_CONTINUE()
#ifdef __win__
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%I64u", v);
#elif __APPLE__
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%llu", v);
#else
    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%lu", v);
#endif
    return *this;
}

LogStream& LogStream::operator<<(float v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%.10lf", v);
    return *this;
}

LogStream& LogStream::operator<<(double v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%.20lf", v);
    return *this;
}

LogStream& LogStream::operator<<(const std::string& v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%s", v.c_str());
    return *this;
}

LogStream& LogStream::operator<<(const char* v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%s", v);
    return *this;
}

LogStream& LogStream::operator<<(char v) {
    CHECK_CONTINUE()

    log_->len_ += snprintf(log_->log_ + log_->len_, kLogBlockSize - log_->len_, "%c", v);
    return *this;
}

}  // namespace common
}  // namespace cppnet