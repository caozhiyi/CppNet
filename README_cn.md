<p align="left"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="left">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

查看更多细节 [wiki](https://github.com/caozhiyi/CppNet/wiki)

## 简介

Cppnet是一个封装在TCP协议上的proactor模式multi-thread C++11网络库，支持在windows，linux以及macOS上编译使用。     
 - `简单`：   
    + 对外暴漏最少接口，所有的网络响应都封装为异步回调的形式
    + 接口声明类似系统原生
    + 只新增一个buffer接口集
    + 支持IPv6和IPv4

- `快速`：    
    + 分别采用epoll，IOCP，kqueue做底层事件驱动
    + 多线程惊群交由内核处理
    + 参照SGI STL和Nginx实现了内存池，每个连接都独享一个内存池对象，所有从内存池中申请的内存都由智能指针管理
    + 用时间轮实现定时器

- `明了`： 
    + 结构上分为三层：事件驱动层，会话管理层，接口层，各层之间通过回调向上通知
    + 各个模块之间职责分工明确，上帝的事儿归上帝管，凯撒的事儿归凯撒管
    + 通过接口解耦模块，符合最小接口原则和依赖倒置原则

## 接口

`cppnet`对外接口主要包括三种类型，也分别定义在三个头文件中：
- 初始化和全局配置类，定义在[cppnet](/include/cppnet.h)
- `socket`操作类，[cppnet_socket](/include/cppnet_socket.h)
- `buffer`读取类，定义在[cppnet_buffer](/include/cppnet_buffer.h)
   
接口详情请参考[API](/doc/api/api_cn.md)。

## 示例

所有示例都在 [test](/test) 目录下:   
- [simple](/test/simple)是一个简单的使用示例。   
- [echo](/test/echo)实现了10000连接量的echo的测试程序。   
- [http](/test/http)参照muduo实现了一个简单的http服务器。   
- [sendfile](/test/sendfile)是一个文件发送和接收示例。   
- [pingpong](/test/pingpong)是一个pingpong测试程序。   
- [rpc](/test/rpc)是一个简单的rpc示例。   

## 效率

目前用Apache `ab` 对[test](/test)目录中的`http`测试服务做了压测，并与`muduo`做了对比。   
详情请看[ab压测](/doc/efficiency/apache_ab_bench_cn.md)。

## 编译

请看[编译](/doc/build/build_cn.md)

## 协议

CppNet使用BSD 3-Clause使用条款，详情请看[BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)。
