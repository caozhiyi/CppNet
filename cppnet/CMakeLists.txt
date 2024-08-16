cmake_minimum_required(VERSION 3.10)

project(cppnet CXX)

aux_source_directory(${PROJECT_SOURCE_DIR} src_files)
aux_source_directory(${PROJECT_SOURCE_DIR}/event src_files)
aux_source_directory(${PROJECT_SOURCE_DIR}/socket src_files)

IF(WIN32)
    aux_source_directory(${PROJECT_SOURCE_DIR}/event/epoll src_files)
    aux_source_directory(${PROJECT_SOURCE_DIR}/event/epoll/wepoll src_files)
ELSEIF(APPLE)
    aux_source_directory(${PROJECT_SOURCE_DIR}/event/kqueue src_files)
ELSEIF(UNIX)
    IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        aux_source_directory(${PROJECT_SOURCE_DIR}/event/kqueue src_files)
    ELSE()
        aux_source_directory(${PROJECT_SOURCE_DIR}/event/epoll src_files)
    ENDIF()
ENDIF()

SET(cppnet_source ${src_files} PARENT_SCOPE)
