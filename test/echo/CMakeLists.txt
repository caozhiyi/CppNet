add_executable(echoclient EchoClient.cpp)
target_link_libraries(echoclient PUBLIC cppnet)


add_executable(echoserver EchoServer.cpp)
target_link_libraries(echoserver PUBLIC cppnet)

