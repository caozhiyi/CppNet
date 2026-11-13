// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef COMMON_UTIL_TIME
#define COMMON_UTIL_TIME

// VENDORED-FROM: quicX/src/common/util/time.h (CppNet adapted)
// Stage 3.1 of upgrade: relocated to cppnet::common namespace.
// Old API names (FTU_*, GetFormatTime, ...) are kept untouched — quicX's
// renamed kXxxFormat enums are NOT applied here, because CppNet business
// code still uses the legacy names. Re-exports at the end of this header
// keep cppnet:: references compiling unchanged.

#include <string>

namespace cppnet {
namespace common {

static const uint8_t __format_time_buf_size = sizeof("xxxx-xx-xx:xx:xx:xx:xxx");
// quicX-style alias for the same constant; introduced as part of the
// staged common/log/ replacement so the new logger code can compile
// without renaming. When common/util/ is replaced wholesale in a later
// stage, the older name will be retired.
static const uint8_t kFormatTimeBufSize = __format_time_buf_size;

enum FormatTimeUnit {
    FTU_YEAR        = 1, // 2021
    FTU_MONTH       = 2, // 2021-03
    FTU_DAY         = 3, // 2021-03-16
    FTU_HOUR        = 4, // 2021-03-16:10
    FTU_MINUTE      = 5, // 2021-03-16:10:03
    FTU_SECOND      = 6, // 2021-03-16:10:03:33
    FTU_MILLISECOND = 7, // 2021-03-16:10:03:33:258
};

// get format time string [xxxx-xx-xx xx:xx:xx]
std::string GetFormatTime(FormatTimeUnit unit = FTU_MILLISECOND);
// get format time string as [xxxx-xx-xx xx:xx:xx]
void GetFormatTime(char* buf, uint32_t& len, FormatTimeUnit unit = FTU_MILLISECOND);

// get utc time
uint64_t UTCTimeSec();
uint64_t UTCTimeMsec();

// sleep interval milliseconds
void Sleep(uint32_t interval);

}  // namespace common

// Self-export: keep cppnet:: references compiling unchanged.
using common::FormatTimeUnit;
using common::FTU_YEAR;
using common::FTU_MONTH;
using common::FTU_DAY;
using common::FTU_HOUR;
using common::FTU_MINUTE;
using common::FTU_SECOND;
using common::FTU_MILLISECOND;
using common::GetFormatTime;
using common::UTCTimeSec;
using common::UTCTimeMsec;
using common::Sleep;

}  // namespace cppnet

#endif