
INCLUDES=\
         -I. \
         -I./base \
         -I./net \
         -I./net/linux \
         -I./test \

files=\
         Buffer.o \
         Log.o \
         LoopBuffer.o \
         MemaryPool.o \
         TimeTool.o \
         OSInfo.o \
         Timer.o \
         AcceptSocket.o \
         CEpoll.o \
         Socket.o \
         SocketBase.o \

CppNet:$(files) ./test/CppNet.cpp
	g++ $(files) $(INCLUDES) ./test/CppNet.cpp -g -o CppNet -lpthread -fPIC -m64 -std=c++11 -lstdc++
	
MemaryPool.o : ./base/MemaryPool.h ./base/MemaryPool.cpp
	g++ $(INCLUDES) -c -g ./base/MemaryPool.cpp ./base/MemaryPool.h -lpthread -fPIC -m64 -std=c++11 -lstdc++
Buffer.o : ./base/Buffer.h ./base/Buffer.cpp
	g++ $(INCLUDES) -c -g ./base/Buffer.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
Log.o : ./base/Log.h ./base/Log.cpp
	g++ $(INCLUDES) -c -g ./base/Log.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
LoopBuffer.o : ./base/LoopBuffer.h ./base/LoopBuffer.cpp MemaryPool.o
	g++ $(INCLUDES) -c -g ./base/LoopBuffer.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
TimeTool.o : ./base/TimeTool.h ./base/TimeTool.cpp
	g++ $(INCLUDES) -c -g ./base/TimeTool.h ./base/TimeTool.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
OSInfo.o : ./net/OSInfo.h ./net/OSInfo.cpp
	g++ $(INCLUDES) -c -g ./net/OSInfo.h ./net/OSInfo.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
Timer.o : ./net/Timer.h ./net/Timer.cpp
	g++ $(INCLUDES) -c -g ./net/Timer.h ./net/Timer.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
AcceptSocket.o : ./net/AcceptSocket.h ./net/linux/AcceptSocket.cpp  ./base/MemaryPool.h ./base/MemaryPool.cpp Log.o 
	g++ $(INCLUDES) -c -g ./net/AcceptSocket.h ./net/linux/AcceptSocket.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
CEpoll.o : ./net/linux/CEpoll.h ./net/linux/CEpoll.cpp Log.o
	g++ $(INCLUDES) -c -g ./net/linux/CEpoll.h ./net/linux/CEpoll.cpp -fpermissive -lpthread -fPIC -m64 -std=c++11 -lstdc++
Socket.o : ./net/Socket.h ./net/linux/Socket.cpp Log.o
	g++ $(INCLUDES) -c -g ./net/Socket.h ./net/linux/Socket.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++
SocketBase.o : ./net/SocketBase.h ./net/linux/SocketBase.cpp
	g++ $(INCLUDES) -c -g ./net/SocketBase.h ./net/linux/SocketBase.cpp -lpthread -fPIC -m64 -std=c++11 -lstdc++

clean:
	rm -rf *.o *.a