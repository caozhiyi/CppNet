#ifdef __linux__

#ifndef HEADER_CEPOOLIMPL
#define HEADER_CEPOOLIMPL

#include <sys/epoll.h>

class CEpollImpl
{
public:
	CEpollImpl();
	~CEpollImpl();
};

#endif
#endif