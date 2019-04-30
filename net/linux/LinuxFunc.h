#ifdef __linux__

#ifndef HEADER_LINUXFUNC
#define HEADER_LINUXFUNC

int SetSocketNoblocking(unsigned int sock);

int SetReusePort(unsigned int sock);

void SetCoreFileUnlimit();

#endif
#endif // __linux__
