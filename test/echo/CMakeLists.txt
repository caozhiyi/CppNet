link_directories(${CMAKE_SOURCE_DIR}/../../)

if(UNIX)
	link_libraries(libcppnet.a)
	link_libraries(pthread)
else()
	link_libraries(cppnet.lib)
endif()

add_executable(echoclient EchoClient.cpp)
add_executable(echoserver EchoServer.cpp)

include_directories( 
PUBLIC ${CMAKE_SOURCE_DIR}/../../base
PUBLIC ${CMAKE_SOURCE_DIR}/../../include
)