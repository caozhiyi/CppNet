// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <string>
#include <atomic>
#include <vector>
#include <iostream>
#include <algorithm>  // for std::find

#include "include/cppnet.h"
#include "foundation/util/time.h"

std::atomic_bool __stop(false);
int msg_index = 0;
std::vector<cppnet::Handle> handle_vec;
static const char* __buf_spilt = "\r\n";

std::string GetMsg() {
  return "It is a test msg, It is a long test msg. index : "
    + std::to_string(msg_index++) + __buf_spilt;
}

void ReadFunc(cppnet::Handle handle, cppnet::BufferPtr data, uint32_t len) {
  // print
  char buf[1024] = {0};
  data->Read(buf, 1024);
  std::cout << buf << std::endl;
}

void ConnectFunc(cppnet::Handle handle, uint32_t error) {
  if (error != cppnet::CEC_SUCCESS) {
    std::cout << "something err while connect : " << error << std::endl;
  } else {
    handle_vec.push_back(handle);
  }
}

void DisConnectionFunc(cppnet::Handle handle, uint32_t err) {
  __stop = true;
  if (handle_vec.empty()) {
    std::cout << "20 clients all disconnect" << std::endl;
  }
}

int main() {
  cppnet::CppNet net;
  net.Init(1);

  net.SetConnectionCallback(ConnectFunc);
  net.SetReadCallback(ReadFunc);
  net.SetDisconnectionCallback(DisConnectionFunc);
  for (size_t i = 0; i < 200; i++) {
    net.Connection("127.0.0.1", 8921);
  }
  // wait all connect success.
  fdan::Sleep(2000);

  std::cout << "200 clients all connected" << std::endl;

  // sleep 1s;
  for (auto iter = handle_vec.begin(); iter != handle_vec.end() && !__stop; ++iter) {
    fdan::Sleep(1000);
    std::string msg = GetMsg();
    (*iter)->Write(msg.c_str(), (uint32_t)msg.length());
  }

  net.Join();
}
