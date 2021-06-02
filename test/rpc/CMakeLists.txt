cmake_minimum_required(VERSION 3.10)

SET(common_source func_thread.cpp info_router.cpp parse_package.cpp rpc_client.cpp rpc_server.cpp)

project(rpcserver)
add_executable(${PROJECT_NAME} ${common_source} server.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()


project(rpcclient)
add_executable(${PROJECT_NAME} ${common_source} client.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

