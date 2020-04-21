#include <atomic>
#include <memory>
#include <utility>
#include <stdio.h>
#include <iostream>
#include <functional>
#include <unordered_map>

#include "CppNet.h"
#include "Socket.h"
#include "Runnable.h"

#ifndef __linux__
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

using namespace cppnet;

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

    void OnConnection(const Handle& handle);

    void OnMessage(const Handle& handle, base::CBuffer* data, uint32_t) {
       char buff[65535];
       ++_messages_read;
       int len_get = data->GetCanReadLength();
       _bytes_read += len_get;
       while (data->GetCanReadLength()) {
           int ret = data->Read(buff, 65535);
           handle->Write(buff, ret);
       }
    }

    Client* _owner;
    int64_t _bytes_read;
    int64_t _messages_read;
};

class Client {
public:
    Client(int block_size, int session_count, int timeout, const std::string& ip, int port, cppnet::CCppNet* net) : 
                             _ip(ip), _port(port), _session_count(session_count), _timeout(timeout),
                             _block_size(block_size), _net(net) {
        _num_connected = 0;
        for (int i = 0; i < _block_size; ++i) {
            _message.push_back(static_cast<char>(i % 128));
        }
    }

    void Start() {
        _net->SetTimer(_timeout, std::bind(&Client::HandleTimeout, this, std::placeholders::_1), nullptr);

        for (int i = 0; i < _session_count; ++i) {
            _net->Connection(_ip, _port);
        }
    }

    const std::string& Message() const {
        return _message;
    }

    void OnMessage(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error) {
       if (error == CEC_SUCCESS) {
          auto iter = _sessions.find(handle);
          if (iter != _sessions.end()) {
              iter->second->OnMessage(handle, data, len);
          }

       } else {
          std::cout << " something error while reading. " << std::endl;
       }
    }

    void OnConnect(const Handle& handle, uint32_t error) {
       std::cout << "OnConnect :" << _num_connected.load() << std::endl;
        if (error == CEC_SUCCESS) {
            _num_connected++;
            if (_num_connected.load() == _session_count) {
                std::cout << "all connected" << std::endl;
            }
            auto session = std::unique_ptr<Session>(new Session(this));
            session->OnConnection(handle);
            _sessions[handle] = std::move(session);

        } else {
          std::cout << " something error while connect. error :  " << error << std::endl;
        }
    }

  void OnDisconnect(const Handle&, uint32_t) {
      std::cout << "disconnected :" << _num_connected.load() << std::endl;
      _num_connected--;
      if (_num_connected== 0) {
          std::cout << "all disconnected" << std::endl;

          int64_t totalBytesRead = 0;
          int64_t totalMessagesRead = 0;
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

          delete _net;
      }
  }

private:

    void HandleTimeout(void*) {
        std::cout << "stop" << std::endl;;
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
    cppnet::CCppNet*  _net;
    std::unordered_map<Handle, std::unique_ptr<Session>> _sessions;
};

void Session::OnConnection(const Handle& handle) {
    //SetNoDelay(handle);
    auto msg = _owner->Message();
    handle->Write(msg.c_str(), msg.length());
}

int main(int argc, char* argv[]) {

    if (argc < 7) {
       std::cout << "please input [ip] [port] [thread count] [block size] [session count] [time out]" << std::endl;
       return -1;
    }
    
    std::string ip  = std::string(argv[1]);
    int port        = atoi(argv[2]);
    int threadCount = atoi(argv[3]);
    int block_size  = atoi(argv[4]);
    int session_count = atoi(argv[5]);
    int timeout       = atoi(argv[6]);

    cppnet::CCppNet* net = new cppnet::CCppNet;
    net->Init(threadCount);

    Client client(block_size, session_count, timeout, ip, port, net);

    net->SetConnectionCallback(std::bind(&Client::OnConnect, &client, std::placeholders::_1, std::placeholders::_2));
    net->SetReadCallback(std::bind(&Client::OnMessage, &client, std::placeholders::_1, std::placeholders::_2,
                                   std::placeholders::_3, std::placeholders::_4));
    net->SetDisconnectionCallback(std::bind(&Client::OnDisconnect, &client, std::placeholders::_1, std::placeholders::_2));

    client.Start();

    net->Join();

    return 0;
}

