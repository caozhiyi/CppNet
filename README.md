<p align="center"><img width="500" src="./doc/image/logo.png" alt="cppnet logo"></p>

<p align="center">
    <a href="https://travis-ci.org/caozhiyi/CppNet"><img src="https://travis-ci.org/caozhiyi/CppNet.svg?branch=master" alt="Build Status"></a>
    <a href="https://opensource.org/licenses/BSD-3-Clause"><img src="https://img.shields.io/badge/license-bsd-orange.svg" alt="Licenses"></a>
</p> 

## Introduction

CppNet is a multithreaded network with C++11.                                                                          
It can run on Linux and Windows based on epoll and IOCP.
The base module encapsulates some basic components, includeing:
thread operations base class:(https://blog.csdn.net/u012778714/article/details/80557100),
log Printing:(https://blog.csdn.net/u012778714/article/details/80557596),
message queue:(https://blog.csdn.net/u012778714/article/details/80557596),
memory pool:(https://blog.csdn.net/u012778714/article/details/80299475),
smart pointer on memory pool:(https://blog.csdn.net/u012778714/article/details/80503634),
time tool:(https://blog.csdn.net/u012778714/article/details/80557761).

Network module abstract common interfaces over operating systems, 
All operations are encapsulated in two stages, requests and callbacks.

## Example

See [CppNetClient](/CppNetSev/CppNetServer.cpp) and [CppNetClient](/CppNetCli/CppNetClient.cpp).
Interface of operating library is in [CppNet](/include/CppNet.h).
Interface of network operating is in [Socket](/include/Socket.h).

## Build(Windows)

You can compile with vs2017.

## Build(Linux)

only make, build cppnet library and example.
```
$ make
```

## Licenses

This program is under the terms of the BSD 3-Clause License. See [https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause).



Use of this source code is governed by a BSD-style license.
