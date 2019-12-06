SET(SRCS FuncThread.cpp InfoRouter.cpp ParsePackage.cpp RPCClient.cpp RPCServer.cpp)

add_executable(serrpc ${SRCS} TestServer.cpp)
target_link_libraries(serrpc PUBLIC cppnet)

add_executable(clirpc ${SRCS} TestClient.cpp)
target_link_libraries(clirpc PUBLIC cppnet)