link_directories(${CMAKE_SOURCE_DIR}/../../)

if(UNIX)
	link_libraries(libcppnet.a)
	link_libraries(pthread)
else()
	link_libraries(cppnet.lib)
endif()

SET(SRCS FuncThread.cpp InfoRouter.cpp ParsePackage.cpp RPCClient.cpp RPCServer.cpp)

add_executable(serrpc ${SRCS} TestServer.cpp)
add_executable(clirpc ${SRCS} TestClient.cpp)

include_directories( 
PUBLIC ${CMAKE_SOURCE_DIR}/../../base
PUBLIC ${CMAKE_SOURCE_DIR}/../../include
)