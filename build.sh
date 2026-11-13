#!/bin/sh
# The legacy bare `make -j4` referenced a makefile that no longer exists
# (and which was stuck on c++11 while the sources need C++17 for
# <filesystem>). CMake is the only supported build path now.
set -e
cmake -B build -S .
cmake --build build --parallel
