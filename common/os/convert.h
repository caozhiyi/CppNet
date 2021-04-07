// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_OS_CONVERT
#define COMMON_OS_CONVERT

#include <cstdint>

namespace cppnet {

void Localtime(const uint64_t* time, void* out_tm);

char* ErrnoInfo(uint32_t err);

}

#endif