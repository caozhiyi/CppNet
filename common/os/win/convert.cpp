#include <time.h>
#include <Windows.h>
#include "../convert.h"

namespace cppnet {

void Localtime(const uint64_t* time, void* out_tm) {
    localtime_s((tm*)out_tm, (time_t*)time);
}

char* ErrnoInfo(uint32_t err) {
    LPVOID  hlocal = NULL;
    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL
        , err, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPTSTR)&hlocal, 0, NULL);

    return (char*)hlocal;
}

}
