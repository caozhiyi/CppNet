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

# Automated self-verifying echo test: server + 8 clients in one process,
# byte-exact verification of 1K-64K payloads, watchdog + exit code.
project(echotest)
add_executable(${PROJECT_NAME} echo_test.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

