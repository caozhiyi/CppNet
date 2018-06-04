#include "OSInfo.h"

#ifdef linux
#include <sysconf.h> 
#else  
#define  WIN32_LEAN_AND_MEAN
#include <sysinfoapi.h>
#endif  

int GetCpuNum() {
	unsigned count = 1;
#ifdef linux
	count = sysconf(_SC_NPROCESSORS_CONF);
#else 
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	count = si.dwNumberOfProcessors;
#endif  
	return count;
}