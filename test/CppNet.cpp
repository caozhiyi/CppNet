#include <thread>
#include <iostream>
#ifdef __linux__
#include "CEpoll.h"
#else
#include "IOCP.h"
#endif // __linux__

#include "Socket.h"
#include "Buffer.h"
#include "AcceptSocket.h"
#include "EventHandler.h"



std::mutex __mutex;

std::map<unsigned int, CMemSharePtr<CSocket>> client_map;

void ReadFunc(CMemSharePtr<CEventHandler>& event, int error);
std::function<void(CMemSharePtr<CEventHandler>& event, int error)> read_back = ReadFunc;

void WriteFunc(CMemSharePtr<CEventHandler>& event, int error) {
	std::cout << "WriteFunc" << std::endl;
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "write count: " << event->_off_set << std::endl << std::endl;
	if (error != EVENT_ERROR_CLOSED) {
		event->_client_socket.Lock()->SyncRead(read_back);
	} else {
		std::unique_lock<std::mutex> lock(__mutex);
		client_map.erase(event->_client_socket.Lock()->GetSocket());
	}
}
std::function<void(CMemSharePtr<CEventHandler>& event, int error)> write_back = WriteFunc;

void ReadFunc(CMemSharePtr<CEventHandler>& event, int error) {
	std::cout << "ReadFunc" << std::endl;
	std::cout << *(event->_buffer) << std::endl;
	event->_buffer->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << event->_off_set << std::endl << std::endl;
	
	event->_buffer->Clear();
	if (error != EVENT_ERROR_CLOSED) {
		event->_client_socket.Lock()->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"), write_back);
	} else {
		std::unique_lock<std::mutex> lock(__mutex);
		client_map.erase(event->_client_socket.Lock()->GetSocket());
	}
}

void AcceptFunc(CMemSharePtr<CAcceptEventHandler>& event, int error) {
	std::unique_lock<std::mutex> lock(__mutex);
	client_map[event->_client_socket->GetSocket()] = event->_client_socket;
	std::cout << "AcceptFunc" << std::endl;
	std::cout << "client address :" << event->_client_socket->GetAddress() << std::endl << std::endl;
	event->_client_socket->SyncRead(read_back);
}
#include "Log.h"
//#include "LinuxFunc.h"
int main() {
#ifndef __linux__
	InitScoket();
#endif
	

	CLog::Instance().SetLogLevel(LOG_DEBUG_LEVEL);
	CLog::Instance().SetLogName("CppNet.txt");
	CLog::Instance().Start();

#ifdef __linux__
	std::shared_ptr<CEventActions> event_actions(new CEpoll);
#else
	std::shared_ptr<CEventActions> event_actions(new CIOCP);
#endif // __linux__

	
	event_actions->Init();
	CAcceptSocket sock(event_actions);

	CMemaryPool pool;
	auto _accept_event = MakeNewSharedPtr<CAcceptEventHandler>(&pool);

	std::vector<std::thread> thread_vec;

	for (int i = 0; i < 8; i++) {
		thread_vec.push_back(std::thread(std::bind(&CEventActions::ProcessEvent, event_actions)));
	}

	std::function<void(CMemSharePtr<CAcceptEventHandler>& event, int error)> accept_func = AcceptFunc;
	sock.Bind(8500, "0.0.0.0");
	sock.Listen(10);
	sock.SyncAccept(accept_func, read_back);

	for (int i = 0; i < 8; i++) {
		thread_vec[i].join();
	}
#ifndef __linux__
	DeallocSocket();
#endif
}