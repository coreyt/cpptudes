# CMakeLists.txt — Cpptude #002: File Processor
#
# This is the first cpptude with multiple translation units (.cpp files).
# CMake compiles each .cpp independently and links them together.
#
# BUILD:
#   cmake -B build -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build
#
# RUN:
#   ./build/file_processor

cmake_minimum_required(VERSION 3.20)
project(cpptude_002_file_processor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- The executable: two .cpp files compiled separately, linked together ---
# CMake compiles file_processor.cpp and main.cpp as independent translation
# units, then links the resulting object files. This mirrors the C++ build
# model explained in the README's Build Model sidebar.
add_executable(file_processor
    main.cpp
    file_processor.cpp
)

# --- Warnings: treat all warnings as errors ---
# Cross-platform: GCC/Clang flags and MSVC equivalents
target_compile_options(file_processor PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Wpedantic -Werror
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX
    >
)

# --- Sanitizers: always-on in Debug builds ---
# ASan and UBSan are infrastructure, not optional debugging tools.
# They replace the runtime safety net that C# developers rely on.
# Uses generator expressions for multi-config generator compatibility.
target_compile_options(file_processor PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined -fno-omit-frame-pointer -g
    >
)
target_link_options(file_processor PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined
    >
)
