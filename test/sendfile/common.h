#include <string.h> // for memset

enum STATUS {
    hello,
    sending
};

struct FileHeader {
    int  _length;
    char _name[64];
    char _md5[128]; 

    FileHeader() {
        memset(_name, 0, 64);
        memset(_md5, 0, 128);
    }
};

const int __header_len = sizeof(FileHeader);
const int __read_len   = 1024*1024;