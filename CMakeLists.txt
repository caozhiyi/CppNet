cmake_minimum_required(VERSION 3.10)
project(cppnet CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(${CMAKE_SOURCE_DIR}/base BASESOURCE)
aux_source_directory(${CMAKE_SOURCE_DIR}/net NETSOURCE)
if(UNIX)
	aux_source_directory(${CMAKE_SOURCE_DIR}/net/linux NETLINUXSOURCE)
	include_directories(${CMAKE_SOURCE_DIR}/net/linux NETLINUXSOURCE)
else()
	aux_source_directory(${CMAKE_SOURCE_DIR}/net/win NETLINUXSOURCE)
	include_directories(${CMAKE_SOURCE_DIR}/net/win NETLINUXSOURCE)
endif()

add_library(cppnet STATIC ${BASESOURCE} ${NETSOURCE} ${NETLINUXSOURCE})

target_include_directories(cppnet 
PUBLIC ${CMAKE_SOURCE_DIR}/base
PUBLIC ${CMAKE_SOURCE_DIR}/net
PUBLIC ${CMAKE_SOURCE_DIR}/include 
)

target_compile_options(cppnet
PUBLIC
-lstdc++
-pipe
-fPIC
-m64
)

target_link_libraries(cppnet
PUBLIC
pthread
)

if(BUILD_TEST)
add_subdirectory(${CMAKE_SOURCE_DIR}/test)
endif()