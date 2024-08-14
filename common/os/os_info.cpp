// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <thread>
#include "common/os/os_info.h"

uint32_t cppnet::GetCpuNum() {
    return std::thread::hardware_concurrency();
}