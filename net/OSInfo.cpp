#include "OSInfo.h"

#ifdef Linux
#include <sysconf.h> 
#else 
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