# Quick Start

### Build
Download the source code through `git`, and then compile the `cppnet` static library，please refer to [Build](../build/build.md).   
The header file to be referenced is defined in [include](../../include), just need reference a header file.
```c++
#include "include/cppnet.h"
```

### Use
Construct an instance of `cppnet` and call the `Init` interface for initialization, like
```c++
CppNet net;
net.Init(2);
```
Then set the callback notification function, like
```c++
net.SetAcceptCallback(std::bind(&HttpServer::OnConnection, &server, std::placeholders::_1, std::placeholders::_2));
net.SetWriteCallback(std::bind(&HttpServer::OnMessageSend, &server, std::placeholders::_1, std::placeholders::_2));
net.SetReadCallback(std::bind(&HttpServer::OnMessage, &server, std::placeholders::_1, std::placeholders::_2, 
                                              std::placeholders::_3));
net.SetDisconnectionCallback(std::bind(&HttpServer::OnDisConnection, &server, std::placeholders::_1, std::placeholders::_2));
```
Other API details, see [API](../api/api.md).   
Relevant examples can be referred to [test](../../test).

### Best Practices

**About Thread**   
+ On Linux and MacOS, Each received client socket will only be active in one thread, So you don't have to think about thread safety.    
+ On Windows, The IOCP schedules all IO threads to wake up, The read and write of each socket is not fixed in one thread, so thread safety should be considered.      
+ All network IO and callbacks are performed in child threads. Do not perform time-consuming operations in callbacks.   
+ If the main thread does not have other blocking operations, call the `Join` interface to block and wait for the network IO thread.   


**Connection Context**   
`CNSocket` Provided
```c++
virtual void SetContext(void* context) = 0;
virtual void* GetContext() = 0;
```
connection context setting interface. It can be set in the connection request callback, obtained in the send or read callback, and destructed in the connection disconnection callback. There is no need to maintain a central data structure in the application layer.   
The use process can be referred to [httpser](../../test/http/http_server.cpp)。

**Socket Object**   
The `CNSocket` handle that is passed in each callback, it's life cycle is maintained by intelligent pointer internal `cppnet`. It is not recommended to hold it in the application layer, so as not to affect the resource release after the connection is closed.

**Configure**   
Common configurations are defined in [cppnet_config.h](../../cppnet/cppnet_config.h), The function of each configuration item can be referred to the notes.      
On the Linux platform, the efficiency of opening the `EPOLLEXCLUSIVE` will be improved by nearly `40%`(see [ab bench](../efficiency/apache_ab_bench.md)), If the kernel version is higher than `4.5`, it is recommended to enable this setting.

**Multi Port**   
When there are multiple ports to listen on, it can be called repeatedly
```c++
bool ListenAndAccept(const std::string& ip, uint16_t port);
```
In the callback notification of connection establishment, through 
```c++
virtual uint16_t GetListenPort() = 0;
```
interface gets the listening port to distinguish different services.  
The use process can be referred to [multi_port](../../test/multi_port/multi_port_server.cpp)。  