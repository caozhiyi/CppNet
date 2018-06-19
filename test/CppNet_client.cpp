#include <thread>
#include <iostream>
#include "IOCP.h"
#include "Socket.h"
#include "Buffer.h"
#include "AcceptSocket.h"
#include "EventHandler.h"
#include "MemaryPool.h"

std::mutex __mutex;

std::map<unsigned int, CMemSharePtr<CSocket>> client_map;

void ReadFunc(CMemSharePtr<CEventHandler>& event, int error);
std::function<void(CMemSharePtr<CEventHandler>& event, int error)> read_back = ReadFunc;

void WriteFunc(CMemSharePtr<CEventHandler>& event, int error) {
	std::cout << "WriteFunc" << std::endl;
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "write count: " << event->_off_set << std::endl << std::endl;
	event->_client_socket.Lock()->SyncRead(read_back);
}
std::function<void(CMemSharePtr<CEventHandler>& event, int error)> write_back = WriteFunc;

void ReadFunc(CMemSharePtr<CEventHandler>& event, int error) {
	std::cout << "ReadFunc" << std::endl;
	std::cout << *(event->_buffer) << std::endl;
	event->_buffer->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << event->_off_set << std::endl << std::endl;
	
	event->_buffer->Clear();
	if (error != EVENT_ERROR_CLOSED || error == EVENT_CONNECT) {
		//event->_client_socket.Lock()->SyncRead(read_back);
		event->_client_socket.Lock()->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"), write_back);
		event->_client_socket.Lock()->SyncDisconnection(read_back);
	} else {
		if (client_map.size() < 10) {
			int a = 0;
			a++;
		}
		std::unique_lock<std::mutex> lock(__mutex);
		client_map.erase(event->_client_socket.Lock()->GetSocket());
	}
}

void AcceptFunc(CMemSharePtr<CAcceptEventHandler>& event, int error) {
	client_map[event->_client_socket->GetSocket()] = event->_client_socket;
	std::cout << "AcceptFunc" << std::endl;
	std::cout << "client address :" << event->_client_socket->GetAddress() << std::endl << std::endl;
	std::unique_lock<std::mutex> lock(__mutex);
	event->_client_socket->SyncRead(read_back);
}
#include "Log.h"
int main() {
	InitScoket();

	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
	CLog::Instance().SetLogName("CppNet.txt");
	CLog::Instance().Start();

	std::shared_ptr<CEventActions> event_actions(new CIOCP);
	event_actions->Init();
	//CAcceptSocket sock(event_actions);

	CMemaryPool pool;
	CMemSharePtr<CSocket> sock =  MakeNewSharedPtr<CSocket>(&pool, event_actions);

	/*void* data = &sock;
	data = (void *)((uintptr_t)data | 1);
	data = (void*)((uintptr_t)data & (uintptr_t)~1);
	CMemSharePtr<CSocket> sock1 = *(CMemSharePtr<CSocket>*)data;*/

	std::vector<std::thread> thread_vec;

	for (int i = 0; i < 1; i++) {
		thread_vec.push_back(std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
	}

	std::function<void(CMemSharePtr<CAcceptEventHandler>& event, int error)> accept_func = AcceptFunc;
	
	sock->SyncConnection("192.168.182.131", 8500, read_back);
	sock->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"), write_back);

	for (int i = 0; i < 1; i++) {
		thread_vec[i].join();
	}
	DeallocSocket();
}

//#include <winsock2.h>
//#include <MSWSock.h>
//#pragma comment(lib,"ws2_32.lib")
//
//int main() {
//	static WSADATA __wsa_data;
//	static bool __has_init = false;
//	if (!__has_init && WSAStartup(MAKEWORD(2, 2), &__wsa_data) != 0) {
//		return false;
//
//	}
//	else {
//		__has_init = true;
//	}
//
//	SOCKADDR_IN addr;
//	addr.sin_family = AF_INET;
//	addr.sin_port = htons(8500);
//	addr.sin_addr.S_un.S_addr = inet_addr("192.168.182.131");
//
//	auto func = [addr](int i) {
//	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
//
//	connect(sock, (sockaddr*)&addr, sizeof(addr));
//	char buf[] = "Hello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello worldHello world";
//	for (;;) {
//	send(sock, buf, strlen(buf), 0);
//
//	char buf2[20] = { 0 };
//	recv(sock, buf2, 20, 0);
//
//	Sleep(1000);
//	}
//	closesocket(sock);
//	};
//
//	/*std::thread thread[1500];
//	for (int i = 0; i < 1500; i++) {
//	Sleep(1);
//	thread[i] = std::thread(func, i);
//	}
//	for (int i = 0; i < 1500; i++) {
//	thread[i].join();
//	}*/
//
//	int a = 0;
//	a++;
//}