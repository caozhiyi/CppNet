#include <thread>
#include <iostream>
#include "Socket.h"
#include "Buffer.h"
#include "AcceptSocket.h"
#include "EventHandler.h"

#include "Log.h"
#include "NetObject.h"

void WriteFunc(base::CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "WriteFunc" << std::endl;
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "write count: " << sock->_write_event->_off_set << std::endl << std::endl;
	if (error != EVENT_ERROR_CLOSED) {
		sock->SyncRead();
	}
}

void ReadFunc(base::CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "ReadFunc" << std::endl;
	std::cout << *(sock->_read_event->_buffer) << std::endl;
	sock->_read_event->_buffer->Clear();
	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	std::cout << "Read size : " << sock->_read_event->_off_set << std::endl << std::endl;
	
	if (error != EVENT_ERROR_CLOSED) {
		sock->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"));
	}
}

void AcceptFunc(base::CMemSharePtr<CSocket>& sock, int error) {
	std::cout << "AcceptFunc" << std::endl;
	std::cout << "client address :" << sock->GetAddress() << std::endl << std::endl;
	sock->SyncRead();
}

int num = 0;
uint64_t id = 0;
void TestTimer(void* param) {
    std::cout << "timer" << std::endl;
    num++;
    if (num >= 5) {
        ((CNetObject*)param)->RemoveTimer(id);
    }
}

int main() {
    base::CLog::Instance().SetLogLevel(base::LOG_WARN_LEVEL);
    base::CLog::Instance().SetLogName("CppNet.txt");
    base::CLog::Instance().Start();
	
	CNetObject net;
	net.Init(1);

	net.SetAcceptCallback(AcceptFunc);
	net.SetWriteCallback(WriteFunc);
	net.SetReadCallback(ReadFunc);

	net.ListenAndAccept(8921, "0.0.0.0");

    id = net.SetTimer(2000, TestTimer, &net, true);
	//net.MainLoop();
	//net.Dealloc();
	net.Join();
    base::CLog::Instance().Stop();
    base::CLog::Instance().Join();
}