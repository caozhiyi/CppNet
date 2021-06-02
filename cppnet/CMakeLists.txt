cmake_minimum_required(VERSION 3.10)

project(cppnet CXX)

aux_source_directory(${PROJECT_SOURCE_DIR} src_files)
aux_source_directory(${PROJECT_SOURCE_DIR}/event src_files)
aux_source_directory(${PROJECT_SOURCE_DIR}/socket src_files)

IF (WIN32)
	aux_source_directory(${PROJECT_SOURCE_DIR}/event/win src_files)
	aux_source_directory(${PROJECT_SOURCE_DIR}/socket/win src_files)
ELSEIF (APPLE)
	aux_source_directory(${PROJECT_SOURCE_DIR}/event/mac src_files)
	aux_source_directory(${PROJECT_SOURCE_DIR}/socket/mac src_files)
ELSEIF (UNIX)
	aux_source_directory(${PROJECT_SOURCE_DIR}/event/linux src_files)
	aux_source_directory(${PROJECT_SOURCE_DIR}/socket/linux src_files)
ENDIF ()

SET(cppnet_source ${src_files} PARENT_SCOPE)