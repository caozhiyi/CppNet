SRCS = $(wildcard ./*.cpp)

OBJS = $(patsubst %.cpp, %.o, $(SRCS))

CC = g++

INCLUDES = -I.               \
           -I../../base      \
           -I../../include   \

CCFLAGS = -lpthread -fPIC -m64 -g -std=c++11 -lstdc++

TARGET = ../../libcppnet.a
SERBIN = httpser

all: $(SERBIN)

$(SERBIN):$(OBJS) $(TARGET)
	$(CC) $^ -o $@  $(TARGET)  $(CCFLAGS) $(INCLUDES)

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(SERBIN)