add_executable(sendfileserver SendFileServer.cpp md5.cpp)
target_link_libraries(sendfileserver PUBLIC cppnet)

add_executable(sendfileclient SendFileClient.cpp md5.cpp)
target_link_libraries(sendfileclient PUBLIC cppnet)