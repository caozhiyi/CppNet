cmake_minimum_required(VERSION 3.10)

project(echoclient)
add_executable(${PROJECT_NAME} echo_client.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()


project(echoserver)
add_executable(${PROJECT_NAME} echo_server.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

