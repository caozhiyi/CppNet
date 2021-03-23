SRCS = $(wildcard ./common/alloter/*.cpp       \
                  ./common/buffer/*.cpp        \
                  ./common/log/*.cpp           \
                  ./common/timer/*.cpp         \
                  ./common/network/*.cpp       \
                  ./common/network/posix/*.cpp \
                  ./common/structure/*.cpp     \
                  ./common/thread/*.cpp        \
                  ./common/util/*.cpp          \
                  ./common/os/*.cpp            \
                  ./common/os/posix/*.cpp      \
                  ./common/util/*.cpp          \
                  ./cppnet/event/*.cpp         \
                  ./cppnet/socket/*.cpp        \
                  ./cppnet/event/linux/*.cpp   \
                  ./cppnet/event/mac/*.cpp     \
                  ./cppnet/*.cpp               \
                  )

OBJS = $(patsubst %.cpp, %.o, $(SRCS))


CC = g++

INCLUDES = -I. 

#debug
CCFLAGS = -lpthread -fPIC -m64 -g -std=c++11 -lstdc++ -pipe 

#CCFLAGS = -lpthread -fPIC -m64 -O2 -std=c++11 -lstdc++ -pipe

TARGET = libcppnet.a

all:$(TARGET)

$(TARGET):$(OBJS)
	ar rcs $@ $^

%.o : %.cpp
	$(CC) -c $< -o $@ $(CCFLAGS) $(INCLUDES) 

clean:
	rm -rf $(OBJS) $(TARGET) $(SERBIN) $(CLIBIN)