# Apache ab 压测

Apache `ab` 是一个常用的http服务器压测工具。这里对`cppnet` test目录中的`http`测试服务分别在不同平台进行压测，执行的命令为：
```shell
ab -kc[1-10000] -n100000 http://127.0.0.1:8921/hello
```
每次将上述命令执行三次，取三次中QPS的最高值，并将测试结果绘制为折线图。   
折线图中纵坐标为QPS, 横坐标为并发值。   

### Linux

**压测环境**：   
- 系统环境为Widnows的`WSL2`
- 操作系统为`Ubuntu20.04`
- 系统内存`16G DDR3 2666Mhz`
- CPU为 `i7-6700 @ 3.40GHz` 四核
- 编译优化为`-O2`
- 所有测试均开`2`个线程

在Linux上与`muduo`进行了对比，由于Linux上可以在`REUSE_PORT`之外额外支持`EPOLLEXCLUSIVE`，所以又添加了两种不同选项的对比，结果如下图所示
<p align="left"><img width="896" src="../image/linux_apache_ab_bench.png" alt="linux_apache_ab_bench"></p>

图中`cppnet_1`为使用`REUSE_PORT`，`cppnet_2`为使用`EPOLLEXCLUSIVE`。    
使用`REUSE_PORT`时，整体效率稍稍优于`muduo`。    
在使用`EPOLLEXCLUSIVE`时，性能整体都优于`muduo`(muduo尚未支持`EPOLLEXCLUSIVE`设置)。   

### Windows

**压测环境**：   
- 系统环境为物理真机
- 操作系统为`Windows 10 @ 19042.985`
- 系统内存`16G DDR3 2666HZ`
- CPU为 `i7-6700 @ 3.40GHz` 四核
- 编译优化为`/O2`

由于在Windows环境使用的是`IOCP`，系统内核负责管理线程池，各个socket唤醒操作将在不同的线程上执行，这与Linux和MacOS大相径庭，所以特意列了不同线程数量时的压测表现，结果如下图所示
<p align="left"><img width="896" src="../image/windows_apache_ab_bench.png" alt="linux_apache_ab_bench"></p>

### MacOS

**压测环境**：   
- 系统环境为物理真机
- 操作系统为`MacOS @ 10.15.4`
- 系统内存`8G DDR3 2133Mhz`
- CPU为 `i5-8257U @ 1.40GHz` 四核
- 编译优化为`-O2`
- 开`2`个线程

压测结果如下图所示 
<p align="left"><img width="896" src="../image/macos_apache_ab_bench.png" alt="linux_apache_ab_bench"></p>

### 结语
跨平台与高性能往往难以兼得，由于先开发的Linux平台，后适配的Windows平台，且`IOCP`的使用方式与`Epoll`和`Kqueue`有着极大的不同，框架上也没法过于妥协。Windows平台上有很多保证线程安全的锁，并没有完美发挥出`IOCP`的性能，所以在Windows平台上性能表现一般。        
在Linux和MacOS上，常规并发条件下，大部分QPS都能超过10万，性能表现还算优异。