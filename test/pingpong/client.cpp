// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <mutex>
#include <atomic>
#include <memory>
#include <cstdio>
#include <string>
#include <utility>
#include <iostream>
#include <functional>
#include <unordered_map>

#include "include/cppnet.h"

#ifdef __win__
#include <winsock2.h>
void SetNoDelay(const uint64_t& sock) {
  int opt = 1;
  int ret = setsockopt(sock, SOL_SOCKET, TCP_NODELAY,
    (const char*)&opt, sizeof(opt));
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
  explicit Session(Client* owner):
    owner_(owner),
    bytes_read_(0),
    messages_read_(0) { }

  int64_t BytesRead() const {
     return bytes_read_;
  }

  int64_t MessagesRead() const {
     return messages_read_;
  }

  void OnConnection(cppnet::Handle handle);

  void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data, uint32_t) {
     char buff[65535];
     ++messages_read_;

     uint32_t len_get = data->GetCanReadLength();
     bytes_read_ += len_get;
     while (data->GetCanReadLength()) {
       uint32_t ret = data->Read(buff, 65535);
       handle->Write(buff, ret);
     }
  }

  Client* owner_;
  int64_t bytes_read_;
  int64_t messages_read_;
};

class Client {
 public:
  Client(int block_size, int session_count, int timeout, const std::string& ip,
    int port, cppnet::CppNet* net):
    ip_(ip), port_(port), session_count_(session_count), timeout_(timeout),
    block_size_(block_size), net_(net) {
    num_connected_ = 0;
    for (int i = 0; i < block_size_; ++i) {
      message_.push_back(static_cast<char>(i % 128));
    }
  }

  void Start() {
    net_->AddTimer(timeout_, std::bind(&Client::HandleTimeout,
      this, std::placeholders::_1), nullptr);

    for (int i = 0; i < session_count_; ++i) {
      net_->Connection(ip_, port_);
    }
  }

  const std::string& Message() const {
    return message_;
  }

  void OnMessage(cppnet::Handle handle, cppnet::BufferPtr data,
    uint32_t len) {
    auto session = reinterpret_cast<Session*>(handle->GetContext());
    session->OnMessage(handle, data, len);
  }

  void OnConnect(cppnet::Handle handle, uint32_t error) {
    if (error == cppnet::CEC_SUCCESS) {
      num_connected_++;
      if (num_connected_.load() == session_count_) {
        std::cout << session_count_ << " sessions all connected" << std::endl;
      }
      auto session = new Session(this);
      handle->SetContext(session);
      session->OnConnection(handle);

      std::lock_guard<std::mutex> lock(mutex_);
      sessions_[handle] = session;

    } else {
      std::cout << " something error while connect. error :  "
        << error << std::endl;
    }
  }

  void OnDisconnect(cppnet::Handle, uint32_t) {
    num_connected_.fetch_sub(1);
    if (num_connected_.load() == 0) {
      std::cout << session_count_ << " sessions all disconnected" << std::endl;

      int64_t totalBytesRead = 0;
      int64_t totalMessagesRead = 0;

      for (const auto& session : sessions_) {
        totalBytesRead += session.second->BytesRead();
        totalMessagesRead += session.second->MessagesRead();
      }

      std::cout << totalBytesRead << " total bytes read" << std::endl;
      std::cout << totalMessagesRead << " total messages read" << std::endl;
      std::cout << static_cast<double>(totalBytesRead) /
        static_cast<double>(totalMessagesRead)
          << " average message size" << std::endl;
      std::cout << static_cast<double>(totalBytesRead) /
        ((timeout_ / 1000) * 1024 * 1024)
          << " MiB/s throughput" << std::endl;

      net_->Destory();
    }
  }

 private:
  void HandleTimeout(void*) {
    std::cout << "timeout, to stop connections" << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& session : sessions_) {
      session.first->Close();
    }
  }

  std::string  ip_;
  int      port_;
  int      session_count_;
  int      timeout_;
  int      block_size_;
  std::string     message_;
  std::atomic_uint  num_connected_;
  cppnet::CppNet*   net_;

  std::mutex    mutex_;
  std::unordered_map<cppnet::Handle, Session*> sessions_;
};

void Session::OnConnection(cppnet::Handle handle) {
  SetNoDelay(handle->GetSocket());
  auto msg = owner_->Message();
  handle->Write(msg.c_str(), (uint32_t)msg.length());
}

int main(int argc, char* argv[]) {
  if (argc < 7) {
     std::cout << "please input [ip] [port] [thread count] [block size] [session count] [time out]" << std::endl;
     return -1;
  }

  std::string ip  = std::string(argv[1]);
  int port      = atoi(argv[2]);
  int threadCount   = atoi(argv[3]);
  int block_size  = atoi(argv[4]);
  int session_count = atoi(argv[5]);
  int timeout     = atoi(argv[6]);

  cppnet::CppNet* net = new cppnet::CppNet;
  net->Init(threadCount);

  Client client(block_size, session_count, timeout, ip, port, net);

  net->SetConnectionCallback(std::bind(&Client::OnConnect, &client,
    std::placeholders::_1, std::placeholders::_2));
  net->SetReadCallback(std::bind(&Client::OnMessage, &client,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  net->SetDisconnectionCallback(std::bind(&Client::OnDisconnect, &client,
    std::placeholders::_1, std::placeholders::_2));

  client.Start();

  net->Join();

  delete net;

  return 0;
}

