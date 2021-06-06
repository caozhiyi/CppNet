SER = multi_port_server.cpp
CLI = multi_port_client.cpp

CC = g++

INCLUDES = -I../../

#debug -pg
CCFLAGS = -lpthread -fPIC -m64 -g -std=c++11 -lstdc++ -pipe

#CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe -march=corei7

TARGET = ../../libcppnet.a
SERBIN = multi_port_server
CLIBIN = multi_port_client

all:$(SERBIN) $(CLIBIN)

$(SERBIN):$(SER) $(TARGET)
	$(CC) $(SER) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

$(CLIBIN):$(CLI) $(TARGET)
	$(CC) $(CLI) -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

clean:
	rm -rf $(SERBIN) $(CLIBIN)
