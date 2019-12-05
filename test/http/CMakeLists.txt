aux_source_directory(. HTTPSOURCE)
add_executable(httpser ${HTTPSOURCE})
target_link_libraries(httpser PUBLIC cppnet)