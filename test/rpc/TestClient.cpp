#include <iostream>
#include "RPCClient.h"
#include "Any.h"
#include "Runnable.h"
using namespace std;

void Add1CallBack(int code, std::vector<base::CAny>& ret) {
	if (code == NO_ERROR) {
        cout << code << "  " << base::any_cast<int>(ret[0]) << endl;
	}
}

Call_back func = Add1CallBack;

int main() {
	CRPCClient client;
    client.SetCallBack("Add1", func);
    client.Start(8951, "127.0.0.1");
	for (;;) {
        base::CRunnable::Sleep(1000);
        client.CallFunc("Add1", 100, 200);
	}
}
