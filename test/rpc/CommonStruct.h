#ifndef TEST_RPC_COMMONSTRUCT_HEADER
#define TEST_RPC_COMMONSTRUCT_HEADER

#include <vector>
#include <string>
#include <thread>

#include "Any.h"
#include "Socket.h"

enum MessageType {
	FUNCTION_CALL	= 0x01,	//client call functnion request 
	FUNCTION_RET	= 0x02, //server return functnion response
	FUNCTION_INFO	= 0x04  //server notice functnion info
};

enum ERROR_CODE {
	NO_ERROR			= 0,
	PARAM_TYPE_ERROR	= 1,
	PARAM_NUM_ERROR		= 2,
	NO_FUNC_ERROR		= 3,
	PARSE_FUNC_ERROR	= 4
};

struct FuncCallInfo {
    std::string				    _func_name;
    std::vector<base::CAny>		_func_param_ret;

    cppnet::Handle              _socket;
};


typedef std::function<std::vector<base::CAny>(std::vector<base::CAny>)> CommonFunc;
#endif
