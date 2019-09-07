#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "EventHandler.h"
#include "Buffer.h"
#include "Log.h"
#include "EventActions.h"
#include "AcceptSocket.h"
#include "Socket.h"
#include "LinuxFunc.h"
#include "CppNetImpl.h"

using namespace cppnet;

CAcceptSocket::CAcceptSocket(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions){
    _sock = socket(PF_INET, SOCK_STREAM, 0);
    SetReusePort(_sock);

    _accept_event = base::MakeNewSharedPtr<CAcceptEventHandler>(_pool.get());
}

CAcceptSocket::~CAcceptSocket() {
    
}

bool CAcceptSocket::Bind(uint16_t port, const std::string& ip) {
    SetSocketNoblocking(_sock);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());

    int ret = bind(_sock, (sockaddr *)&addr, sizeof(sockaddr));

    if (-1 == ret) {
        base::LOG_FATAL("linux bind socket filed! error code:%d", errno);
        close(_sock);
        return false;
    }
    _port = port;
    memcpy(_ip, ip.c_str(), ip.length());
    return true;
}

bool CAcceptSocket::Listen() {
    int ret = listen(_sock, SOMAXCONN);
    if (-1 == ret) {
        base::LOG_FATAL("linux listen socket filed! error code:%d", errno);
        close(_sock);
        return false;
    }

    return true;
}

void CAcceptSocket::SyncAccept() {
    if (!_accept_event->_accept_socket) {
        _accept_event->_accept_socket = memshared_from_this();
    }

    if (!_accept_event->_data) {
        _accept_event->_data = _pool->PoolNew<epoll_event>();
        ((epoll_event*)_accept_event->_data)->events = 0;
    }

    //add event to epoll
    if (_event_actions) {
        _accept_event->_event_flag_set |= EVENT_ACCEPT;
        _event_actions->AddAcceptEvent(_accept_event);
    }
}

void CAcceptSocket::_Accept(base::CMemSharePtr<CAcceptEventHandler>& event) {
    sockaddr_in client_addr;
    socklen_t addr_size = 0;

    int sock = 0;
    for (;;) {
        //may get more than one connections
        sock = accept(event->_accept_socket->GetSocket(), (sockaddr*)&client_addr, &addr_size);
        if (sock <= 0) {
            if (errno == EAGAIN) {
                break;
            }
            base::LOG_FATAL("accept socket filed! error code:%d", errno);
            break;
        }
        //set the socket noblocking
        SetSocketNoblocking(sock);

         //create a new socket
        auto client_socket = base::MakeNewSharedPtr<CSocketImpl>(_pool.get(), _event_actions);
    
        client_socket->_sock = sock;
        
        sockaddr_in sock_addr;
        socklen_t len = sizeof(sock_addr);
    
        getpeername(sock, (struct sockaddr*)&sock_addr, &len);

        memcpy(client_socket->_ip, inet_ntoa(sock_addr.sin_addr), __addr_str_len);
        client_socket->_port = ntohs(sock_addr.sin_port);

        //call accept call back function
        CCppNetImpl::Instance()._AcceptFunction(client_socket, EVENT_ACCEPT);
        //start read
        client_socket->SyncRead();
    }
}
#endif // __linux__
