SRCS = $(wildcard ./base/*.cpp ./net/*.cpp ./net/linux/*.cpp ./test/*.cpp)

OBJS = $(SRCS:.cpp = .o)

CC = g++

INCLUDES = -I. \
           -I./base \
           -I./net \
           -I./net/linux \
           -I./test \

LIBS = -L../lib

CCFLAGS = -lpthread -fPIC -m64 -std=c++11 -lstdc++ -fpermissive

OUTPUT = CppNet.out

all:$(OUTPUT)

$(OUTPUT) : $(OBJS)
	$(CC) $^ -o $@ $(INCLUDES) $(LIBS) $(CCFLAGS)

%.o : %.c
	$(CC) -c $< $(CCFLAGS)

clean:
	rm -rf *.out *.o

.PHONY:clean