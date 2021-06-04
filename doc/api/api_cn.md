# API

`cppnet`对外接口主要包括三种类型，也分别定义在三个头文件中：
- 初始化和全局配置类
- `socket`操作类
- `buffer`读取类

## 初始化和全局配置类
此类接口定义于[cppnet](../../include/cppnet.h)文件。    
包括`cppnet`库生命周期及线程的管理，相关网络IO通知回调的设置，以及用户自定义定时器等。    
虽然是全局类接口，却并没有声明为全局函数。何也？为了让一个应用进程可以使用多个cppnet网络库实例。   

### 生命周期管理

#### **初始化`cppnet`网络库**
```c++
void Init(int32_t thread_num = 0);
```
`说明`：   
调用此接口即开始初始化`cppnet`网络库，主要是申请和初始化一些全局类的资源，当然，在`windows`平台也会初始化`WSA`库。   

`参数`：   
`thread_num`：`cppnet`将启动的网络IO线程数量，为0或者数量大于CPU内核数的两倍时，将默认设置为CPU核数量。

#### **析构cppnet网络库**
```c++
void Destory();
```
`说明`：   
调用此接口销毁`cppnet`网络库，并关闭所有网络IO线程。请主动调用此接口释放资源，析构`cppnet`类实例并不能自动释放。

#### **线程等待**
```c++
void Join();
```
`说明`：   
阻塞等待所有网络IO线程，可与主线程业务逻辑结合，非必须调用。

### 全局配置类
#### **设置读取通知回调**
```c++
typedef std::function<void(Handle handle, BufferPtr data, uint32_t len)> read_call_back;
void SetReadCallback(const read_call_back& cb);
```
`说明`：   
设置全局读取通知回调函数。    
连接中断不会通知到此接口，所以回调中`len`值必大于0。

#### **设置发送通知回调**
```c++
typedef std::function<void(Handle handle, uint32_t len)> write_call_back;
void SetWriteCallback(const write_call_back& cb);
```
`说明`：   
设置全局发送通知回调函数，若对发送结果不关心，可不设置。    
连接中断不会通知到此接口，所以回调中`len`值必大于0。 

#### **设置连接中断回调通知**
```c++
typedef std::function<void(Handle handle, uint32_t err)> connect_call_back;
void SetDisconnectionCallback(const connect_call_back& cb);
```
`说明`：   
设置全局连接中断通知回调函数，若对连接状态不关心，可不设置。   
在回调通知中，请关注`err`值:    
   + 若本地主动断开连接，则返回 `CEC_SUCCESS`
   + 若对端主动断开连接，则返回 `CEC_CLOSED`
   + 若连接发生错误而中断，则返回 `CEC_CONNECT_BREAK`

#### **设置socket定时器回调通知**
```c++
typedef std::function<void(Handle)> timer_call_back;
void SetTimerCallback(const timer_call_back& cb);
```
`说明`：   
设置全局socket定时器通知回调函数，若不使用socket定时器，可不设置。    
socket定时器使用参考`cppnet_socket`接口说明。

#### **设置连接请求回调通知**
```c++
typedef std::function<void(Handle handle, uint32_t err)> connect_call_back;
void SetAcceptCallback(const connect_call_back& cb);
```
`说明`：   
服务端收到连接请求后将回调此接口。   
设置全局连接请求通知回调函数，若对连接请求不关心，可不设置。   
在回调通知中，`err`值固定为`CEC_SUCCESS`。   

#### **设置连接结果回调通知**
```c++
typedef std::function<void(Handle handle, uint32_t err)> connect_call_back;
void SetConnectionCallback(const connect_call_back& cb);
```
`说明`：   
客户端发起连接请求后，将连接结果通知到此接口。   
在回调通知中，`err`值为：
   + 若连接成功，则返回 `CEC_SUCCESS`
   + 若连接失败，则返回 `CEC_CONNECT_REFUSE`

### 连接操作类
#### **服务端开启端口服务**
```c++
bool ListenAndAccept(const std::string& ip, uint16_t port);
```
`说明`：   
开启对应`ip`和`port`上的监听服务。    
收到的连接请求将回调到`SetAcceptCallback`设置的回调函数中。   

