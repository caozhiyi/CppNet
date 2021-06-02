cmake_minimum_required(VERSION 3.10)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} http_source)

project(httpser)
add_executable(${PROJECT_NAME} ${http_source})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} ws2_32)
else()
    target_link_libraries(${PROJECT_NAME} cppnet)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()
