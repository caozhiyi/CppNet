#ifdef __linux__
#include <unistd.h>
#else 
#include <sysinfoapi.h>
#endif
#include "OSInfo.h"

uint32_t cppnet::GetCpuNum() {
    unsigned count = 1;
#ifdef __linux__
    count = sysconf(_SC_NPROCESSORS_CONF);
#else 
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    count = si.dwNumberOfProcessors;
#endif  
    return count;
}