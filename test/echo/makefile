SER = EchoServer.cpp
CLI = EchoClient.cpp

CC = g++

INCLUDES = -I../../base      \
           -I../../include   \

CCFLAGS = -lpthread -fPIC -m64 -g -std=c++11 -lstdc++

TARGET = ../../libcppnet.a
SERBIN = echoserver
CLIBIN = echoclient

all:$(SERBIN) $(CLIBIN)

$(SERBIN):$(SER)
	$(CC) $(SER) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

$(CLIBIN):$(CLI)
	$(CC) $(CLI) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

clean:
	rm -rf $(SERBIN) $(CLIBIN)