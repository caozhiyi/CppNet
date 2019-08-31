<p align="left"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="left">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

## 简介

CppNet是一个封装在Tcp协议上的Proactor模式multi-thread C++11网络库，目前支持在windows和linux上编译使用。     
 简单：对外只导出了少量的调用接口，所有的网络IO都封装为异步回调的形式，且接口声明都尽可能的像是调用系统socket API，对客户端而言，只多了一个新增的buffer对象类型。   
 快速：分别采用epoll和IOCP做底层事件驱动，其中epoll多线程惊群通过端口复用交由Linux内核处理。参照SGI STL和Nginx实现了内存池，每个建立连接的socket都独享一个内存池对象，所有从内存池中申请的内存都由智能指针管理。   
 明了：结构上分为三层：事件驱动层，会话管理层，接口层，各层之间通过回调向上通知。各个模块之间职责分工明确，上帝的事儿归上帝管，凯撒的事儿归凯撒管。最大的类不超过500行代码。   

## 接口

所有的接口文件都在 [include](/include) 中，其中关于库初始化和定时器的接口定义在 [CppNet](/include/CppNet.h) 中：   
```c++
    void Init(int32_t thread_num, bool log = false, bool per_handl_thread = false);
    void Dealloc();

    // thread join
    void Join();

    // must set callback before listen
    void SetReadCallback(const read_call_back& func);
    void SetWriteCallback(const write_call_back& func);
    void SetDisconnectionCallback(const connection_call_back& func);

    //timer
    uint64_t SetTimer(int32_t interval, const timer_call_back& func, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(const connection_call_back& func);
    bool ListenAndAccept(int16_t port, std::string ip, uint32_t listen_num);

    //client
    void SetConnectionCallback(const connection_call_back& func);
```
因为所有的网络IO接口都被定义为回调通知的模式，所以初始化库的时候需要设置各个调用的回调函数。     
这里通过设置回调而不是提供虚函数继承的方式，是希望尽量的简单，减少类的继承关系，增加回调的灵活性，你可以将回调设置为任意一个函数。      
关于网络IO的接口定义在[Socket](/include/Socket.h)中：   
```c++
    // get socket ip and adress
    int16_t GetIpAddress(const Handle& handle, std::string& ip, uint16_t& port);
    // post sync read event.
    int16_t SyncRead(const Handle& handle);
    // post sync write event.
    int16_t SyncWrite(const Handle& handle, const char* src, int32_t len);

    // post sync read event with time out
    int16_t SyncRead(const Handle& handle, int32_t interval);
    // post sync write event with time out
    int16_t SyncWrite(const Handle& handle, int32_t interval, const char* src, int32_t len);

    // post a sync task to io thread
    int16_t PostTask(std::function<void(void)>& func);
#ifndef __linux__
    // sync connection. 
    int16_t SyncConnection(const std::string& ip, int16_t port, const char* buf, int32_t buf_len);
#endif
    int16_t SyncConnection(const std::string& ip, int16_t port);

    int16_t SyncDisconnection(const Handle& handle);

    int16_t Close(const Handle& handle);
```
接口的作用通过声明和注释即可明了。需要关注的是接口返回的错误码，与回调函数的声明一起定义在[CppDefine](/include/CppDefine.h)中：
```c++
    enum CPPNET_ERROR_CODE {
        CEC_SUCCESS                = 1,    // success.
        CEC_TIMEOUT                = 2,    // the event time out call back.
        CEC_CLOSED                 = 3,    // remote close the socket.
        CEC_INVALID_HANDLE         = 4,    // invalid cppnet handle, can't find in socket manager.
        CEC_FAILED                 = 5,    // call function failed.
        CEC_CONNECT_BREAK          = 6,    // connect break.
        CEC_CONNECT_REFUSE         = 7     // remote refuse connect or server not exist.
    };
```
每个接口在采取下一步动作时应先检测一下当前返回的错误码，以获知当前连接是否正常。  

## 示例

简单使用实例可以看 [CppNetServer](/CppNetSev/CppNetServer.cpp) 和 [CppNetClient](/CppNetCli/CppNetClient.cpp)。   
其他示例在 [test](/test) 目录下，[echo](/test/echo)实现了10000连接量的echo的测试程序，[http](/test/http)参照muduo实现了一个简单的http服务器。

## 编译(Windows)

你可以使用vs2017来编译CppNet库和示例。    

## 编译(Linux)

只需要在源码目录下执行make即可编译CppNet库和示例。   
其他示例则需要在编译完静态库之后，分别在本地目录里执行make。   
```
$ make
```

## 协议

Hudp使用BSD 3-Clause使用条款，详情请看[https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)。