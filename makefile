.PHONY: build

build: CppNet

INCLUDES=\
		-I. \
		-I../base/ \
		-I../net/ \
		-I../net/linux \
		-I../test \

files=\
		Buffer.o \
		Log.o \
		LoopBuffer.o \
		MemaryPool.o \
		PoolSharedPtr.o \
		TimeTool.o \
		OSInfo.o \
		Timer.o \
		AcceptSocket.o \
		CEpoll.o \
		Socket.o \
		SocketBase.o \
		CppNet.o \

CppNet:$(files)
	g++ $(INCLUDES) $(files) -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
Buffer.o : Buffer.h Buffer.cpp
	g++ $(INCLUDES) -c Buffer.cpp Buffer.h -lpthread -fPIC -m64 -std=c++11 -lstdc++
Log.o : Log.h Log.cpp
	g++ $(INCLUDES) -c Log.h Log.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
LoopBuffer.o : LoopBuffer.h LoopBuffer.cpp
	g++ $(INCLUDES) LoopBuffer.cpp LoopBuffer.h -lpthread -fPIC -m64 -std=c++11 -lstdc++
MemaryPool.o : MemaryPool.h MemaryPool.cpp
	g++ $(INCLUDES) -c MemaryPool.cpp MemaryPool.h -lpthread -fPIC -m64 -std=c++11 -lstdc++
TimeTool.o : TimeTool.h TimeTool.cpp
	g++ $(INCLUDES) -c TimeTool.h TimeTool.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
OSInfo.o : OSInfo.h OSInfo.cpp
	g++ $(INCLUDES) OSInfo.h OSInfo.cpp -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
Timer.o : Timer.h Timer.cpp
	g++ $(INCLUDES) Timer.h Timer.cpp -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
AcceptSocket.o : AcceptSocket.h AcceptSocket.cpp
	g++ $(INCLUDES) AcceptSocket.h AcceptSocket.cpp -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
CEpoll.o : CEpoll.h CEpoll.cpp
	g++ $(INCLUDES) CEpoll.h CEpoll.cpp -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
Socket.o : Socket.h Socket.cpp
	g++ $(INCLUDES) Socket.h Socket.cpp -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
SocketBase.o : SocketBase.h SocketBase.cpp
	g++ $(INCLUDES) SocketBase.h SocketBase.cpp -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++

clean:
	rm -rf *.o *.a