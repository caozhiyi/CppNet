# 快速开始

### 编译
通过`git`下载源码，然后编译`cppnet`静态库，编译请参考[编译](../build/build_cn.md)。   
需要引用的头文件定义在[include](../../include)目录中，只需引用一个头文件即可。
```c++
#include "include/cppnet.h"
```

### 使用
构造`CppNet`实例，并调用`Init`接口初始化，类似
```c++
CppNet net;
net.Init(2);
```
之后设置各个关心的回调通知函数，类似：
```c++
net.SetAcceptCallback(std::bind(&HttpServer::OnConnection, &server, std::placeholders::_1, std::placeholders::_2));
net.SetWriteCallback(std::bind(&HttpServer::OnMessageSend, &server, std::placeholders::_1, std::placeholders::_2));
net.SetReadCallback(std::bind(&HttpServer::OnMessage, &server, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3));
net.SetDisconnectionCallback(std::bind(&HttpServer::OnDisConnection, &server, std::placeholders::_1, std::placeholders::_2));
```
相关使用示例可参考[test](../../test)。

### 最佳实践

**线程**   
+ 在Linux和MacOS上，每个接收到的client socket都只会在一个线程中活动，所以不必考虑线程安全问题。  
+ Windows上，由IOCP调度所有的IO线程唤醒，每个socket的读写不固定在一个线程内，需考虑线程安全问题。   
+ 所有的网络IO和回调均在子线程中进行，请勿在各回调中进行耗时操作。   
+ 若主线程没有其他阻塞操作，请调用`Join`接口阻塞等待网络IO线程。


**连接上下文**   
`CNSocket`提供了
```c++
virtual void SetContext(void* context) = 0;
virtual void* GetContext() = 0;
```
连接上下文设置接口，可以在连接请求的回调中设置，在发送或读取回调中获取，在连接断开回调中析构，不用在应用层维护一个中心的数据结构。   
使用过程可参考[httpser](../../test/http/http_server.cpp)。

**Socket对象**   
建立连接的`CNSocket`句柄在每次回调中传递，`cppnet`内部通过智能指针维护其生命周期，不建议在应用层持有，以免影响连接关闭后的资源释放。

**配置**   
通用的配置都定义在[cppnet_config.h](../../cppnet/cppnet_config.h)文件中，其中各配置项作用可参考注释。   
在Linux平台上，开启`EPOLLEXCLUSIVE`标识效率会提升近`40%`(参考[ab压测结果](../efficiency/apache_ab_bench_cn.md))，如果内核版本高于`4.5`建议开启此项设置。