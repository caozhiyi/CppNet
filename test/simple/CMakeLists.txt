add_executable(server CppNetServer.cpp)
target_link_libraries(server PUBLIC cppnet)

add_executable(client CppNetClient.cpp)
target_link_libraries(client PUBLIC cppnet)