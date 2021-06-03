cmake_minimum_required(VERSION 3.10)

project(cppnet)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CXX_FLAGS
 -g
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 -std=c++11
 -rdynamic
 )

if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
else()
  list(APPEND CXX_FLAGS "-m64")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if (WIN32)
add_definitions(-D__win__)
add_definitions(-D__use_iocp__)
endif ()

# output
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# include
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(common)
add_subdirectory(cppnet)

add_library(${PROJECT_NAME} STATIC ${common_source} ${cppnet_source})

add_subdirectory(test)