// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#ifndef CPPNET_CPPNET_CONFIGE
#define CPPNET_CPPNET_CONFIGE

#include <cstdint>

namespace cppnet {

// on linux, we have two ways to avoid thundering herd:
// 1. use tcp flag set reuse port flag, mulit socket listen to one same port.
// 2. only use one socket but set EPOLLEXCLUSIVE when add to epoll.(require linux kernal > 4.5)

// use reuse port tcp flag. must set true on windows and macOS.
static const bool __reuse_port             = true;
// use EPOLLEXCLUSIVE flag. must set false on windows and macOS.
// if __epoll_exclusive set ture on linux, please set __reuse_port false.
static const bool __epoll_exclusive        = false;

// size of block memory in block memory pool.
static const uint16_t __mem_block_size     = 1024;
// how many block memory will be add to block memory pool.
static const uint16_t __mem_block_add_step = 5;
// max number of blocks in memory pool. If block memory more than this number, will reduce to half.
static const uint16_t __max_block_num      = 10;
// max data to write when net is busy.
static const uint32_t __max_write_cache    = 1024 * 1024 * 4;

// address buffer length in socket.
static const uint16_t __addr_str_len       = 16;
// log level. 
static const uint16_t __log_level          = 15; // info level
// log file name.
static const char* __log_file_name         = "cppnet_log";
// open log print.
static const bool __open_log               = false;

// epoll use et model.
static const bool __epoll_use_et                   = true;
// the start extend size of read buff while buff is't enough. 
static const uint16_t __linux_read_buff_expand_len = 4096;
// max extend size of read buff while buff is't enough. 
static const uint32_t __linux_read_buff_expand_max = 65536;
// max size of buffer will get from buffer. Be careful IOV_MAX.
static const uint16_t __linux_write_buff_get       = 4096;

// iocp buffer length.
static const uint16_t __iocp_buff_size = 1024*4;
// number of iocp accept event post when first call accept
static const uint16_t __iocp_accept_event_num = 20;


}

#endif
