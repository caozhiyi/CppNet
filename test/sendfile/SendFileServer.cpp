#include <iostream>
#include <string>
#include <string.h>
#include <map>
#include <fstream>
#include <string.h> // for strlen
#include "CppNet.h"
#include "Common.h"
#include "md5.h"

using namespace cppnet;

struct Content {
    FileHeader _head;
    STATUS     _status;
    Content() {
        _status = hello;
        _file = nullptr;
    }
    ~Content() {
        if (_file) {
            delete _file;
        }
    }
    std::fstream* _file;
};

std::map<Handle, Content> _recv_map;

void WriteFunc(const Handle& handle, uint32_t len, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while write : " << error << std::endl;
    }
    // do nothing 
}

void ReadFunc(const Handle& handle, base::CBuffer* data, uint32_t len, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while read : " << error << std::endl;

    } else {
        auto iter = _recv_map.find(handle);
        if (iter == _recv_map.end()) {
            std::cout << "can't find handle" << std::endl;
            return;
        }
        if (iter->second._status == hello) {
            if (data->GetCanReadLength() < __header_len) {
                std::cout << "continue recv ... " << std::endl;
                return;
            }

            data->Read((char*)&(iter->second._head), __header_len);

            strncat(iter->second._head._name, ".bk", sizeof(".bk"));

            iter->second._file = new std::fstream(iter->second._head._name, std::ios::out | std::ios::binary);

            std::cout << "get file name   : " << iter->second._head._name << std::endl;
            std::cout << "get file length : " << iter->second._head._length << std::endl;
            std::cout << "get file md5    : " << iter->second._head._md5 << std::endl;
            if (*(iter->second._file)) {
                Write(handle, "OK", strlen("OK"));
                std::cout << "start to recv. " << std::endl;
                iter->second._status = sending;

            } else {
                Write(handle, "NO", strlen("NO"));
                std::cout << "refuse to recv. " << std::endl;
            }

        } else if (iter->second._status == sending) {
            while (data->GetCanReadLength() > 0) {
                char buf[__read_len];
                int len = data->Read(buf, __read_len);
                iter->second._head._length -= len;
                iter->second._file->write(buf, len);
                if (iter->second._head._length <= 0) {
                    std::cout << "recv end. " << std::endl;
                    iter->second._file->sync();
                    iter->second._file->close();
                    char md5_buf[128] = { 0 };
                    Compute_file_md5(iter->second._head._name, md5_buf);
                    if (strcmp(md5_buf, iter->second._head._md5) == 0) {
                        Write(handle, "OK", strlen("OK"));
                        std::cout << "recv ok. " << std::endl;

                    } else {
                        Write(handle, "NO", strlen("NO"));
                        std::cout << "recv failed. " << std::endl;
                    }
                }
            }
        }
    }
}

void ConnectFunc(const Handle& handle, uint32_t error) {
    if (error != CEC_SUCCESS) {
        std::cout << "something err while connect : " << error << std::endl;
    }
    if (_recv_map.find(handle) == _recv_map.end()) {
        _recv_map[handle] = Content();
    }
}

void DisConnectFunc(const Handle& handle, uint32_t error) {
    auto iter = _recv_map.find(handle);
    if (iter != _recv_map.end()) {
        _recv_map.erase(iter);
    }
}

int main() {

    cppnet::Init(1);

    cppnet::SetAcceptCallback(ConnectFunc);
    cppnet::SetWriteCallback(WriteFunc);
    cppnet::SetReadCallback(ReadFunc);
    cppnet::SetDisconnectionCallback(DisConnectFunc);

    cppnet::ListenAndAccept("0.0.0.0", 8921);

    cppnet::Join();
}