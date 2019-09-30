SRCS = FuncThread.cpp InfoRouter.cpp ParsePackage.cpp RPCClient.cpp RPCServer.cpp

OBJS = $(patsubst %.cpp, %.o, $(SRCS))
CLI = TestClient.cpp
SER = TestServer.cpp


CC = g++

INCLUDES = -I.               \
           -I../../base      \
           -I../../include   \

#debug
#CCFLAGS = -lpthread -fPIC -m64 -g -pg -std=c++11 -lstdc++ -pipe 

CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe -march=corei7 

TARGET = ../../libcppnet.a
SERBIN = serrpc
CLIBIN = clirpc

all: $(SERBIN) $(CLIBIN)

$(SERBIN):$(OBJS) $(TARGET) $(SER)
	$(CC) $^ -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

$(CLIBIN):$(OBJS) $(TARGET) $(CLI)
	$(CC) $^ -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(SERBIN) $(CLIBIN)