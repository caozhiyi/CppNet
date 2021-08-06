// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <string>
#include <thread>
#include <cstring>  // for strlen
#include <iostream>

#include "include/cppnet.h"

static const int __buf_len = 2048;
static const char* __buf_spilt = "\r\n";

void ReadFunc(cppnet::Handle handle, cppnet::BufferPtr data, uint32_t len) {
  char msg_buf[__buf_len] = {0};
  uint32_t need_len = 0;
  uint32_t find_len = (uint32_t)strlen(__buf_spilt);
  // get recv data to send back.
  uint32_t size = data->ReadUntil(msg_buf, __buf_len,
    __buf_spilt, find_len, need_len);
  handle->Write(msg_buf, size);
}

void ConnectFunc(cppnet::Handle handle, uint32_t error) {
  if (error != cppnet::CEC_SUCCESS) {
    std::cout << "something err while connect : " << error << std::endl;
  }
}

int main() {
  // start 4 threads
  cppnet::CppNet net;
  net.Init(4);

  net.SetAcceptCallback(ConnectFunc);
  net.SetReadCallback(ReadFunc);
  net.SetDisconnectionCallback(ConnectFunc);

  net.ListenAndAccept("0.0.0.0", 8921);

  net.Join();
}
