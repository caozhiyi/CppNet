
add_executable(pingpongserver Server.cpp)
target_link_libraries(pingpongserver PUBLIC cppnet)

add_executable(pingpongclient Client.cpp)
target_link_libraries(pingpongclient PUBLIC cppnet)