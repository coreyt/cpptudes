# CMakeLists.txt — Cpptude #003: Parameter Passing
#
# Minimum CMake 3.20+ (required by curriculum)
# Minimum compilers: GCC 13+, Clang 17+, MSVC 19.30+
#
# Usage:
#   cmake -B build -DCMAKE_BUILD_TYPE=Debug    # Debug with sanitizers
#   cmake -B build -DCMAKE_BUILD_TYPE=Release  # Optimized build
#   cmake --build build
#   ./build/parameter_passing

cmake_minimum_required(VERSION 3.20)
project(cpptude-003-parameter-passing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(parameter_passing
    main.cpp
    telemetry.cpp
)

# Strict warnings — every cpptude builds with -Wall -Wextra -Werror
target_compile_options(parameter_passing PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Werror -Wpedantic
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX
    >
)

# Sanitizers in Debug mode — ASan and UBSan are infrastructure, not optional
# (see best-practices.md Principle 8)
target_compile_options(parameter_passing PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined -fno-omit-frame-pointer -g
    >
)
target_link_options(parameter_passing PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined
    >
)
