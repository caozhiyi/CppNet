<p align="left"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="left">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

See [chinese](/README_cn.md)    
See the details in chinese [Wiki](https://github.com/caozhiyi/CppNet/wiki)
## Introduction

Cppnet is a proactor mode and multithreaded network with C++11 on tcp. Support Window, Linux and macOS.    
 - `Simple`:    
    + Only export a little interfaces, all net io insterface are asynchronous callbacks
    + Insterface as much as possible like calling the socket API of the system
    + There is only one additional buffer interface for the client
    + Support both IPv4 and IPv6

 - `Fast`: 
    + Use epoll, IOCP and kqueue
    + Multithreaded threads are handled by the kernel
    + Each socket has a single memory pool object. All memory requested from the memory pool is managed by an intelligent pointer 
    + Using time wheel to realize timer   
 
 - `Clear`:
    + Three layers: event-driven layer, session management layer and interface layer
    + Upward notification through callbacks between layers. Clear division of responsibilities among modules, pay to Caesar what belongs to Caesar and God what belongs to God
    + The interface decoupling module is used to meet the minimum interface principle and dependency inversion principle  

## Interface

`cppnet` has types of external interfaces, which are also defined in three header files   
- Initialization and global configuration, defined in [cppnet](/include/cppnet.h)   
- `socket` operation, defined in [cppnet_socket](/include/cppnet_socket.h)   
- `buffer` read, defined in [cppnet_buffer](/include/cppnet_buffer.h)   
   
API details see [API](/doc/api/api.md).   

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

## Build

Look at [Build](/doc/build/build.md)

## Licenses

This program is under the terms of the BSD 3-Clause License. See [https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause).
