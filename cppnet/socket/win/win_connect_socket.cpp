#include "win_connect_socket.h"

#include "cppnet/cppnet_base.h"
#include "cppnet/socket/rw_socket.h"
#include "cppnet/event/win/expend_func.h"
#include "cppnet/event/win/accept_event.h"
#include "cppnet/event/action_interface.h"

#include "common/log/log.h"
#include "common/os/convert.h"
#include "common/network/socket.h"
#include "common/network/io_handle.h"
#include "common/buffer/buffer_queue.h"
#include "common/alloter/pool_alloter.h"

#ifdef SetPort
#undef SetPort
#endif

namespace cppnet {

std::shared_ptr<ConnectSocket> MakeConnectSocket() {
    return std::make_shared<WinConnectSocket>();
}

WinConnectSocket::WinConnectSocket():
	_in_actions(false) {
	for (uint16_t i = 0; i < __iocp_accept_event_num; i++) {
		auto event = std::make_shared<AcceptEvent>();
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
    auto& event = _accept_event_vec[index];
    auto accept_event = std::dynamic_pointer_cast<AcceptEvent>(event);
	// create a new socket
	auto sock_ret = OsHandle::TcpSocket();
	if (sock_ret._return_value < 0) {
		LOG_ERROR("create socket failed. errno:%d, info:%s", sock_ret._errno, ErrnoInfo(sock_ret._errno));
		return;
	}
    accept_event->SetClientSocket(sock_ret._return_value);
	if (!accept_event->GetSocket()) {
		accept_event->SetSocket(shared_from_this());
	}

	auto actions = GetEventActions();
	if (actions) {
		actions->AddAcceptEvent(event);
	}
}

void WinConnectSocket::OnAccept(std::shared_ptr<AcceptEvent> event) {
	auto cppnet_base = _cppnet_base.lock();
	if (!cppnet_base) {
		return;
	}

	SOCKADDR_IN* client_addr = NULL;
	int remote_len = sizeof(SOCKADDR_IN);
	SOCKADDR_IN* LocalAddr = NULL;
	int localLen = sizeof(SOCKADDR_IN);

	// accept a socket and read msg
	AcceptExScokAddrs(event->GetBuf(), __iocp_buff_size - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPSOCKADDR*)&LocalAddr, &localLen, (LPSOCKADDR*)&client_addr, &remote_len);

	// create a new rw socket
	std::shared_ptr<AlloterWrap> alloter = std::make_shared<AlloterWrap>(MakePoolAlloterPtr());
	Address addr(AT_IPV4);

	addr.SetIp(inet_ntoa(client_addr->sin_addr));
	addr.SetPort(client_addr->sin_port);

	auto sock = MakeRWSocket(event->GetClientSocket(), std::move(alloter));

	sock->SetCppNetBase(cppnet_base);
	sock->SetEventActions(_event_actions);
	sock->SetAddress(std::move(addr));
	sock->SetDispatcher(GetDispatcher());

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