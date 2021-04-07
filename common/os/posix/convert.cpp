// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <time.h>
#include <string.h>
#include "../convert.h"

namespace cppnet {

void Localtime(const uint64_t* time, void* out_tm) {
    ::localtime_r((time_t*)time, (tm*)out_tm);
}

char* ErrnoInfo(uint32_t err) {
    return strerror(err);
}

}