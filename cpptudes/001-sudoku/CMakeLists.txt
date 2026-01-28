# CMakeLists.txt — Cpptude #001: Sudoku Solver
#
# Minimum CMake 3.20+ (required by curriculum)
# Minimum compilers: GCC 12+, Clang 15+, MSVC 19.30+
#
# Usage:
#   cmake -B build -DCMAKE_BUILD_TYPE=Debug    # Debug with sanitizers
#   cmake -B build -DCMAKE_BUILD_TYPE=Release  # Optimized build
#   cmake --build build
#   ./build/sudoku

cmake_minimum_required(VERSION 3.20)
project(cpptude-001-sudoku LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sudoku
    main.cpp
    sudoku.cpp
    sudoku.hpp
)

# Strict warnings — every cpptude builds with -Wall -Wextra -Werror
target_compile_options(sudoku PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Werror -Wpedantic
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX
    >
)

# Sanitizers in Debug mode — ASan and UBSan are infrastructure, not optional
# (see best-practices.md Principle 8)
target_compile_options(sudoku PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined -fno-omit-frame-pointer -g
    >
)
target_link_options(sudoku PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined
    >
)
