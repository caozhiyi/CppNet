<p align="left"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="left">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

See [chinese](/README_cn.md)    
See the details in chinese [wiki](https://github.com/caozhiyi/CppNet/wiki)
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

## Quick Start
Quick use of `cppnet` and precautions, see [quick start](doc/start/quick_start.md).

## Interface

`cppnet` has three types of external interfaces, which are also defined in three header files   
- Initialization and global configuration, defined in [cppnet](/include/cppnet.h)   
- `socket` operation, defined in [cppnet_socket](/include/cppnet_socket.h)   
- `buffer` read, defined in [cppnet_buffer](/include/cppnet_buffer.h)   
   
For details, see [API](/doc/api/api.md).   

## Example

All simples are in [test](/test):   
- [simple](/test/simple): Most simple example.   
- [echo](/test/echo): Test program of echo with 200 connection.   
- [http](/test/http): Simple HTTP server is implemented with reference to muduo.   
- [sendfile](/test/sendfile): Example of sending and receiving files.   
- [pingpong](/test/pingpong): Pingpong test program.   
- [rpc](/test/rpc): Interesting rpc program.   
- [multi_port](/test/multi_port): Example of multiple listening ports.      

## Efficiency

Apache `ab` is used to pressure test the `http` test service in the [test](/test) directory, and compared with `muduo`.    
For details, see [ab benchmark](/doc/efficiency/apache_ab_bench.md)。

## Build

Look at [Build](/doc/build/build.md)

## Licenses

This program is under the terms of the BSD 3-Clause License. For details, see [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause).
