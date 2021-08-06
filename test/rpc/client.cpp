#include <iostream>

#include "rpc_client.h"
#include "foundation/util/any.h"
#include "foundation/util/time.h"
using namespace std;

void Add1CallBack(int code, std::vector<fdan::Any>& ret) {
    if (code == NO_ERROR) {
        cout << code << "  " << fdan::any_cast<int>(ret[0]) << endl;
    }
}

Call_back func = Add1CallBack;

int main() {
    RPCClient client;
    client.SetCallBack("Add1", func);
    client.Start(8951, "127.0.0.1");
    for (;;) {
        fdan::Sleep(1000);
        client.CallFunc("Add1", 100, 200);
    }
}
