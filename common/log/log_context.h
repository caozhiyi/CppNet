// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

#ifndef COMMON_LOG_LOG_CONTEXT
#define COMMON_LOG_LOG_CONTEXT

#include <cstring>
#include <string>

namespace cppnet {
namespace common {

class LogContext {
public:
    // Append a tag to the current context.
    // Recommended to pass a pre-formatted string like "[conn:123]"
    static void Append(const char* tag, size_t len);

    // Truncate the context to a specific length.
    // Used to restore the context state (pop).
    static void Truncate(size_t len);

    // Get the current length of the context.
    static size_t Size();

    // Get the full context string.
    static const char* GetTag();
};

// RAII Guard for automatically appending and removing tags
class LogTagGuard {
public:
    explicit LogTagGuard(const std::string& tag) {
        old_len_ = LogContext::Size();
        LogContext::Append(tag.c_str(), tag.length());
    }

    // Overload for c-string to avoid std::string construction if not needed
    LogTagGuard(const char* tag) {
        old_len_ = LogContext::Size();
        LogContext::Append(tag, strlen(tag));
    }

    ~LogTagGuard() { LogContext::Truncate(old_len_); }

private:
    size_t old_len_;
};

}  // namespace common

// Compatibility aliases: legacy code uses these unqualified.
using common::LogContext;
using common::LogTagGuard;

}  // namespace cppnet

#endif
