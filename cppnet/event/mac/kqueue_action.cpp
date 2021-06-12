// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <thread>
#include <unistd.h>    // for close

#include "kqueue_action.h"
#include "include/cppnet_type.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/socket/connect_socket.h"
#include "cppnet/event/event_interface.h"

#include "common/log/log.h"
#include "common/util/time.h"
#include "common/os/convert.h"
#include "common/util/os_return.h"
#include "common/network/socket.h"
#include "common/network/address.h"
#include "common/network/io_handle.h"
#include "common/timer/timer_interface.h"

namespace cppnet {

std::shared_ptr<EventActions> MakeEventActions() {
    return std::make_shared<KqueueEventActions>();
}

KqueueEventActions::KqueueEventActions():
    _kqueue_handler(-1) {
    _active_list.resize(1024);
    _kqueue_timeout.tv_nsec = 0;
    _kqueue_timeout.tv_sec = 0;
}

KqueueEventActions::~KqueueEventActions() {
    if (_kqueue_handler > 0) {
        OsHandle::Close(_kqueue_handler);
    }
    if (_pipe[0] > 0) {
        OsHandle::Close(_pipe[0]);
    }
    if (_pipe[0] > 0) {
        OsHandle::Close(_pipe[0]);
    }
}

bool KqueueEventActions::Init(uint32_t thread_num) {
    _kqueue_handler = kqueue();
    if (_kqueue_handler < 0) {
        LOG_ERROR("create kqueue failed. errno:%d", errno);
        return false;
    }

    if (pipe((int*)_pipe) == -1) {
        LOG_FATAL("pipe init failed! error : %d", errno);
        return false;
    }
    
    SocketNoblocking(_pipe[0]);
    SocketNoblocking(_pipe[1]);

    struct kevent ev;
    EV_SET(&ev, _pipe[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    _change_list.push_back(ev);
    return true;
}

bool KqueueEventActions::Dealloc() {
    Wakeup();
    return true;
}

bool KqueueEventActions::AddSendEvent(Event* event) {
    if (event->GetType() & ET_WRITE) {
        return false;
    }
    event->AddType(ET_WRITE);

    auto sock = event->GetSocket();
    if (sock) {
        struct kevent ev;
        EV_SET(&ev, sock->GetSocket(), EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, (void*)event);

        _change_list.push_back(ev);
        return true;
    }
    LOG_WARN("socket is already distroyed! event %s", "AddSendEvent");
    return false;
}

bool KqueueEventActions::AddRecvEvent(Event* event) {
    if (event->GetType() & ET_READ) {
        return false;
    }
    event->AddType(ET_READ);

    auto sock = event->GetSocket();
    if (sock) {
        struct kevent ev;
        EV_SET(&ev, sock->GetSocket(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)event);
        _change_list.push_back(ev);
        return true;
    }
    LOG_WARN("socket is already distroyed! event %s", "AddRecvEvent");
    return false;
}

bool KqueueEventActions::AddAcceptEvent(Event* event) {
    if (event->GetType() & ET_ACCEPT) {
        return false;
    }
    event->AddType(ET_ACCEPT);

    auto sock = event->GetSocket();
    if (sock) {
        struct kevent ev;
        EV_SET(&ev, sock->GetSocket(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)event);
    
        _change_list.push_back(ev);
        return true;
    }
    LOG_WARN("socket is already distroyed! event %s", "AddAcceptEvent");
    return false;
}

bool KqueueEventActions::AddConnection(Event* event, Address& address) {
    if (event->GetType() & ET_CONNECT) {
        return false;
    }
    event->AddType(ET_CONNECT);
    
    auto sock = event->GetSocket();
    if (sock) {
        SocketNoblocking(sock->GetSocket());

        auto ret = OsHandle::Connect(sock->GetSocket(), address);

        auto rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
        if (ret._return_value == 0) {
            rw_sock->OnConnect(event, CEC_SUCCESS);
            return true;
    
        } else if (ret._errno == EINPROGRESS) {
            if (CheckConnect(rw_sock->GetSocket())) {
                rw_sock->OnConnect(event, CEC_SUCCESS);
                return true;
            }
        }
        rw_sock->OnConnect(event, CEC_CONNECT_REFUSE);
        LOG_ERROR("connect to peer failed! errno:%d, info:%s", ret._errno, ErrnoInfo(ret._errno));
        return false;
        
    }

    LOG_WARN("socket is already destroyed!, event:%s", "AddConnection");
    return false;
}

bool KqueueEventActions::AddDisconnection(Event* event) {
    if (event->GetType() & ET_DISCONNECT) {
        return false;
    }
    event->AddType(ET_DISCONNECT);

    auto sock = event->GetSocket();
    if (!sock) {
        return false;
    }
    
    std::shared_ptr<RWSocket> socket = std::dynamic_pointer_cast<RWSocket>(sock);
    OsHandle::Close(socket->GetSocket());
    socket->OnDisConnect(event, CEC_SUCCESS);
    return true;
}

bool KqueueEventActions::DelEvent(Event* event) {
    auto sock = event->GetSocket();
    if (!sock) {
        return false;
    }
    
    struct kevent read_ev;
    EV_SET(&read_ev, sock->GetSocket(), EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
    _change_list.push_back(read_ev);

    struct kevent write_ev;
    EV_SET(&write_ev, sock->GetSocket(), EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
    _change_list.push_back(write_ev);

    event->ClearType();
    return true;
}

void KqueueEventActions::ProcessEvent(int32_t wait_ms) {
    int16_t ret = 0;
    if (wait_ms > 0) {
        _kqueue_timeout.tv_sec = (uint64_t)wait_ms / 1000;
        _kqueue_timeout.tv_nsec = ((uint64_t)wait_ms - (_kqueue_timeout.tv_sec * 1000)) * 1000000;

        ret = kevent(_kqueue_handler, &*_change_list.begin(), (int)_change_list.size(), &*_active_list.begin(), (int)_active_list.size(), &_kqueue_timeout);
    } else {
        ret = kevent(_kqueue_handler, &*_change_list.begin(), (int)_change_list.size(), &*_active_list.begin(), (int)_active_list.size(), nullptr);
    }

    _change_list.clear();
    if (ret < 0) {
        LOG_ERROR("kevent faild! error:%d, info:%s", errno, ErrnoInfo(errno));

    } else {
        LOG_DEBUG("kevent get events! num:%d, TheadId:%lld", ret, std::this_thread::get_id());

        OnEvent(_active_list, ret);
    }
}

void KqueueEventActions::Wakeup() {
    write(_pipe[1], "1", 1);
}

void KqueueEventActions::OnEvent(std::vector<struct kevent>& event_vec, int16_t num) {
    std::shared_ptr<Socket> sock;
    Event* event;

    for (int i = 0; i < num; i++) {
        if (event_vec[i].ident == _pipe[0]) {
            LOG_INFO("weak up the io thread, index : %d", i);
            char buf[4];
            read(_pipe[0], buf, 1);
            continue;
        }

        if (event_vec[i].flags & EV_DELETE) {
            continue;
        }

        event = (Event*)event_vec[i].udata;
        sock = event->GetSocket();
        if (!sock) {
            LOG_WARN("kqueue weak up but socket already destroy, index : %d", i);
            continue;
        }

        // accept event
        if (event->GetType() & ET_ACCEPT) {
            std::shared_ptr<ConnectSocket> connect_sock = std::dynamic_pointer_cast<ConnectSocket>(sock);
            connect_sock->OnAccept(event);

        } else {
            // write event
            if (event_vec[i].flags & EV_CLEAR) {
                std::shared_ptr<RWSocket> rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
                event->RemoveType(ET_WRITE);
                rw_sock->OnWrite(event, event_vec[i].data);
            // read event
            } else {
                std::shared_ptr<RWSocket> rw_sock = std::dynamic_pointer_cast<RWSocket>(sock);
                rw_sock->OnRead(event, event_vec[i].data);
            }
        }   
    }
}

}