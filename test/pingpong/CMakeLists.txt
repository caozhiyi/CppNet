project(pingpongserver)
add_executable(${PROJECT_NAME} Server.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/pingpong")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()



project(pingpongclient)
add_executable(${PROJECT_NAME} Client.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test/pingpong")
if(UNIX)
    target_link_libraries(${PROJECT_NAME} libcppnet.a)
    target_link_libraries(${PROJECT_NAME} pthread)
else()
    target_link_libraries(${PROJECT_NAME} ws2_32)
    target_link_libraries(${PROJECT_NAME} cppnet)
endif()





# link_directories(${CMAKE_SOURCE_DIR}/../../)
# if(UNIX)
# 	link_libraries(libcppnet.a)
# 	link_libraries(pthread)
# else()
# 	link_libraries(cppnet.lib)
# endif()

# add_executable(pingpongserver Server.cpp)
# add_executable(pingpongclient Client.cpp)

# include_directories(
# PUBLIC ${CMAKE_SOURCE_DIR}/../../base
# PUBLIC ${CMAKE_SOURCE_DIR}/../../include
# )
