cmake_minimum_required(VERSION 3.10)
project(cppnet CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(EXECUTABLE_OUTPUT_PATH         ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build)
set(CMAKE_PDB_OUTPUT_DIRECTORY     ${CMAKE_HOME_DIRECTORY}/build)

aux_source_directory(${CMAKE_SOURCE_DIR}/base BASESOURCE)
aux_source_directory(${CMAKE_SOURCE_DIR}/net NETSOURCE)
if(UNIX)
    aux_source_directory(${CMAKE_SOURCE_DIR}/net/linux NETLINUXSOURCE)
else()
    aux_source_directory(${CMAKE_SOURCE_DIR}/net/win NETLINUXSOURCE)
endif()

add_library(cppnet STATIC ${BASESOURCE} ${NETSOURCE} ${NETLINUXSOURCE})

target_include_directories(cppnet PUBLIC ${CMAKE_SOURCE_DIR}/base)
target_include_directories(cppnet PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(cppnet PUBLIC ${CMAKE_SOURCE_DIR}/net)

if(UNIX)
target_include_directories(cppnet PUBLIC ${CMAKE_SOURCE_DIR}/net/linux)
target_compile_options(cppnet PUBLIC -lstdc++ -pipe -fPIC -m64 -O2 )
target_link_libraries(cppnet PUBLIC pthread )
else()
target_include_directories(cppnet PUBLIC ${CMAKE_SOURCE_DIR}/net/win)
target_compile_options(cppnet PUBLIC /Oy- /W1)
target_compile_definitions(cppnet PUBLIC -D_CRT_SECURE_NO_WARNINGS -D_AMD64_)
target_link_libraries(${PROJECT_NAME} ws2_32)
endif()


OPTION(BUILD_TEST "" on)
if(${BUILD_TEST})
    add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()
