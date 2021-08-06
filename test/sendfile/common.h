// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)
// Copyright <caozhiyi5@gmail.com>

#include <string.h>  // for memset

#ifndef TEST_SENDFILE_COMMON_H_
#define TEST_SENDFILE_COMMON_H_

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

static const int __header_len = sizeof(FileHeader);
static const int __read_len   = 4096;

#endif  // TEST_SENDFILE_COMMON_H_
