<p align="left"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="left">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

See [chinese](/README_cn.md)    
See the details in chinese [Wiki](https://github.com/caozhiyi/CppNet/wiki)
## Introduction

Cppnet is a proactor mode and multithreaded network with C++11 on tcp. Support Window, Linux and macOS.    
 Simple:    
 + Only export a little interfaces, all net ios insterface are asynchronous callbacks
 + Insterface as much as possible like calling the socket API of the system
 + There is only one additional buffer object type for the client
 + Support both IPv4 and IPv6

 Fast: 
 + Use epoll, IOCP and kqueue
 + Multithreaded threads are handled by the kernel through port reuse
 + Each socket has a single memory pool object. All memory requested from the memory pool is managed by an intelligent pointer 
 + Using time wheel to realize timer   
 
 Clear：
 + Three layers: event-driven layer, session management layer and interface layer
 + Upward notification through callbacks between layers. Clear division of responsibilities among modules, pay to Caesar what belongs to Caesar and God what belongs to God
 + The interface decoupling module is used to meet the minimum interface principle and dependency inversion principle  

## Interface

All the interface files are in [include](/include). The interface definitions for library initialization and timer are in [CppNet](/include/CppNet.h):    
```c++
class CppNet {
public:
    // common
    // init cppnet library.
    // thread_num : the number of running threads.
    void Init(int32_t thread_num);
    void Destory();

    // thread join
    void Join();

    // must set callback before listen
    void SetReadCallback(const read_call_back& cb);
    void SetWriteCallback(const write_call_back& cb);
    void SetDisconnectionCallback(const connect_call_back& cb);

    // return timer id
    uint64_t AddTimer(int32_t interval, const user_timer_call_back& cb, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(const connect_call_back& cb);
    bool ListenAndAccept(const std::string& ip, int16_t port);

    //client
    void SetConnectionCallback(const connect_call_back& cb);
    bool Connection(const std::string& ip, int16_t port);
};
```
Since all network IO interfaces are defined as callback notification modes, callback functions for each call need to be set when initializing the library.     
By setting callbacks instead of providing virtual function inheritance, we hope to be as simple as possible, reduce the inheritance relationship of classes, and increase the flexibility of callbacks. You can set callbacks to any function.         
The interface definition for network IO are in [Socket](/include/Socket.h):      
```c++
class CNSocket {
public:
    // get socket ip and adress
    virtual bool GetAddress(std::string& ip, uint16_t& port);
    // post sync write event.
    virtual bool Write(const char* src, uint32_t len);
    // close the connect
    virtual bool Close();
    // add a timer. must set timer call back
    // interval support max 1 minute
    // return a timer id
    virtual uint64_t AddTimer(uint32_t interval, bool always = false);
    virtual void StopTimer(uint64_t timer_id);
};
```
The function of the interface is evident through declarations and annotations. Attention should be paid to the error code returned by the interface, defined in [CppDefine](/include/CppDefine.h):    
```c++
enum CPPNET_ERROR_CODE {
    CEC_SUCCESS                = 0,    // success.
    CEC_TIMEOUT                = 1,    // the event time out call back.
    CEC_CLOSED                 = 2,    // remote close the socket.
    CEC_INVALID_HANDLE         = 3,    // invalid cppnet handle, can find in socket manager.
    CEC_FAILED                 = 4,    // call function failed.
    CEC_CONNECT_BREAK          = 5,    // connect break.
    CEC_CONNECT_REFUSE         = 6,    // remote refuse connect or server not exist.
};
```
All notifications about the connection status are called back to the connection related functions.     

## Example

All simples are in [test](/test):   
[simple](/test/simple): A most simple example.   
[echo](/test/echo): A test program of echo with 10000 connection.   
[http](/test/http): A simple HTTP server is implemented with reference to muduo.   
[sendfile](/test/sendfile): An example of sending and receiving files.   
[pingpong](/test/pingpong): A pingpong test program.   
[rpc](/test/rpc): A interesting rpc program.   

## Efficiency
Only use apache ab test HTTP echo，comparison with Muduo. The command executed is：ab -kc[1-2000] -n100000 http://127.0.0.1:8000/hello.
<p align="left"><img width="896" src="./doc/image/muduo_vs_cppnet.png" alt="mudo vs cppnet"></p>

## Build(Windows)

You can compile Cppnet library and example with vs2019.   

## Build((Linux & macOS)

The Cppnet library and examples can be compiled simply by executing make in the source directory.     
Other examples need to make in local directories after compiling static libraries.     
```
$ make -j4
```

## Licenses

This program is under the terms of the BSD 3-Clause License. See [https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause).
