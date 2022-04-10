// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <thread>
#include <cstring>
#ifdef __win__
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <signal.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/version.h>
#define WSAEWOULDBLOCK 10035
#endif

#include "epoll_action.h"
#include "include/cppnet_type.h"
#include "cppnet/cppnet_config.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/event_interface.h"

#include "common/log/log.h"
#include "common/util/time.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/network/io_handle.h"
#include "common/alloter/alloter_interface.h"

namespace cppnet {

std::shared_ptr<EventActions> MakeEventActions() {
    return std::make_shared<EpollEventActions>();
}

EpollEventActions::EpollEventActions():
#ifdef __win__
    _epoll_handler(nullptr) {
#else
    _epoll_handler(-1) {
#endif
    _active_list.resize(1024);
    memset(_pipe, 0, sizeof(_pipe));
    memset(&_pipe_content, 0, sizeof(_pipe_content));
}

EpollEventActions::~EpollEventActions() {
    if (_epoll_handler > 0) {
#ifdef __win__
        epoll_close(_epoll_handler);
#else
        close(_epoll_handler);
#endif
    }
}

bool EpollEventActions::Init(uint32_t thread_num) {
    // get EPOLL handle. the param is invalid since LINUX 2.6.8
    _epoll_handler = epoll_create(1500);
#ifdef __win__
    if (_epoll_handler == nullptr) {
#else
    if (_epoll_handler == -1) {
#endif
        LOG_FATAL("EPOLL init failed! error : %d", errno);
        return false;
    }
#ifdef __win__
    if (!Pipe(_pipe)) {
#else
    if (pipe((int*)_pipe) == -1) {
#endif
        LOG_FATAL("pipe init failed! error : %d", errno);
        return false;
    }

    SocketNoblocking(_pipe[1]);
    SocketNoblocking(_pipe[0]);

    _pipe_content.events = EPOLLIN;
    _pipe_content.data.fd = _pipe[0];
    int32_t ret = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, _pipe[0], &_pipe_content);
    if (ret < 0) {
        LOG_FATAL("add pipe handle to EPOLL failed! error :%d", errno);
        return false;
    }
    return true;
}

bool EpollEventActions::Dealloc() {
    Wakeup();
    return true;
}

bool EpollEventActions::AddSendEvent(Event* event) {
    if (event->GetType() & ET_WRITE) {
        return false;
    }
    event->AddType(ET_WRITE);

    epoll_event* ep_event = (epoll_event*)event->GetData();
    if (!ep_event) {
        if (!MakeEpollEvent(event, ep_event)) {
            return false;
        }
    }

    // already in EPOLL
    if (ep_event->events & EPOLLOUT) {
        return true;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }

    if (AddEvent(ep_event, EPOLLOUT, sock->GetSocket(), event->GetType() & ET_INACTIONS)) {
        event->AddType(ET_INACTIONS);
        return true;
    }

    LOG_WARN("add event to socket failed! event %s", "AddSendEvent");
    return false;
}

bool EpollEventActions::AddRecvEvent(Event* event) {
    if (event->GetType() & ET_READ) {
        return false;
    }
    event->AddType(ET_READ);
    
    epoll_event* ep_event = (epoll_event*)event->GetData();
    if (!ep_event) {
        if (!MakeEpollEvent(event, ep_event)) {
            return false;
        }
    }

    // already in EPOLL
    if (ep_event->events & EPOLLIN) {
        return true;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }

    if (AddEvent(ep_event, EPOLLIN, sock->GetSocket(), event->GetType() & ET_INACTIONS)) {
        event->AddType(ET_INACTIONS);
        return true;
    }

    LOG_WARN("add event to socket failed! event %s", "AddRecvEvent");
    return false;
}

bool EpollEventActions::AddAcceptEvent(Event* event) {
    if (event->GetType() & ET_ACCEPT) {
        return false;
    }
    event->AddType(ET_ACCEPT);

    epoll_event* ep_event = (epoll_event*)event->GetData();
    if (!ep_event) {
        // TODO where to delete it.
        ep_event = new epoll_event;
        memset(ep_event, 0, sizeof(epoll_event));
        event->SetData(ep_event);

        ep_event->data.ptr = (void*)event;
    }

     // already in EPOLL
    if (ep_event->events & EPOLLIN) {
        return true;
    }

    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }

    if (AddEvent(ep_event, EPOLLIN, sock->GetSocket(), event->GetType() & ET_INACTIONS)) {
        event->AddType(ET_INACTIONS);
        return true;
    }
    
    return false;
}

bool EpollEventActions::AddConnection(Event* event, Address& addr) {
    if (event->GetType() & ET_CONNECT) {
        return false;
    }
    event->AddType(ET_CONNECT);

    auto sock = event->GetSocket();
    if (sock) {
        // the socket must not in EPOLL
        if (event->GetType() & ET_INACTIONS) {
            return false;
        }
   
        // set no unblocking before connect.
        SocketNoblocking(sock->GetSocket());

        auto ret = OsHandle::Connect(sock->GetSocket(), addr);

        auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
        if (ret._return_value == 0) {
            rw_sock->OnConnect(CEC_SUCCESS);
            return true;

        } else if (ret._errno == EINPROGRESS || ret._errno == WSAEWOULDBLOCK) {
            auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
            rw_sock->AddTimer(__connect_recheck_time_ms);
            return false;
        }
        rw_sock->OnConnect(CEC_CONNECT_REFUSE);
        LOG_WARN("connect event failed! %d", ret._errno);
        return false;
    }
    LOG_WARN("connection event is already destroyed!,%s", "AddConnection");
    return false;
}

bool EpollEventActions::AddDisconnection(Event* event) {
    if (event->GetType() & ET_DISCONNECT) {
        return false;
    }
    event->AddType(ET_DISCONNECT);
    
    auto sock = event->GetSocket();
    if (!sock) {
        return false;
    }
    
    std::shared_ptr<RWSocket> socket = std::dynamic_pointer_cast<RWSocket>(sock);
    if (!DelEvent(event)) {
        return false;
    }
    OsHandle::Close(socket->GetSocket());
    socket->OnDisConnect(CEC_SUCCESS);
    return true;
}

bool EpollEventActions::DelEvent(Event* event) {
    auto sock = event->GetSocket();
    if (!sock) {
        return false;
    }
    epoll_event* ev = (epoll_event*)event->GetData();
    int32_t ret = epoll_ctl(_epoll_handler, EPOLL_CTL_DEL, sock->GetSocket(), ev);
    if (ret < 0) {
        LOG_ERROR("remove event from EPOLL failed! error :%d, socket : %d", errno, sock->GetSocket());
        return false;
    }

    event->ClearType();
    LOG_DEBUG("remove a socket from EPOLL, %d", sock->GetSocket());
    return true;
}

void EpollEventActions::ProcessEvent(int32_t wait_ms) {
    int16_t ret = epoll_wait(_epoll_handler, &*_active_list.begin(), (int)_active_list.size(), wait_ms);
    if (ret == -1) {
        if (errno == EINTR) {
            return;
        }
        LOG_ERROR("EPOLL wait failed! error:%d, info:%s", errno, ErrnoInfo(errno));

    } else {
        LOG_DEBUG("EPOLL get events! num:%d, TheadId: %ld", ret, std::this_thread::get_id());

        OnEvent(_active_list, ret);
    }
}

void EpollEventActions::Wakeup() {
#ifdef __win__
    if (send(_pipe[1], "1", 1, 0) <= 0) {
#else
    if (write(_pipe[1], "1", 1) <= 0) {
#endif
        LOG_ERROR_S << "write to pipe failed when weak up.";
    }
}

void EpollEventActions::OnEvent(std::vector<epoll_event>& event_vec, int16_t num) {
    std::shared_ptr<Socket> sock;
    Event* event = nullptr;

    for (int i = 0; i < num; i++) {
        if ((uint32_t)event_vec[i].data.fd == _pipe[0]) {
            LOG_WARN("weak up the IO thread, index : %d", i);
            char buf[4];
#ifdef __win__
            if (recv(_pipe[0], buf, 1, 0) <= 0) {
#else
            if (read(_pipe[0], buf, 1) <= 0) {
#endif
                LOG_ERROR_S << "read from pipe failed when weak up.";
            }
            continue;
        }

        event = (Event*)event_vec[i].data.ptr;
        sock = event->GetSocket();
        if (!sock) {
            LOG_WARN("EPOLL weak up but socket already destroy, index : %d", i);
            continue;
        }

        // accept event
        if (event->GetType() & ET_ACCEPT) {
            std::shared_ptr<ConnectSocket> connect_sock = std::dynamic_pointer_cast<ConnectSocket>(sock);
            connect_sock->OnAccept();

        } else {
            std::shared_ptr<RWSocket> rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
            if (event_vec[i].events & EPOLLIN) {
                // close
                if (event_vec[i].events & EPOLLRDHUP) {
                    rw_sock->OnDisConnect(CEC_CLOSED);
                    
                } else {
                    rw_sock->OnRead();
                }
            }

            if (event_vec[i].events & EPOLLOUT) {
                rw_sock->OnWrite();
            }

            if (event_vec[i].events & EPOLLHUP) {
                rw_sock->OnDisConnect(CEC_CLOSED);
            }
        }
    }
}

bool EpollEventActions::AddEvent(epoll_event* ev, int32_t event_flag, uint64_t sock, bool in_actions) {
    //if not add to EPOLL
    if (!(ev->events & event_flag)) {
#ifdef __win__
        ev->events |= event_flag;
#else
        if (__epoll_use_et) {
            ev->events |= event_flag | EPOLLET;

        } else {
             ev->events |= event_flag;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
        if (__epoll_exclusive) {
            ev->events |= EPOLLEXCLUSIVE;
        }
#endif
#endif

        int32_t ret = 0;
        if (in_actions) {
            ret = epoll_ctl(_epoll_handler, EPOLL_CTL_MOD, sock, ev);

        } else {
            ret = epoll_ctl(_epoll_handler, EPOLL_CTL_ADD, sock, ev);
        }
            
        if (ret == 0) {
            return true;
        }
        LOG_ERROR("modify event to EPOLL failed! error :%d, sock: %d", errno, sock);
    }
    return false;
}

bool EpollEventActions::MakeEpollEvent(Event* event, epoll_event* &ep_event) {
    auto sock = event->GetSocket();
    if (!sock) {
        LOG_WARN("socket is already destroyed! event %s", "AddSendEvent");
        return false;
    }

    auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
    ep_event = rw_sock->GetAlloter()->PoolNew<epoll_event>();
    memset(ep_event, 0, sizeof(epoll_event));
    event->SetData(ep_event);
    ep_event->data.ptr = (void*)event;

    return true;
}

#ifdef __win__
bool EpollEventActions::Pipe(SOCKET fd[2]) {
        SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0)
        return false;

    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port = 0;	/* kernel chooses port.	 */
    if (bind(listener, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) == -1) {
        return false;
    }
    if (listen(listener, 1) == -1) {
        return false;
    }
    SOCKET connector = socket(AF_INET, SOCK_STREAM, 0);
    if (connector < 0) {
        return false;
    }

    /* We want to find out the port number to connect to.  */
    struct sockaddr_in connect_addr;
    int size = sizeof(connect_addr);
    if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1) {
        return false;
    }
        
    if (size != sizeof(connect_addr)) {
        return false;
    }

    if (connect(connector, (struct sockaddr *) &connect_addr, sizeof(connect_addr)) == -1) {
        return false;
    }

    size = sizeof(listen_addr);
    SOCKET acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
    if (acceptor < 0) {
        return false;
    }
    if (size != sizeof(listen_addr)) {
        return false;
    }

    // check options
    if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1) {
        return false;
    }
    if (size != sizeof(connect_addr)
        || listen_addr.sin_family != connect_addr.sin_family
        || listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
        || listen_addr.sin_port != connect_addr.sin_port) {
        return false;
    }
    closesocket(listener);

    fd[0] = connector;
    fd[1] = acceptor;
    return true;
}
#endif

}