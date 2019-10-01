#ifndef HEADER_NET_CNCONFIGE
#define HEADER_NET_CNCONFIGE

#include "Type.h"
#include "Log.h"

namespace cppnet {

// size of block memory in block memory pool.
static const uint16_t __mem_block_size     = 1024;
// how many block memory will be add to block memory pool.
static const uint16_t __mem_block_add_step = 5;
// max number of blocks in memory pool. If block memory more than this number, will reduce to half.
 static const uint16_t __max_block_num     = 10;

// address buffer length in socket.
static const uint16_t __addr_str_len       = 16;
// log level. 
static const base::LogLevel __log_level    = base::LOG_DEBUG_LEVEL;
// log file name .
static const std::string __log_file_name   = "CppNetLog";
// open log print.
static const bool __open_log               = false;

#ifdef __linux__

// every thread has a epoll handle.
static const bool __per_handle_thread              = true;
// the start extend size of read buff while buff is't enough. 
static const uint16_t __linux_read_buff_expand_len = 4096;
// max extend size of read buff while buff is't enough. 
static const uint32_t __linux_read_buff_expand_max = 65536;
// max size of buffer will get from buffer. Be careful IOV_MAX.
static const uint16_t __linux_write_buff_get       = 4096;

#else

// iocp buffer length.
static const uint16_t __iocp_buff_size = 8192;

#endif

}

#endif