#### **客户端发起连接请求**
```c++
bool Connection(const std::string& ip, uint16_t port);
```
`说明`：   
发起对应`ip`和`port`的连接请求。  
连接结果将回调到`SetConnectionCallback`设置的回调函数中。

### 定时器类

#### **添加定时器**
```c++
typedef std::function<void(void*)> user_timer_call_back;
uint64_t AddTimer(int32_t interval, const user_timer_call_back& cb, void* param = nullptr, bool always = false);
```
`说明`：   
设置自定义定时器，定时器回调将随机绑定到某个网络IO线程上。    
返回值为定时器ID，可用于移除定时器。

`参数`：   
`interval`：定时器超时时间，单位为毫秒。   
`cb`：定时器回调通知函数。   
`param`：定时器回调通知参数。   
`always`：定时器是否周期性回调。   

#### **移除定时器**
```c++
void RemoveTimer(uint64_t timer_id);
```

## socket操作类
此类接口定义于[cppnet_socket](../../include/cppnet_socket.h)文件。    
socket操作包括信息获取以及数据写入和关闭。    
数据读取通过回调通知，并不能通过socket操作读取数据。    
socket实例通过上述回调接口的第一个参数获取，不能主动创建socket实例，也不建议应用层维护和持有socket实例。   

#### **获取系统socket句柄**
```c++
virtual uint64_t GetSocket() = 0;
```

#### **获取对端地址**
```c++
virtual bool GetAddress(std::string& ip, uint16_t& port) = 0;
```

#### **发送数据**
```c++
virtual bool Write(const char* src, uint32_t len) = 0;
```
`说明`：   
发送结果将通知到`SetWriteCallback`接口设置的回调函数中。   
当网络繁忙，发送缓存满时，此接口可能调用失败。    
发送缓存大小见[cppnet_config](../../cppnet/cppnet_config.h)中`__max_write_cache`。

#### **关闭连接**
```c++
virtual void Close() = 0;
```
`说明`：   
主动关闭连接，结果将通知到`SetDisconnectionCallback`接口设置的回调函数中。  

#### **添加定时器**
```c++
virtual void AddTimer(uint32_t interval, bool always = false) = 0;
```
`说明`：   
添加定时器，定时器将通知到`SetTimerCallback`接口设置的回调函数中。

`参数`：   
`interval`：定时器超时时间，单位为毫秒。   
`always`：定时器是否周期性回调。   

#### **移除定时器**
```c++
virtual void StopTimer() = 0;
```

## buffer读取类
此类接口定义于[cppnet_buffer](../../include/cppnet_buffer.h)文件。   
包括不同方式的数据读取接口，并不支持数据写入。   

#### **数据读取而不移动游标**
```c++
virtual uint32_t ReadNotMovePt(char* res, uint32_t len) = 0;
```
`说明`：   
此接口可用于数据嗅探，读取指定数据的数据而不移动读取游标，意味着可以重复读取。    
注意检测返回值。    

#### **数据读取**
```c++
virtual uint32_t Read(char* res, uint32_t len) = 0;
```
`说明`：   
注意检测返回值。   

#### **清除**
```c++
virtual void Clear() = 0;
```

#### **指定数量读取**
```c++
virtual uint32_t ReadUntil(char* res, uint32_t len) = 0;
```
`说明`：   
强制读取指定数量数据，当可读数据小于len值时，不读取数据。   

#### **移动读取游标**
```c++
virtual int32_t MoveReadPt(int32_t len) = 0;
```
`说明`：   
可配合`ReadNotMovePt`接口使用，因为读取过的`buffer`将归还内存池，所以只支持向后移动游标。   

#### **读取到指定字符**
```c++
virtual uint32_t ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len) = 0;
```
`说明`：   
当没有找到指定字符串时，不做任何读取。    
当找到指定字符串但是读取缓存不够时，在最后一个参数返回需要字符数量。    

`参数`：   
`res`：读取缓存地址。    
`len`：读取缓存长度。
`find`：指定字符串地址。
`find_len`：指定字符串长度。
`need_len`：返回需要字节长度。

#### **可读数据长度**
```c++
virtual uint32_t GetCanReadLength() = 0;
```

#### **查找字符串**
```c++
    virtual uint32_t FindStr(const char* s, uint32_t s_len) = 0;
```
`说明`：  
返回查找到字符串首位置的索引。   