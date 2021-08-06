// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <memory>
#include <string>
#include <fstream>
#include <cstring>  // for memset
#include <iostream>
#include <functional>

#include "md5.h"
#include "common.h"
#include "include/cppnet.h"
#include "foundation/util/time.h"

class SendFile {
 public:
  SendFile(const std::string& file, cppnet::CppNet* net):
    block_(false),
    status_(hello),
    file_name_(file),
    net_(net) {
  }

  ~SendFile() {
    file_.close();
  }

  void OnWrite(cppnet::Handle handle, uint32_t len) {
    if (block_) {
      block_ = false;
      Send(handle);
    }
  }

  void OnRecv(cppnet::Handle handle, std::shared_ptr<cppnet::Buffer> data,
    uint32_t len) {
    char ret_char[4] = {0};
    if (data->GetCanReadLength() >= 2) {
      data->Read(ret_char, 4);

    } else {
      std::cout << "3" << std::endl;
      return;
    }
    std::cout << "4" << std::endl;
    std::string ret(ret_char);
    std::cout << "recv from server : " << ret << std::endl;

    if (status_ == hello) {
      if (ret == "OK") {
        std::cout << "start to send file ..." << std::endl;
        status_ = sending;
        Send(handle);

      } else {
        std::cout << "server refuse recv the file!" << std::endl;
      }

    } else if (status_ == sending) {
      if (ret == "OK") {
        std::cout << "send file success!" << std::endl;

      } else {
        std::cout << "something error while sending!" << std::endl;
      }

      net_->Destory();
    }
  }

  void OnConnect(cppnet::Handle handle, uint32_t err) {
    if (err == cppnet::CEC_SUCCESS) {
      std::cout << "start to header ..." << std::endl;
      GetFileHeader();
      std::cout << "get file name   : " << header_._name << std::endl;
      std::cout << "get file length : " << header_._length << std::endl;
      std::cout << "get file md5  : " << header_._md5 << std::endl;
      handle->Write((char*)&header_, sizeof(header_));

    } else {
      std::cout << "connect to server failed." << std::endl;
    }
  }

 private:
  bool GetFileHeader() {
    file_.open(file_name_, std::ios::binary | std::ios::in);
    if (!file_.good()) {
      return false;
    }

    sprintf(header_._name, "%s", file_name_.c_str());
    file_.seekg(0, file_.end);
    header_._length = (int)file_.tellg();
    file_.seekg(0, file_.beg);

    Compute_file_md5(file_name_.c_str(), header_._md5);
    return true;
  }

  void Send(const cppnet::Handle& handle) {
    char buf[__read_len];
    while (!file_.eof()) {
      file_.read(buf, __read_len);
      int len =  (int)file_.gcount();
      if (!handle->Write(buf, len)) {
        block_ = true;
        return;
      }
    }
  }

 private:
  std::fstream    file_;
  FileHeader      header_;
  STATUS          status_;
  std::string     file_name_;
  bool            block_;
  cppnet::CppNet* net_;
};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "must input a file." << std::endl;
    return 1;
  }

  std::string file_name = argv[1];
  cppnet::CppNet* net(new cppnet::CppNet());
  SendFile file(file_name, net);

  net->Init(1);
  net->SetConnectionCallback(std::bind(&SendFile::OnConnect, &file,
    std::placeholders::_1, std::placeholders::_2));
  net->SetReadCallback(std::bind(&SendFile::OnRecv, &file,
    std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  net->SetWriteCallback(std::bind(&SendFile::OnWrite, &file,
    std::placeholders::_1, std::placeholders::_2));

  net->Connection("127.0.0.1", 8921);
  net->Join();
  return 0;
}
