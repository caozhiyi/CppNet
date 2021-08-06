// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <string>
#include <vector>
#include <iostream>

#include "include/cppnet.h"
#include "foundation/util/time.h"

void ConnectFunc(cppnet::Handle handle, uint32_t err) {
  if (err == cppnet::CEC_SUCCESS) {
    std::string msg("it is a test message.");
    handle->Write(msg.c_str(), (uint32_t)msg.length());
    handle->Close();

  } else {
    std::cout << " [ConnectFunc] some thing error : " << err << std::endl;
  }
}


int main() {
  cppnet::CppNet net;
  net.Init(1);
  net.SetConnectionCallback(ConnectFunc);

  net.Connection("127.0.0.1", 8921);
  net.Connection("127.0.0.1", 8922);

  fdan::Sleep(2000);
}
