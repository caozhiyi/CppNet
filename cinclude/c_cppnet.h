// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CINCLUDE_CPPNET
#define CINCLUDE_CPPNET

extern "C" {

/* define types */
/*****************************************************/
// socket
_declspec(dllexport) typedef void* NetHandle;
_declspec(dllexport) typedef void* BufferHandle;
_declspec(dllexport) typedef void* SocketHandle;

enum CPPNET_ERROR_CODE {
    CEC_SUCCESS        = 0,    // success.
    CEC_CLOSED         = 1,    // remote close the socket.
    CEC_CONNECT_BREAK  = 2,    // connection break.
    CEC_CONNECT_REFUSE = 3,    // remote refuse connect or server not exist.
};

/*****************************************************/

/*net handle function*/
/*****************************************************/
_declspec(dllexport) NetHandle Init(int thread_num = 0);
_declspec(dllexport) void Destory(NetHandle net);

// thread join
_declspec(dllexport) void Join(NetHandle net);

// must set callback before listen
_declspec(dllexport) void SetReadCallback(NetHandle net, void cb(SocketHandle, BufferHandle, int));
_declspec(dllexport) void SetWriteCallback(NetHandle net, void cb(SocketHandle, int));
_declspec(dllexport) void SetDisconnectionCallback(NetHandle net, void cb(SocketHandle, int));

// if use socket timer, set it
_declspec(dllexport) void SetTimerCallback(NetHandle net, void cb(SocketHandle));

// return timer id
_declspec(dllexport) long AddTimer(NetHandle net, int interval, void cb(void*), void* param = nullptr, bool always = false);
_declspec(dllexport) void RemoveTimer(NetHandle net, long timer_id);

//server
_declspec(dllexport) void SetAcceptCallback(NetHandle net, void cb(SocketHandle, int));
_declspec(dllexport) bool ListenAndAccept(NetHandle net, const char* ip, int port);

//client
_declspec(dllexport) void SetConnectionCallback(NetHandle net, void cb(SocketHandle, int));
_declspec(dllexport) bool Connection(NetHandle net, const char* ip, int port);
/*****************************************************/

/*socket handle function*/
/*****************************************************/
// get os native socket
_declspec(dllexport) long GetSocket(SocketHandle sock);
// get local listen port
_declspec(dllexport) int GetListenPort(SocketHandle sock);
// get socket IP and address
_declspec(dllexport) bool GetAddress(SocketHandle sock, char* ip, int ip_len, int* port);

// post sync write event.
_declspec(dllexport) bool Write(SocketHandle sock, const char* src, int len);
// close the connect
_declspec(dllexport) void Close(SocketHandle sock);

// add a timer. must set timer call back
// interval support max 1 minute
_declspec(dllexport) void SockAddTimer(SocketHandle sock, int interval, bool always = false);
// stop the timer
_declspec(dllexport) void StopTimer(SocketHandle sock);

// set cppnet socket context.
_declspec(dllexport) void SetContext(SocketHandle sock, void* context);
// get context.
_declspec(dllexport) void* GetContext(SocketHandle sock);
/*****************************************************/

/*socket handle function*/
/*****************************************************/
// read to data to buf but don't move the read point.
// return read size.
_declspec(dllexport) int ReadNotMovePt(BufferHandle buffer, char* buf, int buf_len);
// read data to res buf and move the read point.
// return read size.
_declspec(dllexport) int Read(BufferHandle buffer, char* res, int len);

// clear all data
_declspec(dllexport) void Clear(BufferHandle buffer);

// move read point
_declspec(dllexport) int MoveReadPt(BufferHandle buffer, int len);

// do not read when can't find specified character.
// return read bytes when read otherwise return 0
// when find specified character but res length is too short, 
// return 0 and the last param return need length
_declspec(dllexport) int ReadUntil(BufferHandle buffer, char* res, int len, const char* find, int find_len, int* need_len);

// return size of data that can be read
_declspec(dllexport) int GetCanReadLength(BufferHandle buffer);

// return can read bytes
_declspec(dllexport) int FindStr(BufferHandle buffer, const char* s, int s_len);
/*****************************************************/

}
#endif