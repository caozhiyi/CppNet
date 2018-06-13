#ifdef __linux__

#ifndef HEADER_LINUXFUNC
#define HEADER_LINUXFUNC

#include <fcntl.h>

int SetSocketNoblocking(unsigned int sock) {
	int old_option = fcntl(sock, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(sock, F_SETFL, new_option);
	return old_option;
}

#endif
#endif // __linux__
