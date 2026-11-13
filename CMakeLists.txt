cmake_minimum_required(VERSION 3.13)

project(cppnet C CXX)

# ---------------------------------------------------------------------------
# Language / build standard
# ---------------------------------------------------------------------------
# Phase 0.1 of UPGRADE_PLAN: bump to C++17. This is a hard prerequisite for
# vendoring quicX/common modules in later phases (event loop / timing wheel /
# buffer chunks all rely on C++17 features such as if-constexpr, structured
# bindings, std::optional, std::string_view, inline variables, etc.).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to a sane build type when the user doesn't pick one (e.g. Ninja /
# single-config generators). We want optimized + assertions during early
# productionization work, so RelWithDebInfo is the most useful default.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
        "Choose: Debug Release RelWithDebInfo MinSizeRel" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# ---------------------------------------------------------------------------
# Compiler warnings
# ---------------------------------------------------------------------------
# Phase 0 establishes the quality bar. We turn on -Wall/-Wextra here but
# deliberately do NOT enable -Werror yet -- the codebase has known warnings
# from the legacy implementation that phases 1-7 will clean up. Once the
# vendored modules land we'll flip -Werror on per-target.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -fPIC
        -pipe
    )
endif()

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
endif()

# 32-bit cross compile (kept from legacy build for backward compatibility)
if(CMAKE_BUILD_BITS EQUAL 32)
    add_compile_options(-m32)
    add_link_options(-m32)
endif()

# ---------------------------------------------------------------------------
# Platform defines
# ---------------------------------------------------------------------------
if(WIN32)
    add_definitions(-D__win__)
    add_definitions(-D__use_iocp__)
endif()

# ---------------------------------------------------------------------------
# Sanitizers (Phase 0.3)
# ---------------------------------------------------------------------------
# Opt-in via -DCPPNET_SANITIZER=address|thread|undefined|leak.
# See cmake/Sanitizers.cmake for the full menu.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

# ---------------------------------------------------------------------------
# Output layout
# ---------------------------------------------------------------------------
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------
# Includes
# ---------------------------------------------------------------------------
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ---------------------------------------------------------------------------
# Threads (replaces the legacy bare `-lpthread` / `-lstdc++` flags)
# ---------------------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Subprojects
# ---------------------------------------------------------------------------
add_subdirectory(common)
add_subdirectory(cppnet)

add_library(${PROJECT_NAME} STATIC ${common_source} ${cppnet_source})
target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Apply selected sanitizer (no-op when CPPNET_SANITIZER is unset).
cppnet_target_enable_sanitizer(${PROJECT_NAME})

add_subdirectory(test)
