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

void WriteFunc(CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "WriteFunc" << std::endl;
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "write count: " << sock->_write_event->_off_set << std::endl << std::endl;
	if (error != EVENT_ERROR_CLOSED) {
		sock->SyncRead();
	}
}

void ReadFunc(CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "ReadFunc" << std::endl;
	std::cout << *(sock->_read_event->_buffer) << std::endl;
	sock->_read_event->_buffer->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << sock->_read_event->_off_set << std::endl << std::endl;
	
	if (error != EVENT_ERROR_CLOSED) {
		sock->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"));
	}
}

void AcceptFunc(CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "AcceptFunc" << std::endl;
	std::cout << "client address :" << sock->GetAddress() << std::endl << std::endl;
	sock->SyncRead();
}

#include "Log.h"
#include "NetObject.h"
int main() {
	CLog::Instance().SetLogLevel(LOG_WARN_LEVEL);
	CLog::Instance().SetLogName("CppNet.txt");
	CLog::Instance().Start();
	
	CNetObject net;
	net.Init(1);

	net.SetAcceptCallback(AcceptFunc);
	net.SetWriteCallback(WriteFunc);
	net.SetReadCallback(ReadFunc);

	net.ListenAndAccept(8921, "127.0.0.1");

	//net.MainLoop();
	//net.Dealloc();
	net.Join();
	CLog::Instance().Stop();
	CLog::Instance().Join();
}