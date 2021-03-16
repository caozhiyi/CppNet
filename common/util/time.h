#ifndef COMMON_UTIL_TIME
#define COMMON_UTIL_TIME

#include <string>

namespace cppnet {

static const uint8_t __format_time_buf_size = sizeof("xxxx-xx-xx:xx:xx:xx:xxx");

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

}

#endif