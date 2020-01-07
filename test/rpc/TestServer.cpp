#include <iostream>
#include "Any.h"
#include "RPCServer.h"

std::vector<base::CAny> Add1(std::vector<base::CAny> param) {
    int res = base::any_cast<int>(param[0])  + base::any_cast<int>(param[1]);
    std::vector<base::CAny> ret;
    ret.push_back(base::CAny(res));
	return ret;
}

int main() {
	CRPCServer server;
	server.Init(4);
	server.RegisterFunc("Add1", "i(ii)", Add1);
	server.Start(8951, "0.0.0.0");
}
