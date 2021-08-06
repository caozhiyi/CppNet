#include <iostream>
#include "rpc_server.h"
#include "foundation/util/any.h"

std::vector<fdan::Any> Add1(std::vector<fdan::Any> param) {
    int res = fdan::any_cast<int>(param[0])  + fdan::any_cast<int>(param[1]);
    std::vector<fdan::Any> ret;
    ret.push_back(fdan::Any(res));
    return ret;
}

int main() {
    RPCServer server;
    server.Init(4);
    server.RegisterFunc("Add1", "i(ii)", Add1);
    server.Start(8951, "0.0.0.0");
}
