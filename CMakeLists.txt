cmake_minimum_required(VERSION 3.10)

project(cppnet CXX)

set(FOUNDATION_DIR ${CMAKE_SOURCE_DIR}/foundation/cmake)

message(${FOUNDATION_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CXX_FLAGS
 -O2
 -std=c++11
 -rdynamic
 -lpthread
 -fPIC
 -lstdc++
 -pipe
 )

if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
else()
  list(APPEND CXX_FLAGS "-m64")
endif()

if (WIN32)
add_definitions(-D__win__)
add_definitions(-D__use_iocp__)
endif ()

# output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# include
include_directories(${CMAKE_SOURCE_DIR})

find_package(FOUNDATION REQUIRED)

if(FOUNDATION_FOUND)
    add_subdirectory(foundation)
    include_directories(${FOUNDATION_INCLUDE_DIR})
else()
    message("not found foundation")
endif(FOUNDATION_FOUND)

add_subdirectory(cppnet)
add_library(${PROJECT_NAME} STATIC ${cppnet_source})
target_link_libraries(${PROJECT_NAME} foundation)

add_subdirectory(test)