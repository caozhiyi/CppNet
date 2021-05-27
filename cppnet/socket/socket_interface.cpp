// Use of this source code is governed by a BSD 3-Clause License
// that can be found in the LICENSE file.

// Author: caozhiyi (caozhiyi5@gmail.com)

#include "socket_interface.h"

namespace cppnet {

#ifdef __win__
ThreadSafeUnorderedMap<uint64_t, std::shared_ptr<Socket>> Socket::__all_socket_map;
#else
thread_local std::unordered_map<uint64_t, std::shared_ptr<Socket>> Socket::__all_socket_map;
#endif

}