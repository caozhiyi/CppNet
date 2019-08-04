#ifdef __linux__

#ifndef HEADER_NET_LINUX_LINUXFUNC
#define HEADER_NET_LINUX_LINUXFUNC

namespace cppnet {

    int SetSocketNoblocking(unsigned int sock);

    int SetReusePort(unsigned int sock);

    void SetCoreFileUnlimit();

}
#endif
#endif // __linux__
