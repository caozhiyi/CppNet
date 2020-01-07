project(sendfileserver)
add_executable(${PROJECT_NAME} SendFileServer.cpp md5.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/sendfile")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()



project(sendfileclient)
add_executable(${PROJECT_NAME} SendFileClient.cpp md5.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/sendfile")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()

