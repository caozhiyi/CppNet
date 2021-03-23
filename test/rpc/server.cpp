#include <iostream>
#include "rpc_server.h"
#include "common/util/any.h"

std::vector<cppnet::Any> Add1(std::vector<cppnet::Any> param) {
    int res = cppnet::any_cast<int>(param[0])  + cppnet::any_cast<int>(param[1]);
    std::vector<cppnet::Any> ret;
    ret.push_back(cppnet::Any(res));
	return ret;
}

int main() {
	RPCServer server;
	server.Init(4);
	server.RegisterFunc("Add1", "i(ii)", Add1);
	server.Start(8951, "0.0.0.0");
}
