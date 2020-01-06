project(server)
add_executable(${PROJECT_NAME} CppNetServer.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/simple")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()



project(client)
add_executable(${PROJECT_NAME} CppNetClient.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/simple")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()

