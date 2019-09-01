SRCS = $(wildcard ./base/*.cpp ./net/*.cpp ./net/linux/*.cpp)

OBJS = $(patsubst %.cpp, %.o, $(SRCS))

SER = CppNetSev/CppNetServer.cpp
CLI = CppNetCli/CppNetClient.cpp

CC = g++

INCLUDES = -I.           \
           -I./base      \
           -I./net       \
           -I./net/linux \
           -I./include   \

#debug
#CCFLAGS = -lpthread -fPIC -m64 -g -pg -std=c++11 -lstdc++ -pipe 

CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe -march=corei7 

TARGET = libcppnet.a
SERBIN = cppnetser
CLIBIN = cppnetcli

all:$(TARGET) $(SERBIN) $(CLIBIN)

$(SERBIN):$(SER) $(TARGET)
	$(CC) $(SER) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

$(CLIBIN):$(CLI) $(TARGET)
	$(CC) $(CLI) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

$(TARGET):$(OBJS)
	ar rcs $@ $^

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(TARGET) $(SERBIN) $(CLIBIN)