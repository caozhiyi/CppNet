#ifndef __linux__

#include "Log.h"
#include "IOCP.h"
#include "Buffer.h"
#include "CppNetImpl.h"
#include "EventActions.h"
#include "AcceptSocket.h"
#include "EventHandler.h"
#include "WinExpendFunc.h"

using namespace cppnet;

CAcceptSocket::CAcceptSocket(std::shared_ptr<CEventActions>& event_actions) : CSocketBase(event_actions){
    _accept_event = base::MakeNewSharedPtr<CAcceptEventHandler>(_pool.get());
    _accept_event->_data = _pool->PoolNew<EventOverlapped>();
}

CAcceptSocket::~CAcceptSocket() {
    
}

bool CAcceptSocket::Bind(uint16_t port, const std::string& ip) {
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.S_un.S_addr = ip.empty() ? INADDR_ANY : inet_addr(ip.c_str());

    int ret = bind(_sock, (sockaddr *)&addr, sizeof(sockaddr));

    if (SOCKET_ERROR == ret) {
        base::LOG_FATAL("win32 bind socket filed! errno : %d", GetLastError());
        WSACleanup();
        closesocket(_sock);
        return false;
    }
    _port = port;
    memcpy(_ip, ip.c_str(), ip.length());
    return true;
}

bool CAcceptSocket::Listen() {
    int ret = listen(_sock, SOMAXCONN);
    if (SOCKET_ERROR == ret) {
        base::LOG_FATAL("win32 listen socket filed! errno : %d", GetLastError());
        WSACleanup();
        closesocket(_sock);
        return false;
    }

    return true;
}

void CAcceptSocket::SyncAccept() {
    if (!_accept_event->_accept_socket) {
        _accept_event->_accept_socket = memshared_from_this();
    }

    if (!_accept_event->_client_socket) {
        _accept_event->_client_socket = base::MakeNewSharedPtr<CSocketImpl>(_pool.get(), _event_actions);
    }

    if (_event_actions) {
        _accept_event->_event_flag_set |= EVENT_ACCEPT;
        _event_actions->AddAcceptEvent(_accept_event);
    }
}

void CAcceptSocket::_Accept(base::CMemSharePtr<CAcceptEventHandler>& event) {
    EventOverlapped* context = (EventOverlapped*)event->_data;

    SOCKADDR_IN* client_addr = NULL;
    int remote_len = sizeof(SOCKADDR_IN);
    SOCKADDR_IN* LocalAddr = NULL;
    int localLen = sizeof(SOCKADDR_IN);

    // accept a socket and  read msg
    __AcceptExScokAddrs(context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&client_addr, &remote_len);

    memcpy(event->_client_socket->_ip, inet_ntoa(client_addr->sin_addr), __addr_str_len);
    event->_client_socket->_port = client_addr->sin_port;
    event->_client_socket->_read_event->_buffer->Write(context->_lapped_buffer, event->_client_socket->_read_event->_off_set);
    // get client socket
    event->_client_socket->_read_event->_client_socket = event->_client_socket;

    // call accept call back function
    auto cppnet_ins = GetCppnetInstance();
    if (cppnet_ins) {
        cppnet_ins->_AcceptFunction(event->_client_socket, event->_event_flag_set);
        cppnet_ins->_ReadFunction(event->_client_socket->_read_event, EVENT_READ);
    }

    context->Clear();
    // get a new client socket.windows create a new socket here. 
    event->_client_socket = base::MakeNewSharedPtr<CSocketImpl>(_pool.get(), _event_actions);
    event->_client_socket->SetCppnetInstance(cppnet_ins);
    //post accept again
    SyncAccept();
}
#endif