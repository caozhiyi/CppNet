#include <iostream>

#include "rpc_client.h"
#include "common/util/any.h"
#include "common/util/time.h"
using namespace std;

void Add1CallBack(int code, std::vector<cppnet::Any>& ret) {
    if (code == NO_ERROR) {
        cout << code << "  " << cppnet::any_cast<int>(ret[0]) << endl;
    }
}

Call_back func = Add1CallBack;

int main() {
    RPCClient client;
    client.SetCallBack("Add1", func);
    client.Start(8951, "127.0.0.1");
    for (;;) {
        cppnet::Sleep(1000);
        client.CallFunc("Add1", 100, 200);
    }
}
