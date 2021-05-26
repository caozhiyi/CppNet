// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <WS2tcpip.h>
#include "win_connect_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/win/expend_func.h"
#include "cppnet/event/win/accept_event.h"
#include "cppnet/event/action_interface.h"
#include "cppnet/event/win/iocp_action.h"

#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/network/io_handle.h"
#include "common/buffer/buffer_queue.h"
#include "common/alloter/pool_alloter.h"

#ifdef SetAddrPort
#undef SetAddrPort
#endif

namespace cppnet {

std::shared_ptr<ConnectSocket> MakeConnectSocket() {
    return std::make_shared<WinConnectSocket>();
}

WinConnectSocket::WinConnectSocket():
	_in_actions(false) {
	for (uint16_t i = 0; i < __iocp_accept_event_num; i++) {
		auto event = new WinAcceptEvent(i);
		_accept_event_vec.emplace_back(event);
	}
}

WinConnectSocket::~WinConnectSocket() {

}

void WinConnectSocket::Accept() {
    __all_socket_map[_sock] = shared_from_this();
	for (uint16_t i = 0; i < __iocp_accept_event_num; i++) {
        Accept(i);
	}
}

void WinConnectSocket::Accept(uint16_t index) {
    auto event = _accept_event_vec[index];
    auto accept_event = dynamic_cast<WinAcceptEvent*>(event);
    // create a new socket
    auto sock_ret = OsHandle::TcpSocket();
    if (sock_ret._return_value < 0) {
        LOG_ERROR("create socket failed. errno:%d, info:%s", sock_ret._errno, ErrnoInfo(sock_ret._errno));
        return;
    }
    setsockopt(sock_ret._return_value, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
        (char *)&_sock, sizeof(_sock));

    accept_event->SetClientSocket(sock_ret._return_value);
    if (!accept_event->GetSocket()) {
        accept_event->SetSocket(shared_from_this());
    }

    auto actions = GetEventActions();
    if (actions) {
        auto iocp = std::dynamic_pointer_cast<IOCPEventActions>(actions);
        iocp->AddWinAcceptEvent(event);
    }
}

void WinConnectSocket::OnAccept(WinAcceptEvent* event) {
	auto cppnet_base = _cppnet_base.lock();
	if (!cppnet_base) {
		return;
	}

	SOCKADDR_STORAGE* client_addr = NULL;
	int remote_len = sizeof(SOCKADDR_STORAGE);
	SOCKADDR_STORAGE* LocalAddr = NULL;
	int localLen = sizeof(SOCKADDR_STORAGE);

	// accept a socket and read msg
	AcceptExSockAddrs(event->GetBuf(), __iocp_buff_size - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
		sizeof(SOCKADDR_STORAGE) + 16, sizeof(SOCKADDR_STORAGE) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&client_addr, &remote_len);

    // Does this call have any effect ?
    setsockopt(event->GetClientSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
        (char *)&_sock, sizeof(_sock));

	// create a new rw socket
	std::shared_ptr<AlloterWrap> alloter = std::make_shared<AlloterWrap>(MakePoolAlloterPtr());
	Address address;

	SOCKADDR* addr_pt = (SOCKADDR*)client_addr;
	void* addr = nullptr;
	switch (addr_pt->sa_family) {
	case AF_INET:
		addr = &((SOCKADDR_IN*)addr_pt)->sin_addr;
		address.SetAddrPort(ntohs(((SOCKADDR_IN*)addr_pt)->sin_port));
		address.SetType(AT_IPV4);
		break;
	case AF_INET6:
		addr = &((SOCKADDR_IN6*)addr_pt)->sin6_addr;
		address.SetAddrPort((((struct sockaddr_in6*)addr_pt)->sin6_port));
		address.SetType(AT_IPV6);
		break;
	default:
		LOG_ERROR("invalid socket address family. family:%d", addr_pt->sa_family);
		return;
	}
	char str_addr[INET6_ADDRSTRLEN] = { 0 };
	inet_ntop(AF_INET6, addr, str_addr, sizeof(str_addr));
	address.SetIp(str_addr);

	auto sock = MakeRWSocket(event->GetClientSocket(), std::move(alloter));

	sock->SetCppNetBase(cppnet_base);
	sock->SetEventActions(_event_actions);
	sock->SetAddress(std::move(address));
	sock->SetDispatcher(GetDispatcher());

    auto action = GetEventActions();
    auto iocp = std::dynamic_pointer_cast<IOCPEventActions>(action);
    iocp->AddToIOCP(event->GetClientSocket());

	__all_socket_map[event->GetClientSocket()] = sock;

	auto buffer = sock->GetReadBuffer();
	buffer->Write(event->GetBuf(), event->GetBufOffset());


	// call accept call back function
	cppnet_base->OnAccept(sock);
	cppnet_base->OnRead(sock, event->GetBufOffset());

	//post accept again
	Accept(event->GetIndex());

	// wait for read
	sock->Read();
}

}