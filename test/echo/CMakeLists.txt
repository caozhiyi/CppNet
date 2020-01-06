project(echoclient)
add_executable(${PROJECT_NAME} EchoClient.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/echo")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()



project(echoserver)
add_executable(${PROJECT_NAME} EchoServer.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/echo")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()

