#include "socket_interface.h"

namespace cppnet {

thread_local std::unordered_map<uint64_t, std::shared_ptr<Socket>> Socket::__all_socket_map;

}