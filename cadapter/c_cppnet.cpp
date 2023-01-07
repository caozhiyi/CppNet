// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include <string>
#include <cstdint>
#include "include/cppnet.h"
#include "cinclude/c_cppnet.h"

extern "C" _declspec(dllexport) NetHandle Init(int thread_num) {
	cppnet::CppNet* net = new cppnet::CppNet();
	net->Init(thread_num);
	return net;
}

extern "C" _declspec(dllexport) void Destory(NetHandle net) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->Destory();
	delete handle;
}

extern "C" _declspec(dllexport) void Join(NetHandle net) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->Join();
}

extern "C" _declspec(dllexport) void SetReadCallback(NetHandle net, void cb(SocketHandle, BufferHandle, int)) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->SetReadCallback([cb](cppnet::Handle sock, cppnet::BufferPtr buffer, int err) {
		cb(sock.get(), buffer.get(), err);
	});
}

extern "C" _declspec(dllexport) void SetWriteCallback(NetHandle net, void cb(SocketHandle, int)) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->SetWriteCallback([cb](cppnet::Handle sock, int err) {
		cb(sock.get(), err);
	});
}

extern "C" _declspec(dllexport) void SetDisconnectionCallback(NetHandle net, void cb(SocketHandle, int)) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->SetDisconnectionCallback([cb](cppnet::Handle sock, int err) {
		cb(sock.get(), err);
	});
}

extern "C" _declspec(dllexport) void SetTimerCallback(NetHandle net, void cb(SocketHandle)) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->SetTimerCallback([cb](cppnet::Handle sock) {
		cb(sock.get());
	});
}

extern "C" _declspec(dllexport) long AddTimer(NetHandle net, int interval, void cb(void*), void* param, bool always) {
	if (net == nullptr) {
		return 0;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	auto timer_cb = [cb](void* pm) {
		cb(pm);
	};
	return handle->AddTimer(interval, timer_cb, param, always);
}

extern "C" _declspec(dllexport) void RemoveTimer(NetHandle net, long timer_id) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->RemoveTimer(timer_id);
}

extern "C" _declspec(dllexport) void SetAcceptCallback(NetHandle net, void cb(SocketHandle, int)) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->SetAcceptCallback([cb](cppnet::Handle sock, int err) {
		cb(sock.get(), err);
	});
}

extern "C" _declspec(dllexport) bool ListenAndAccept(NetHandle net, const char* ip, int port) {
	if (net == nullptr) {
		return false;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	return handle->ListenAndAccept(ip, port);
}

extern "C" _declspec(dllexport) void SetConnectionCallback(NetHandle net, void cb(SocketHandle, int)) {
	if (net == nullptr) {
		return;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	handle->SetConnectionCallback([cb](cppnet::Handle sock, int err) {
		cb(sock.get(), err);
	});
}

extern "C" _declspec(dllexport) bool Connection(NetHandle net, const char* ip, int port) {
	if (net == nullptr) {
		return false;
	}
	cppnet::CppNet* handle = (cppnet::CppNet*)net;
	return handle->Connection(ip, port);
}

extern "C" _declspec(dllexport) long GetSocket(SocketHandle sock) {
	if (sock == nullptr) {
		return 0;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	return handle->GetSocket();
}

extern "C" _declspec(dllexport) int GetListenPort(SocketHandle sock) {
	if (sock == nullptr) {
		return 0;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	return handle->GetListenPort();
}

// get socket IP and address
extern "C" _declspec(dllexport) bool GetAddress(SocketHandle sock, char* ip, int ip_len, int* port) {
	if (sock == nullptr || ip == nullptr) {
		return false;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	std::string temp_ip;
	uint16_t temp_port;
	handle->GetAddress(temp_ip, temp_port);
	memcpy(ip, temp_ip.data(), temp_ip.length() > ip_len ? ip_len : temp_ip.length());
	*port = temp_port;
	return true;
}

extern "C" _declspec(dllexport) bool Write(SocketHandle sock, const char* src, int len) {
	if (sock == nullptr) {
		return 0;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	return handle->Write(src, len);
}

extern "C" _declspec(dllexport) void Close(SocketHandle sock) {
	if (sock == nullptr) {
		return;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	handle->Close();
}

extern "C" _declspec(dllexport) void SockAddTimer(SocketHandle sock, int interval, bool always) {
	if (sock == nullptr) {
		return;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	handle->AddTimer(interval, always);
}

extern "C" _declspec(dllexport) void StopTimer(SocketHandle sock) {
	if (sock == nullptr) {
		return;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	handle->StopTimer();
}

extern "C" _declspec(dllexport) void SetContext(SocketHandle sock, void* context) {
	if (sock == nullptr) {
		return;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	handle->SetContext(context);
}

extern "C" _declspec(dllexport) void* GetContext(SocketHandle sock) {
	if (sock == nullptr) {
		return nullptr;
	}
	cppnet::CNSocket* handle = (cppnet::CNSocket*)sock;
	return handle->GetContext();
}

extern "C" _declspec(dllexport) int ReadNotMovePt(BufferHandle buffer, char* buf, int buf_len) {
	if (buffer == nullptr) {
		return 0;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	return handle->ReadNotMovePt(buf, buf_len);
}

extern "C" _declspec(dllexport) int Read(BufferHandle buffer, char* res, int len) {
	if (buffer == nullptr) {
		return 0;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	return handle->Read(res, len);
}

extern "C" _declspec(dllexport) void Clear(BufferHandle buffer) {
	if (buffer == nullptr) {
		return;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	return handle->Clear();
}

extern "C" _declspec(dllexport) int MoveReadPt(BufferHandle buffer, int len) {
	if (buffer == nullptr) {
		return 0;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	return handle->MoveReadPt(len);
}

extern "C" _declspec(dllexport) int ReadUntil(BufferHandle buffer, char* res, int len, const char* find, int find_len, int* need_len) {
	if (buffer == nullptr) {
		return 0;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	uint32_t need;
	uint32_t ret = handle->ReadUntil(res, len, find, find_len, need);
	*need_len = need;
	return ret;
}

extern "C" _declspec(dllexport) int GetCanReadLength(BufferHandle buffer) {
	if (buffer == nullptr) {
		return 0;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	return handle->GetCanReadLength();
}

extern "C" _declspec(dllexport) int FindStr(BufferHandle buffer, const char* s, int s_len) {
	if (buffer == nullptr) {
		return 0;
	}
	cppnet::Buffer* handle = (cppnet::Buffer*)buffer;
	return handle->FindStr(s, s_len);
	
}