// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_UTIL_OS_RETURN
#define COMMON_UTIL_OS_RETURN

// VENDORED-FROM: quicX/src/common/util/os_return.h (CppNet adapted)
// Stage 3.1 of upgrade: relocated to cppnet::common namespace.
// Old aliases are re-exported via using-declarations at the end (self-export).

#include <cstdint>

namespace cppnet {
namespace common {

template <typename T>
struct SysCallResult {
  T _return_value;
  int32_t _errno;
};

using SysCallInt32Result = SysCallResult<int32_t>;
using SysCallInt64Result = SysCallResult<int64_t>;

}  // namespace common

// Self-export: keep cppnet::SysCallXxxResult working for existing call sites.
template <typename T>
using SysCallResult = common::SysCallResult<T>;
using SysCallInt32Result = common::SysCallInt32Result;
using SysCallInt64Result = common::SysCallInt64Result;

}  // namespace cppnet

#endif