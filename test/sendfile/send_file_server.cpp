// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <map>
#include <string>
#include <memory>
#include <fstream>
#include <cstring>
#include <iostream>
#include <string.h>  // for strlen

#include "md5.h"
#include "common.h"
#include "include/cppnet.h"

struct Content {
  FileHeader head;
  STATUS   status;
  Content() {
    status = hello;
    file = nullptr;
  }
  ~Content() {
    if (file) {
      delete file;
    }
  }
  std::fstream* file;
};

std::map<cppnet::Handle, Content> _recv_map;

void WriteFunc(cppnet::Handle handle, uint32_t len) {
  // std::cout << "something err while write : " << std::endl;
}

void ReadFunc(cppnet::Handle handle, std::shared_ptr<cppnet::Buffer> data,
  uint32_t len) {
  auto iter = _recv_map.find(handle);
  if (iter == _recv_map.end()) {
    std::cout << "can't find handle" << std::endl;
    return;
  }
  if (iter->second.status == hello) {
    if (data->GetCanReadLength() < __header_len) {
      std::cout << "continue recv ... " << std::endl;
      return;
    }

    data->Read((char*)&(iter->second.head), __header_len);
    strncat(iter->second.head._name, ".bk", sizeof(".bk"));
    iter->second.file = new std::fstream(iter->second.head._name,
      std::ios::out | std::ios::binary);

    std::cout << "get file name   : " << iter->second.head._name << std::endl;
    std::cout << "get file length : " << iter->second.head._length << std::endl;
    std::cout << "get file md5  : " << iter->second.head._md5 << std::endl;
    if (*(iter->second.file)) {
      handle->Write("OK", (uint32_t)strlen("OK"));
      std::cout << "start to recv. " << std::endl;
      iter->second.status = sending;

    } else {
      handle->Write("NO", (uint32_t)strlen("NO"));
      std::cout << "refuse to recv. " << std::endl;
    }

  } else if (iter->second.status == sending) {
    while (data->GetCanReadLength() > 0) {
      char buf[__read_len];
      int len = data->Read(buf, __read_len);
      iter->second.head._length -= len;
      iter->second.file->write(buf, len);
      if (iter->second.head._length <= 0) {
        std::cout << "recv end. " << std::endl;
        iter->second.file->sync();
        iter->second.file->close();
        char md5_buf[128] = { 0 };
        Compute_file_md5(iter->second.head._name, md5_buf);
        if (strcmp(md5_buf, iter->second.head._md5) == 0) {
          handle->Write("OK", (uint32_t)strlen("OK"));
          std::cout << "recv ok. " << std::endl;

        } else {
          handle->Write("NO", (uint32_t)strlen("NO"));
          std::cout << "recv failed. " << std::endl;
        }
      }
    }
  }
}

void ConnectFunc(cppnet::Handle handle, uint32_t error) {
  if (error != cppnet::CEC_SUCCESS) {
    std::cout << "something err while connect : " << error << std::endl;
  }
  if (_recv_map.find(handle) == _recv_map.end()) {
    _recv_map[handle] = Content();
  }
}

void DisConnectFunc(cppnet::Handle handle, uint32_t error) {
  auto iter = _recv_map.find(handle);
  if (iter != _recv_map.end()) {
    _recv_map.erase(iter);
  }
}

int main() {
  cppnet::CppNet net;
  net.Init(1);

  net.SetAcceptCallback(ConnectFunc);
  net.SetWriteCallback(WriteFunc);
  net.SetReadCallback(ReadFunc);
  net.SetDisconnectionCallback(DisConnectFunc);

  net.ListenAndAccept("0.0.0.0", 8921);

  net.Join();
}
