# API

`cppnet` has three types of external interfaces:
- Initialization and global configuration classes
- `socket` operation
- `buffer` operation

## Initialization And Global Configuration
This type of interface is defined in [cppnet](../../include/cppnet.h).    
It includes the management of `cppnet` library life cycle and thread, the setting of related network IO notification callback, and user-defined timer.       
Although it is a global class interface, it is not declared as a global function. Why? In order to enable an application process to use multiple cppnet network library instances.   

### Life Cycle Management

#### **Initialize `cppnet` Network Library**
```c++
void Init(int32_t thread_num = 0);
```
`explain`:   
Call this interface to initialize the `cppnet` network library, mainly to apply for and initialize some global class resources. Of course, the `WSA` library will also be initialized on the `Windows` platform.     

`param`:   
`thread_num`：if is set to 0 or more than twice the number of CPU cores, it will be set to the number of CPU cores by default.   

#### **Destroy `cppnet` Library**
```c++
void Destory();
```
`explain`:   
Call this interface to destroy the `cppnet` Network Library and close all network IO threads.   

#### **Thread Waiting**
```c++
void Join();
```
`explain`:   
Blocking and waiting for all network IO threads, can be combined with the business logic of the main thread and may not be called.   

### Global Configuration
#### **Set Read Notification Callback**
```c++
typedef std::function<void(Handle handle, BufferPtr data, uint32_t len)> read_call_back;
void SetReadCallback(const read_call_back& cb);
```
`explain`:   
Set the global read notification callback function.   
Connection interruption will not notify this interface, so the `len` value in the callback must be greater than 0.   

#### **Set Send Notification Callback**
```c++
typedef std::function<void(Handle handle, uint32_t len)> write_call_back;
void SetWriteCallback(const write_call_back& cb);
```
`explain`:   
Set the global send notification callback function. If you don't care about the sending result, you don't need to set it.      
Connection interruption will not notify this interface, so the `len` value in the callback must be greater than 0.   

#### **Set Connection Interrupt Callback Notification**
```c++
typedef std::function<void(Handle handle, uint32_t err)> connect_call_back;
void SetDisconnectionCallback(const connect_call_back& cb);
```
`explain`:   
Set the global connection interrupt notification callback function. If you do not care about the connection status, you can not set it.   
In callback notification, please pay attention to the `err` value:    
   + If the local active disconnect, will get `CEC_SUCCESS`
   + If the opposite end actively disconnects, will get `CEC_CLOSED`
   + If the connection is interrupted due to an error, will get `CEC_CONNECT_BREAK`

#### **Set Socket Timer Callback Notification**
```c++
typedef std::function<void(Handle)> timer_call_back;
void SetTimerCallback(const timer_call_back& cb);
```
`explain`:   
Set the global socket timer notification callback function, if you do not use the socket timer, you can not set it.      
Socket timer use can be see `cppnet_socket` interface description.   

#### **Set Connection Request Callback Notification**
```c++
typedef std::function<void(Handle handle, uint32_t err)> connect_call_back;
void SetAcceptCallback(const connect_call_back& cb);
```
`explain`:   
The server will call back this interface after receiving the connection request.   
Set the global connection request notification callback function. If you don't care about the connection request, don't set it.   
In callback notification, the `err` value is fixed to `CEC_SUCCESS`.    

#### **Set Connection Result Callback Notification**
```c++
typedef std::function<void(Handle handle, uint32_t err)> connect_call_back;
void SetConnectionCallback(const connect_call_back& cb);
```
`explain`:   
Set the connection result callback to notify the client of the connection request, and then notify this interface of the connection result.   
In callback notification, `err` value is:
   + If the connection is successful, will get `CEC_SUCCESS`
   + If the connection fails, will get `CEC_CONNECT_REFUSE`

### Connection Operation
#### **Server Opens Port Service**
```c++
bool ListenAndAccept(const std::string& ip, uint16_t port);
```
`explain`:   
Start the listening service on the corresponding `ip` and `port`.   
The received connection request will be called back to the callback function set by `SetAcceptCallback`.   

#### **Client Initiates Connection Request**
```c++
bool Connection(const std::string& ip, uint16_t port);
```
`explain`:   
Initiate the connection request corresponding to `ip` and `port`.   
The connection result will be called back to the callback function set by `SetConnectionCallback`.

