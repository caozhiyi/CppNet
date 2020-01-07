#include <string>
#include <fstream>
#include <string.h> // for memset
#include <iostream>

#include "md5.h"
#include "CppNet.h"
#include "Socket.h"
#include "Common.h"

using namespace cppnet;

class CSendFile {
public:
    CSendFile(const std::string& file) : _status(hello),
                                         _file_name(file) {
    }

    ~CSendFile() {

    }

    void OnRecv(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t err) {
        if (err != CEC_SUCCESS) {
            std::cout << "something error while read." << std::endl;
            return;
        }
        
        char ret_char[4] = {0};
        if (data->GetCanReadLength() >= 2) {
            data->Read(ret_char, 4);

        } else {
            return;
        }
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

            Close(handle);
            Dealloc();
        }
    }

    void OnConnect(const Handle& handle, uint32_t err) {
        if (err == CEC_SUCCESS) {
            std::cout << "start to header ..." << std::endl;
            GetFileHeader();
            std::cout << "get file name   : " << _header._name << std::endl;
            std::cout << "get file length : " << _header._length << std::endl;
            std::cout << "get file md5    : " << _header._md5 << std::endl;
            Write(handle, (char*)&_header, sizeof(_header));

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
        _header._length = _file.tellg();
        _file.seekg(0, _file.beg);

        Compute_file_md5(_file_name.c_str(), _header._md5);
        return true;
    }
    
    void Send(const Handle& handle) {
        char buf[__read_len];
        while (!_file.eof()) {
            _file.read(buf, __read_len);
            int len =  _file.gcount();
            cppnet::Write(handle, buf, len);
        }
        _file.close();
    }

private:
    std::fstream _file;
    FileHeader   _header;
    STATUS       _status;
    std::string  _file_name;
};

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cout << "must input a file." << std::endl;
        return 1;
    }

    std::string file_name = argv[1];
    CSendFile file(file_name);

    cppnet::Init(1);
    cppnet::SetConnectionCallback(std::bind(&CSendFile::OnConnect, &file, std::placeholders::_1, std::placeholders::_2));
    cppnet::SetReadCallback(std::bind(&CSendFile::OnRecv, &file, std::placeholders::_1, std::placeholders::_2,
                                      std::placeholders::_3, std::placeholders::_4));

    cppnet::Connection("127.0.0.1", 8921);
    
    cppnet::Join();
    return 0;
}