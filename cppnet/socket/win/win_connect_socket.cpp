#include <errno.h>
#include <MSWSock.h>
#include <winsock2.h>

#include "posix_connect_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/win/expend_func.h"

#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/network/io_handle.h"
#include "common/alloter/pool_alloter.h"

namespace cppnet {

std::shared_ptr<ConnectSocket> MakeConnectSocket() {
    return std::make_shared<PosixConnectSocket>();
}

PosixConnectSocket::PosixConnectSocket() {

}

PosixConnectSocket::~PosixConnectSocket() {

}

void ConnectSocket::OnAccept() {
    SOCKADDR_IN* client_addr = NULL;
    int remote_len = sizeof(SOCKADDR_IN);
    SOCKADDR_IN* LocalAddr = NULL;
    int localLen = sizeof(SOCKADDR_IN);

    // accept a socket and  read msg
    AcceptExScokAddrs(context->_lapped_buffer, context->_wsa_buf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
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

}