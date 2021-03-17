#ifndef CPPNET_CPPNET_BASE
#define CPPNET_CPPNET_BASE

#include <memory>
#include "include/cppnet_type.h"

namespace cppnet {

class Socket;
class EventActions;

class CppNetBase: public std::enable_shared_from_this<CppNetBase> {
public:
    CppNetBase();
    ~CppNetBase();
    // common
    void Init(uint32_t thread_num);
    void Dealloc();
    void Join();

    // set call back
    void SetReadCallback(const read_call_back& cb) { _read_call_back = cb; }
    void SetWriteCallback(const write_call_back& cb) { _write_call_back = cb; }
    void SetDisconnectionCallback(const connect_call_back& cb) { _disconnec_call_back = cb; }

    // about timer
    uint64_t SetTimer(uint32_t interval, const std::function<void(void*)>& func, void* param = nullptr, bool always = false);
    void RemoveTimer(uint64_t timer_id);

    //server
    void SetAcceptCallback(const connect_call_back& cb) { _accept_call_back = cb; }
    bool ListenAndAccept(const std::string& ip, uint16_t port);

    //client
    void SetConnectionCallback(const connect_call_back& cb) { _connec_call_back = cb; }
    bool Connection(const std::string& ip, uint16_t port);

    // call back
    void OnAccept(std::shared_ptr<Socket> sock);
    void OnRead(std::shared_ptr<Socket> sock, uint32_t len);
    void OnWrite(std::shared_ptr<Socket> sock, uint32_t len);
    void OnConnect(std::shared_ptr<Socket> sock, uint16_t err);
    void OnDisConnect(std::shared_ptr<Socket> sock, uint16_t err);

private:
    read_call_back     _read_call_back;
    write_call_back    _write_call_back;
    connect_call_back  _connec_call_back;
    connect_call_back  _disconnec_call_back;
    connect_call_back  _accept_call_back;

    /*
    std::mutex              _mutex;
    std::vector<std::shared_ptr<std::thread>>                                _thread_vec;
    std::unordered_map<uint64_t, base::CMemSharePtr<CAcceptSocket>>          _accept_socket;
    std::unordered_map<std::thread::id, std::shared_ptr<CEventActions>>      _actions_map;
    */
};

}
#endif