SER = echo_server.cpp
CLI = echo_client.cpp

CC = g++

INCLUDES = -I../../

#debug
#CCFLAGS = -lpthread -fPIC -m64 -g -std=c++11 -lstdc++ -pipe 

CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe

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