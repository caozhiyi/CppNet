#include <thread>
#include <iostream>
#include "IOCP.h"
#include "Socket.h"
#include "Buffer.h"
#include "AcceptSocket.h"
#include "EventHandler.h"

std::mutex __mutex;

std::map<unsigned int, CMemSharePtr<CSocket>> client_map;

void ReadFunc(CMemSharePtr<CEventHandler>& event, int error);
std::function<void(CMemSharePtr<CEventHandler>& event, int error)> read_back = ReadFunc;

void WriteFunc(CMemSharePtr<CEventHandler>& event, int error) {
	//std::cout << "WriteFunc" << std::endl;
	//std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	//std::cout << "write count: " << event->_off_set << std::endl << std::endl;
}
std::function<void(CMemSharePtr<CEventHandler>& event, int error)> write_back = WriteFunc;

void ReadFunc(CMemSharePtr<CEventHandler>& event, int error) {
	/*std::cout << "ReadFunc" << std::endl;
	std::cout << *(event->_buffer) << std::endl;
	event->_buffer->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << event->_off_set << std::endl << std::endl;*/
	event->_buffer->Clear();
	if (error != EVENT_ERROR_CLOSED) {
		event->_client_socket.Lock()->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"), write_back);
		event->_client_socket.Lock()->SyncRead(read_back);
	} else {
		client_map.erase(event->_client_socket.Lock()->GetSocket());
	}
}

void AcceptFunc(CMemSharePtr<CAcceptEventHandler>& event, int error) {
	client_map[event->_client_socket->GetSocket()] = event->_client_socket;
	//std::cout << "AcceptFunc" << std::endl;
	//std::cout << "client address :" << event->_client_socket->GetAddress() << std::endl << std::endl;

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
	CAcceptSocket sock(event_actions);

	std::vector<std::thread> thread_vec;

	for (int i = 0; i < 1; i++) {
		thread_vec.push_back(std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
	}

	std::function<void(CMemSharePtr<CAcceptEventHandler>& event, int error)> accept_func = AcceptFunc;
	sock.Bind(8500, "127.0.0.1");
	sock.Listen(10);
	sock.SyncAccept(accept_func, read_back);

	for (int i = 0; i < 1; i++) {
		thread_vec[i].join();
	}
	DeallocSocket();
}