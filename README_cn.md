<p align="left"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="left">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

查看更多细节 [Wiki](https://github.com/caozhiyi/CppNet/wiki)

## 简介

Cppnet是一个封装在TCP协议上的proactor模式multi-thread C++11网络库，支持在windows，linux以及macOS上编译使用。     
 - **简单**：   
    + 对外暴漏最少接口，所有的网络响应都封装为异步回调的形式
    + 接口声明类似系统原生
    + 只新增一个buffer接口集
    + 支持IPv6和IPv4

- **快速**：    
    + 分别采用epoll，IOCP，kqueue做底层事件驱动
    + 多线程惊群交由内核处理
    + 参照SGI STL和Nginx实现了内存池，每个连接都独享一个内存池对象，所有从内存池中申请的内存都由智能指针管理
    + 用时间轮实现定时器

- **明了**： 
    + 结构上分为三层：事件驱动层，会话管理层，接口层，各层之间通过回调向上通知
    + 各个模块之间职责分工明确，上帝的事儿归上帝管，凯撒的事儿归凯撒管
    + 通过接口解耦模块，符合最小接口原则和依赖倒置原则

## 接口

所有的接口文件都在 [include](/include) 中，其中关于库初始化和用户自定义定时器的接口定义在 [CppNet](/include/CppNet.h) 中：   
```c++
// cppnet instance
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
因为所有的网络IO接口都被定义为回调通知的模式，所以初始化库的时候需要设置各个通知的回调函数。     
这里通过设置回调而不是提供虚函数继承的方式，是希望尽量的简单，减少类的继承关系，增加回调的灵活性，你可以将回调设置为任意一个函数。      
关于网络IO的接口定义在[Socket](/include/Socket.h)中：   
```c++
class CNSocket {
public:
    // get socket
    virtual uint64_t GetSocket() = 0;
    // get socket IP and address
    virtual bool GetAddress(std::string& ip, uint16_t& port) = 0;
    // post sync write event.
    virtual bool Write(const char* src, uint32_t len) = 0;
    // close the connect
    virtual bool Close() = 0;
    // add a timer. must set timer call back
    // interval support max 1 minute
    // return a timer id
    virtual uint64_t AddTimer(uint32_t interval, bool always = false) = 0;
    virtual void StopTimer(uint64_t timer_id) = 0;
};
```
接口的作用通过声明和注释即可明了。需要关注的是接口返回的错误码，与回调函数的声明一起定义在[CppDefine](/include/CppDefine.h)中：
```c++
    // error code
    CEC_SUCCESS                = 0,    // success.
    CEC_CLOSED                 = 1,    // remote close the socket.
    CEC_CONNECT_BREAK          = 2,    // connect break.
    CEC_CONNECT_REFUSE         = 3,    // remote refuse connect or server not exist.
```
关于连接状态的所有通知都会回调到连接相关的函数中。    

## 示例

所有示例都在 [test](/test) 目录下:   
[simple](/test/simple)是一个简单的使用示例。   
[echo](/test/echo)实现了10000连接量的echo的测试程序。   
[http](/test/http)参照muduo实现了一个简单的http服务器。   
[sendfile](/test/sendfile)是一个文件发送和接收示例。   
[pingpong](/test/pingpong)是一个pingpong测试程序。   
[rpc](/test/rpc)是一个简单的rpc示例。   

## 效率
目前只用ab做了http echo测试，与muduo做了对比，执行的命令为：ab -kc[1-2000] -n100000 http://127.0.0.1:8921/hello.
<p align="left"><img width="896" src="./doc/image/muduo_vs_cppnet.png" alt="mudo vs cppnet"></p>


## 编译

编译可参考[这里](/doc/build/build_cn.md)

## 协议

CppNet使用BSD 3-Clause使用条款，详情请看[https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)。
