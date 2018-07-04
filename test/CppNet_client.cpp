#include <thread>
#include <iostream>
#include "IOCP.h"
#include "Socket.h"
#include "Buffer.h"
#include "AcceptSocket.h"
#include "EventHandler.h"
#include "MemaryPool.h"

void WriteFunc(CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "WriteFunc" << std::endl;
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "write count: " << sock->_write_event->_off_set << std::endl << std::endl;
	sock->SyncRead();
}

void ConnectionFunc(CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "ConnectionFunc" << std::endl;
	sock->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"));
}

void DisConnectionFunc(CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "DisConnectionFunc" << std::endl;
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

#include "Log.h"
#include "NetObject.h"

int main() {
	CLog::Instance().SetLogLevel(LOG_WARN_LEVEL);
	CLog::Instance().SetLogName("CppNet.txt");
	CLog::Instance().Start();

	CNetObject net;
	net.Init(2);

	net.SetConnectionCallback(ConnectionFunc);
	net.SetWriteCallback(WriteFunc);
	net.SetReadCallback(ReadFunc);
	net.SetDisconnectionCallback(DisConnectionFunc);

	auto sock = net.Connection(8921, "127.0.0.1");
	CRunnable::Sleep(2000);
	
	sock->SyncDisconnection();
	//net.MainLoop();
	//net.Dealloc();
	net.Join();
	CLog::Instance().Stop();
	CLog::Instance().Join();
}