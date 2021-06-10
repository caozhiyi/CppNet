#ifndef TEST_RPC_COMMONSTRUCT_HEADER
#define TEST_RPC_COMMONSTRUCT_HEADER

#include <vector>
#include <string>
#include <thread>

#include "common/util/any.h"
#include "include/cppnet_type.h"
#include "include/cppnet_socket.h"

enum MessageType {
    FUNCTION_CALL    = 0x01,    //client call functnion request 
    FUNCTION_RET    = 0x02, //server return functnion response
    FUNCTION_INFO    = 0x04  //server notice functnion info
};

enum ERROR_CODE {
    NO_ERROR            = 0,
    PARAM_TYPE_ERROR    = 1,
    PARAM_NUM_ERROR        = 2,
    NO_FUNC_ERROR        = 3,
    PARSE_FUNC_ERROR    = 4
};

struct FuncCallInfo {
    std::string                    _func_name;
    std::vector<cppnet::Any>    _func_param_ret;

    cppnet::Handle              _socket;
};


typedef std::function<std::vector<cppnet::Any>(std::vector<cppnet::Any>)> CommonFunc;
#endif
