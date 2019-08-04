#include <iostream>
#include "CppNet.h"
using namespace cppnet;

void WriteFunc(base::CMemSharePtr<cppnet::CSocket>& sock, int error) {
	//std::cout << "WriteFunc" << std::endl;
	//std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
	//std::cout << "write count: " << sock->_write_event->_off_set << std::endl << std::endl;
	////if (error != EVENT_ERROR_CLOSED) {
	//	sock->SyncRead();
	////}
}

void ReadFunc(base::CMemSharePtr<cppnet::CSocket>& sock, int error) {
//	std::cout << "ReadFunc" << std::endl;
//	std::cout << *(sock->_read_event->_buffer) << std::endl;
//	sock->_read_event->_buffer->Clear();
//	std::cout << "Thread ID : " << std::this_thread::get_id() << std::endl;
//	std::cout << "Read size : " << sock->_read_event->_off_set << std::endl << std::endl;
//	
//	//if (error != EVENT_ERROR_CLOSED) {
//		sock->SyncWrite("aaaaa21231231", strlen("aaaaa21231231"));
//	//}
}

void AcceptFunc(base::CMemSharePtr<cppnet::CSocket>& sock, int error) {
	std::cout << "AcceptFunc" << std::endl;
	std::cout << "client address :" << sock->GetAddress() << std::endl << std::endl;
	sock->SyncRead();
}

int main() {

	cppnet::Init(1);

    cppnet::SetAcceptCallback(AcceptFunc);
    cppnet::SetWriteCallback(WriteFunc);
    cppnet::SetReadCallback(ReadFunc);

    cppnet::ListenAndAccept(8921, "0.0.0.0");

	//net.MainLoop();
	//net.Dealloc();
    cppnet::Join();
}