#include <mutex>
#include <atomic>
#include <memory>
#include <utility>
#include <stdio.h>
#include <iostream>
#include <functional>
#include <unordered_map>

#include "include/cppnet.h"

#ifdef __win__
#include <winsock2.h>
void SetNoDelay(const uint64_t& sock) {
    int opt = 1;
    int ret = setsockopt(sock, SOL_SOCKET, TCP_NODELAY, (const char*)&opt, sizeof(opt));
}
#else
#include <netinet/tcp.h>
#include <netinet/in.h>
void SetNoDelay(const uint64_t& sock) {
    int optval = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof(optval)));
}
#endif

class Client;
class Session {
    public:
    Session(Client* owner) : 
        _owner(owner),
        _bytes_read(0),
        _messages_read(0) {

    }

    int64_t BytesRead() const {
       return _bytes_read;
    }

    int64_t MessagesRead() const {
       return _messages_read;
    }

    void OnConnection(cppnet::Handle handle);

    void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, uint32_t) {
       char buff[65535];
       ++_messages_read;

       uint32_t len_get = data->GetCanReadLength();
       _bytes_read += len_get;
       while (data->GetCanReadLength()) {
           uint32_t ret = data->Read(buff, 65535);
           handle->Write(buff, ret);
       }
    }

    Client* _owner;
    int64_t _bytes_read;
    int64_t _messages_read;
};

class Client {
public:
    Client(int block_size, int session_count, int timeout, const std::string& ip, int port, cppnet::CppNet* net) : 
                             _ip(ip), _port(port), _session_count(session_count), _timeout(timeout),
                             _block_size(block_size), _net(net) {
        _num_connected = 0;
        for (int i = 0; i < _block_size; ++i) {
            _message.push_back(static_cast<char>(i % 128));
        }
    }

    void Start() {
        _net->AddTimer(_timeout, std::bind(&Client::HandleTimeout, this, std::placeholders::_1), nullptr);

        for (int i = 0; i < _session_count; ++i) {
            _net->Connection(_ip, _port);
        }
    }

    const std::string& Message() const {
        return _message;
    }

    void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, uint32_t len) {
        auto session = (Session*)handle->GetContext();
        session->OnMessage(handle, data, len);
    }

    void OnConnect(cppnet::Handle handle, uint32_t error) {
        if (error == cppnet::CEC_SUCCESS) {
            _num_connected++;
            if (_num_connected.load() == _session_count) {
                std::cout << _session_count << " sessions all connected" << std::endl;
            }
            auto session = new Session(this);
            handle->SetContext(session);
            session->OnConnection(handle);

            std::lock_guard<std::mutex> lock(_mutex);
            _sessions[handle] = session;

        } else {
            std::cout << " something error while connect. error :  " << error << std::endl;
        }
    }

  void OnDisconnect(cppnet::Handle, uint32_t) {
      _num_connected.fetch_sub(1);
      if (_num_connected.load() == 0) {
          std::cout << _session_count << " sessions all disconnected" << std::endl;

          int64_t totalBytesRead = 0;
          int64_t totalMessagesRead = 0;

          std::lock_guard<std::mutex> lock(_mutex);
          for (const auto& session : _sessions) {
              totalBytesRead += session.second->BytesRead();
              totalMessagesRead += session.second->MessagesRead();
          }

          std::cout << totalBytesRead << " total bytes read" << std::endl;
          std::cout << totalMessagesRead << " total messages read" << std::endl;
          std::cout << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
                    << " average message size" << std::endl;
          std::cout << static_cast<double>(totalBytesRead) / ((_timeout / 1000) * 1024 * 1024)
                    << " MiB/s throughput" << std::endl;

          _net->Destory();
      }
  }

private:

    void HandleTimeout(void*) {
        std::cout << "timeout, to stop connections" << std::endl;
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& session : _sessions) {
            session.first->Close();
        }
    }

    std::string  _ip;
    int          _port;
    int          _session_count;
    int          _timeout;
    int          _block_size;
    std::string       _message;
    std::atomic_uint  _num_connected;
    cppnet::CppNet*   _net;

    std::mutex        _mutex;
    std::unordered_map<cppnet::Handle, Session*> _sessions;
};

void Session::OnConnection(cppnet::Handle handle) {
    SetNoDelay(handle->GetSocket());
    auto msg = _owner->Message();
    handle->Write(msg.c_str(), (uint32_t)msg.length());
}

int main(int argc, char* argv[]) {

    if (argc < 7) {
       std::cout << "please input [ip] [port] [thread count] [block size] [session count] [time out]" << std::endl;
       return -1;
    }
    
    std::string ip    = std::string(argv[1]);
    int port          = atoi(argv[2]);
    int threadCount   = atoi(argv[3]);
    int block_size    = atoi(argv[4]);
    int session_count = atoi(argv[5]);
    int timeout       = atoi(argv[6]);

    cppnet::CppNet* net = new cppnet::CppNet;
    net->Init(threadCount);

    Client client(block_size, session_count, timeout, ip, port, net);

    net->SetConnectionCallback(std::bind(&Client::OnConnect, &client, std::placeholders::_1, std::placeholders::_2));
    net->SetReadCallback(std::bind(&Client::OnMessage, &client, std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3));
    net->SetDisconnectionCallback(std::bind(&Client::OnDisconnect, &client, std::placeholders::_1, std::placeholders::_2));

    client.Start();

    net->Join();

    delete net;

    return 0;
}

