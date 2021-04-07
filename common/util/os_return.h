// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

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