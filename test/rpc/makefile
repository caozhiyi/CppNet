SRCS = func_thread.cpp info_router.cpp parse_package.cpp rpc_client.cpp rpc_server.cpp

OBJS = $(patsubst %.cpp, %.o, $(SRCS))
CLI = client.cpp
SER = server.cpp


CC = g++

INCLUDES = -I../../

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