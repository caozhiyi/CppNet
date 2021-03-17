#ifndef COMMON_UTIL_OS_RETURN
#define COMMON_UTIL_OS_RETURN

#include <cstdint>

namespace cppnet {

template <typename T>
struct SysCallResult {
  T _return_value;
  int32_t _errno;
};

using SysCallInt32Result = SysCallResult<int32_t>;
using SysCallInt64Result = SysCallResult<int64_t>;

}

#endif