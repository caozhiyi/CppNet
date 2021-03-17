#ifdef __APPLE__

#include <thread>
#include <errno.h>     // for errno
#include <unistd.h>    // for close
#include <arpa/inet.h>
#include <sys/socket.h>

#include "kqueue_action.h"
#include "cppnet/address.h"
#include "common/log/log.h"
#include "common/os/socket.h"
#include "include/cppnet_type.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/socket/connect_socket.h"
#include "common/timer/timer_interface.h"
#include "cppnet/socket/event_interface.h"
#include "common/util/time.h"

namespace cppnet {

KqueueEventActions::KqueueEventActions():
    _kqueue_handler(-1) {

}

KqueueEventActions::~KqueueEventActions() {
    if (_kqueue_handler > 0) {
        close(_kqueue_handler);
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
}

bool KqueueEventActions::Dealloc() {
    Wakeup();
    return true;
}

bool KqueueEventActions::AddSendEvent(std::shared_ptr<Event>& event) {
    if (event->GetSettedFlag() & ESF_WRITE) {
        return false;
    }
    event->AddSettedFlag(ESF_WRITE);

    void* udata = (void*)&event;
    udata = (void*)(((uintptr_t)udata) | 1);

    std::shared_ptr<Socket> socket = std::dynamic_pointer_cast<Socket>(event);
    struct kevent ev;
    EV_SET(&ev, socket->GetSocket(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);

    _change_list.push_back(ev);
    return true;
}

bool KqueueEventActions::AddRecvEvent(std::shared_ptr<Event>& event) {
    if (event->GetSettedFlag() & ESF_WRITE) {
        return false;
    }
    event->AddSettedFlag(ESF_WRITE);

    std::shared_ptr<Socket> socket = std::dynamic_pointer_cast<Socket>(event);
    struct kevent ev;
    EV_SET(&ev, socket->GetSocket(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)&event);

    _change_list.push_back(ev);
    return true;
}

bool KqueueEventActions::AddAcceptEvent(std::shared_ptr<Event>& event) {
    if (event->GetSettedFlag() & ESF_READ) {
        return false;
    }
    event->AddSettedFlag(ESF_READ);

    std::shared_ptr<Socket> socket = std::dynamic_pointer_cast<Socket>(event);
    struct kevent ev;
    EV_SET(&ev, socket->GetSocket(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)&event);
    
    _change_list.push_back(ev);
    _listener_map.insert(socket->GetSocket());
    return true;
}

bool KqueueEventActions::AddConnection(std::shared_ptr<Event>& event, Address& address) {
    if (event->GetSettedFlag() & ESF_WRITE) {
        return true;
    }
    event->AddSettedFlag(ESF_WRITE);

    std::shared_ptr<Socket> socket = std::dynamic_pointer_cast<Socket>(event);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(address.GetPort());
    addr.sin_addr.s_addr = inet_addr(address.GetIp().c_str());

    //block here in linux
    SocketNoblocking(socket->GetSocket());

    int32_t res = connect(socket->GetSocket(), (sockaddr *)&addr, sizeof(addr));

    if (res == 0) {
        socket->OnConnect(0);
        return true;

    } else if (errno == EINPROGRESS) {
        /*if (CheckConnect(socket_ptr->GetSocket())) {
            socket_ptr->Recv(socket_ptr->_read_event);
            return true;
        }*/
    }
    socket->OnConnect(CEC_CONNECT_REFUSE);
    LOG_WARN("connect remote failed. err:%d, addr:%s", errno, address.AsString().c_str());
    return false;
}

bool KqueueEventActions::AddDisconnection(std::shared_ptr<Event>& event) {
    std::shared_ptr<Socket> socket = std::dynamic_pointer_cast<Socket>(event);
    close(socket->GetSocket());
    socket->OnDisConnect(CEC_SUCCESS);
}

bool KqueueEventActions::DelEvent(const uint64_t sock) {
    struct kevent read_ev;
    EV_SET(&read_ev, sock, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    _change_list.push_back(read_ev);

    struct kevent write_ev;
    EV_SET(&write_ev, sock, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    _change_list.push_back(write_ev);

    return true;
}

bool KqueueEventActions::DelEvent(std::shared_ptr<Event>& event) {
    std::shared_ptr<Socket> socket = std::dynamic_pointer_cast<Socket>(event);
    return DelEvent(socket->GetSocket());
}

void KqueueEventActions::ProcessEvent() {
    _run = true;

    int32_t wait_time = 0;
    std::vector<struct kevent> event_vec;
    event_vec.resize(1000);
    _cur_utc_time = UTCTimeMsec();

    struct timespec timeout;

    while (_run) {
        int16_t ret = 0;
        wait_time = _timer->MinTime();
        if (wait_time > 0) {
            timeout.tv_nsec = wait_time * 1000000;

            ret = kevent(_kqueue_handler, &*_change_list.begin(), (int)_change_list.size(), &*event_vec.begin(), (int)event_vec.size(), &timeout);
        } else {
            ret = kevent(_kqueue_handler, &*_change_list.begin(), (int)_change_list.size(), &*event_vec.begin(), (int)event_vec.size(), nullptr);
        }

        if (ret == -1) {
            LOG_ERROR("kevent faild! error :%d", errno);
        }

        if (ret > 0) {
            LOG_DEBUG("kevent get events! num :%d, TheadId : %lld", ret, std::this_thread::get_id());

            OnEvent(event_vec, ret);
        }

        OnTask();

        uint64_t cur_time = UTCTimeMsec();
        _timer->TimerRun(cur_time - _cur_utc_time);
        _cur_utc_time = cur_time;
    }

    LOG_INFO("return the net io thread");
}

void KqueueEventActions::PostTask(Task& task) {
    {
        //std::unique_lock<std::mutex> lock(_mutex);
        _task_list.push_back(task);
    }
    Wakeup();
}

void KqueueEventActions::Wakeup() {
    write(_pipe[1], "1", 1);
}

void KqueueEventActions::OnEvent(std::vector<struct kevent>& event_vec, int16_t num) {
    std::shared_ptr<Socket> socket;
    bool is_send = false;
    for (int i = 0; i < num; i++) {
        if (event_vec[i].ident == _pipe[0]) {
            LOG_WARN("weak up the io thread, index : %d", i);
            char buf[4];
            read(_pipe[0], buf, 1);
            continue;
        }

        if (!event_vec[i].udata) {
            LOG_ERROR("the event is nullptr, index : %d", i);
            continue;
        }

        is_send = ((uintptr_t)event_vec[i].udata) & 1;

        if (is_send) {
            void* ptr = (void*)(((uintptr_t)event_vec[i].udata) & (uintptr_t)~1);
            socket = std::dynamic_pointer_cast<Socket>(*(std::shared_ptr<Event>*)ptr);
            socket->OnWrite(event_vec[i].data);

        } else {
            socket = std::dynamic_pointer_cast<Socket>(*(std::shared_ptr<Event>*)event_vec[i].udata);
            auto iter = _listener_map.find(socket->GetSocket());
            if (iter != _listener_map.end()) {
                socket->OnAccept();

            } else {
                socket->OnRead(event_vec[i].data);
            }
        }
    }
}

void KqueueEventActions::OnTask() {
    std::vector<Task> func_vec;
    {
        //std::unique_lock<std::mutex> lock(_mutex);
        func_vec.swap(_task_list);
    }

    for (size_t i = 0; i < func_vec.size(); ++i) {
        func_vec[i]();
    }
}

}

#endif