project(simpleserver)
add_executable(${PROJECT_NAME} cppnet_server.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()


project(simpleclient)
add_executable(${PROJECT_NAME} cppnet_client.cpp)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

