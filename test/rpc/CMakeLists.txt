SET(COMMONSRCS FuncThread.cpp InfoRouter.cpp ParsePackage.cpp RPCClient.cpp RPCServer.cpp)

project(serrpc)
add_executable(${PROJECT_NAME} ${COMMONSRCS} TestServer.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/rpc")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()



project(clirpc)
add_executable(${PROJECT_NAME} ${COMMONSRCS} TestClient.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/rpc")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()

