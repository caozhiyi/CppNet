#include <string>
#include <fstream>
#include <string.h> // for memset
#include <iostream>
#include <functional>

#include "md5.h"
#include "common.h"
#include "include/cppnet.h"
#include "common/util/time.h"

using namespace cppnet;

class SendFile {
public:
    SendFile(const std::string& file, cppnet::CppNet* net):
        _block(false),
        _status(hello),
        _file_name(file), 
        _net(net) {
    }

    ~SendFile() {
        _file.close();
    }

    void OnWrite(Handle handle, uint32_t len) {
        if (_block) {
            _block = false;
            Send(handle);
        }
    }

    void OnRecv(Handle handle, std::shared_ptr<cppnet::Buffer> data, uint32_t len) {
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

        if (_status == hello) {
            if (ret == "OK") {
                std::cout << "start to send file ..." << std::endl;
                _status = sending;
                Send(handle);

            } else {
                std::cout << "server refuse recv the file!" << std::endl;
            }

        } else if (_status == sending) {
            if (ret == "OK") {
                std::cout << "send file success!" << std::endl;

            } else {
                std::cout << "something error while sending!" << std::endl;
            }

            _net->Destory();
        }
    }

    void OnConnect(Handle handle, uint32_t err) {
        if (err == CEC_SUCCESS) {
            std::cout << "start to header ..." << std::endl;
            GetFileHeader();
            std::cout << "get file name   : " << _header._name << std::endl;
            std::cout << "get file length : " << _header._length << std::endl;
            std::cout << "get file md5    : " << _header._md5 << std::endl;
            handle->Write((char*)&_header, sizeof(_header));

        } else {
            std::cout << "connect to server failed." << std::endl;
        }
    }

private:
    bool GetFileHeader() {
        _file.open(_file_name, std::ios::binary | std::ios::in);
        if (!_file.good()) {
            return false;
        }

        sprintf(_header._name, "%s", _file_name.c_str());
        _file.seekg(0, _file.end);
        _header._length = (int)_file.tellg();
        _file.seekg(0, _file.beg);

        Compute_file_md5(_file_name.c_str(), _header._md5);
        return true;
    }
    
    void Send(const Handle& handle) {
        char buf[__read_len];
        while (!_file.eof()) {
            _file.read(buf, __read_len);
            int len =  (int)_file.gcount();
            if (!handle->Write(buf, len)) {
                _block = true;
                return;
            }
        }
    }

private:
    std::fstream _file;
    FileHeader   _header;
    STATUS       _status;
    std::string  _file_name;
    bool         _block;
    cppnet::CppNet* _net;
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
    net->SetConnectionCallback(std::bind(&SendFile::OnConnect, &file, std::placeholders::_1, std::placeholders::_2));
    net->SetReadCallback(std::bind(&SendFile::OnRecv, &file, std::placeholders::_1, std::placeholders::_2,
                                      std::placeholders::_3));
    net->SetWriteCallback(std::bind(&SendFile::OnWrite, &file, std::placeholders::_1, std::placeholders::_2));

    net->Connection("127.0.0.1", 8921);
    
    net->Join();

    return 0;
}