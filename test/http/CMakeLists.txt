aux_source_directory(${CMAKE_CURRENT_LIST_DIR} HTTPSOURCE)

project(httpser)
add_executable(${PROJECT_NAME} ${HTTPSOURCE})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/http")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()