### Timer

#### **Add Timer**
```c++
typedef std::function<void(void*)> user_timer_call_back;
uint64_t AddTimer(int32_t interval, const user_timer_call_back& cb, void* param = nullptr, bool always = false);
```
`explain`:   
Set a custom timer, the timer callback will be randomly bound to a network IO thread.   
The return value is the timer ID, which can be used to remove the timer.   

`param`:   
`interval`: Timer timeout, in milliseconds.   
`cb`: Timer callback notification function.   
`param`: Timer callback notification parameter.     
`always`: Whether the timer calls back periodically.   

#### **Remove Timer**
```c++
void RemoveTimer(uint64_t timer_id);
```

## Socket Operation
This type of interface is defined in [cppnet_socket](../../include/cppnet_socket.h).       
Socket operation includes information acquisition, data writing and closing.    
Data reading is notified by callback, but not by socket operation.      
The socket instance is obtained by the first parameter of the callback interface, and it is not allowed to actively create the socket instance, nor recommend the application layer to maintain and hold the socket instance.    

#### **Get System Socket Handle**
```c++
virtual uint64_t GetSocket() = 0;
```

#### **Get Peer Address**
```c++
virtual bool GetAddress(std::string& ip, uint16_t& port) = 0;
```

#### **Send Data**
```c++
virtual bool Write(const char* src, uint32_t len) = 0;
```
`explain`:   
The sending result will be notified to the callback function set by the `SetWriteCallback` interface.   
When the network is busy and the send cache is full, the call to this interface may fail.   
The size of the send cache is shown in [cppnet_config](../../cppnet/cppnet_config.h) `__ max_ write_ cache`.

#### **Close Connection**
```c++
virtual void Close() = 0;
```
`explain`:   
If the connection is closed actively, the result will be notified to the callback function set by the `SetDisconnectionCallback` interface.

#### **Add Timer**
```c++
virtual void AddTimer(uint32_t interval, bool always = false) = 0;
```
`explain`:   
Add a timer, and the timer will be notified to the callback function set by the `SetTimerCallback` interface.

`param`:   
`interval`: Timer timeout, in milliseconds.   
`always`: Whether the timer calls back periodically.    

#### **Remove Timer**
```c++
virtual void StopTimer() = 0;
```

## Buffer Read
This type of interface is defined in [cppnet_buffer](../../include/cppnet_buffer.h).      
It includes different type of reading interfaces and does not support data writing.   

#### **Data Read Without Moving Cursor**
```c++
virtual uint32_t ReadNotMovePt(char* res, uint32_t len) = 0;
```
`explain`:   
This interface can be used for data sniffing to read the data of the specified data without moving the read cursor, which means that it can be read repeatedly.   
Pay attention to the return value.   

#### **Data Read**
```c++
virtual uint32_t Read(char* res, uint32_t len) = 0;
```
`explain`:   
Pay attention to the return value.   

#### **Clear Data**
```c++
virtual void Clear() = 0;
```

#### **Specified Number Of Reads**
```c++
virtual uint32_t ReadUntil(char* res, uint32_t len) = 0;
```
`explain`:   
Force to read the specified amount of data, when the readable data is less than len value, do not read the data.   

#### **Move Read Cursor**
```c++
virtual int32_t MoveReadPt(int32_t len) = 0;
```
`explain`:     
It can be used with the `ReadNotMovePt` interface. Because the read `buffer` will be returned to the memory pool, it only supports moving the cursor backward.

#### **Read Before Specified String**
```c++
virtual uint32_t ReadUntil(char* res, uint32_t len, const char* find, uint32_t find_len, uint32_t& need_len) = 0;
```
`explain`:   
No reads are made when the specified string is not found.   
When the specified string is found but the read cache is insufficient, the number of characters required is returned in the last parameter.    

`param`:   
`res`: read cache address.    
`len`: read cache length.   
`find`: specifies the string address.     
`find_len`: the length of specifies string.    
`need_len`: returns the length of bytes required.    

#### **Length Of Readable Data**
```c++
virtual uint32_t GetCanReadLength() = 0;
```

#### **Find String**
```c++
virtual uint32_t FindStr(const char* s, uint32_t s_len) = 0;
```
`explain`:  
Returns the index found to the beginning of a string.   