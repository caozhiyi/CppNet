# cmake/Sanitizers.cmake
#
# Phase 0.3 of UPGRADE_PLAN.md
#
# Lightweight, opt-in sanitizer plumbing. Activated by configuring with:
#
#     cmake -B build -DCPPNET_SANITIZER=address
#     cmake -B build -DCPPNET_SANITIZER=thread
#     cmake -B build -DCPPNET_SANITIZER=undefined
#     cmake -B build -DCPPNET_SANITIZER=address,undefined
#     cmake -B build -DCPPNET_SANITIZER=leak
#
# Notes:
#   - Address and Thread are mutually exclusive.
#   - Undefined can be combined with Address (canonical "ASan+UBSan" build).
#   - MSVC only supports address; thread/undefined are silently downgraded.
#   - Use cppnet_target_enable_sanitizer(<target>) to apply flags to each
#     target that should be instrumented.

set(CPPNET_SANITIZER "" CACHE STRING
    "Comma-separated list of sanitizers to enable: address, thread, undefined, leak")

# Sanitizers must be applied uniformly across the entire program. Mixing
# instrumented and uninstrumented translation units in the same binary
# produces undefined references to __tsan_* / __asan_* runtime symbols at
# link time. We therefore apply the flags GLOBALLY via add_compile_options
# / add_link_options below, rather than per-target. The
# cppnet_target_enable_sanitizer() helper is kept as a no-op shim for
# call sites that previously expected a per-target API.
set(_cppnet_sanitizer_flags "")
set(_cppnet_sanitizer_link_flags "")

if(CPPNET_SANITIZER)
    string(REPLACE "," ";" _san_list "${CPPNET_SANITIZER}")

    # Mutual exclusion check: ASan vs TSan.
    list(FIND _san_list "address" _has_asan)
    list(FIND _san_list "thread" _has_tsan)
    if(NOT _has_asan EQUAL -1 AND NOT _has_tsan EQUAL -1)
        message(FATAL_ERROR
            "CPPNET_SANITIZER cannot combine 'address' and 'thread' "
            "(they are mutually exclusive at runtime)")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        foreach(_san IN LISTS _san_list)
            if(_san STREQUAL "address")
                list(APPEND _cppnet_sanitizer_flags
                    -fsanitize=address
                    -fno-omit-frame-pointer
                    -fno-optimize-sibling-calls)
                list(APPEND _cppnet_sanitizer_link_flags -fsanitize=address)
            elseif(_san STREQUAL "thread")
                list(APPEND _cppnet_sanitizer_flags
                    -fsanitize=thread
                    -fno-omit-frame-pointer)
                list(APPEND _cppnet_sanitizer_link_flags -fsanitize=thread)
            elseif(_san STREQUAL "undefined")
                list(APPEND _cppnet_sanitizer_flags
                    -fsanitize=undefined
                    -fno-omit-frame-pointer
                    -fno-sanitize-recover=all)
                list(APPEND _cppnet_sanitizer_link_flags -fsanitize=undefined)
            elseif(_san STREQUAL "leak")
                list(APPEND _cppnet_sanitizer_flags -fsanitize=leak)
                list(APPEND _cppnet_sanitizer_link_flags -fsanitize=leak)
            else()
                message(FATAL_ERROR
                    "Unknown CPPNET_SANITIZER value: '${_san}'. "
                    "Expected one of: address, thread, undefined, leak")
            endif()
        endforeach()
        message(STATUS "CppNet sanitizers enabled: ${CPPNET_SANITIZER}")
    elseif(MSVC)
        # MSVC has /fsanitize=address since VS 16.9; thread/undefined unsupported.
        if(NOT _has_asan EQUAL -1)
            list(APPEND _cppnet_sanitizer_flags /fsanitize=address)
            message(STATUS "CppNet sanitizers enabled: address (MSVC)")
        else()
            message(WARNING
                "CPPNET_SANITIZER='${CPPNET_SANITIZER}' requested but MSVC "
                "only supports 'address'. Other sanitizers ignored.")
        endif()
    else()
        message(WARNING
            "CPPNET_SANITIZER set but compiler '${CMAKE_CXX_COMPILER_ID}' "
            "is not recognized. Sanitizers will not be applied.")
    endif()
endif()

# Apply globally. Must be called from the top-level CMakeLists BEFORE
# any add_subdirectory() / add_library() / add_executable(), otherwise
# targets defined earlier won't pick up the flags and the link will fail
# with undefined __tsan_* / __asan_* symbols.
if(_cppnet_sanitizer_flags)
    add_compile_options(${_cppnet_sanitizer_flags})
    add_link_options(${_cppnet_sanitizer_link_flags})
endif()

# Backwards-compatible no-op shim. Sanitizer flags are now applied
# globally above, so per-target opt-in is unnecessary. Kept so existing
# call sites (and future ones written from muscle memory) don't break.
function(cppnet_target_enable_sanitizer target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "cppnet_target_enable_sanitizer: '${target}' is not a target")
    endif()
    # Intentionally empty: see comment above.
endfunction()
