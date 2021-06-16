SRCS = $(wildcard ./*.cpp)

OBJS = $(patsubst %.cpp, %.o, $(SRCS))

CC = g++

INCLUDES = -I../../

#debug
#CCFLAGS = -lpthread -fPIC -m64 -g -std=c++11 -lstdc++ -pipe 

CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe

TARGET = ../../libcppnet.a
SERBIN = httpser

all: $(SERBIN)

$(SERBIN):$(OBJS) $(TARGET)
	$(CC) $^ -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(SERBIN)