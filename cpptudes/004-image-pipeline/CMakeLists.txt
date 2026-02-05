# CMakeLists.txt — Cpptude #004: Image Pipeline — Move Semantics
#
# This cpptude demonstrates move semantics through an image processing pipeline.
# The ImageBuffer class owns heap-allocated pixel data, making the cost
# difference between copying (memcpy all pixels) and moving (transfer pointer)
# very visible.
#
# BUILD:
#   cmake -B build -DCMAKE_BUILD_TYPE=Debug    # Debug with sanitizers
#   cmake -B build -DCMAKE_BUILD_TYPE=Release  # Optimized build
#   cmake --build build
#
# RUN:
#   ./build/image_pipeline

cmake_minimum_required(VERSION 3.20)
project(cpptude_004_image_pipeline LANGUAGES CXX)

# --- C++20 Standard (required for this cpptude) ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Executable: two translation units compiled separately, then linked ---
add_executable(image_pipeline
    main.cpp
    image_buffer.cpp
)

# --- Strict warnings: every cpptude builds with warnings-as-errors ---
# Cross-platform: GCC/Clang flags and MSVC equivalents.
# Generator expressions ensure correct flags for each compiler.
target_compile_options(image_pipeline PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Wpedantic -Werror
    >
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4 /WX
    >
)

# --- Sanitizers: always-on in Debug builds ---
# ASan catches use-after-move bugs (use-after-free if you dereference
# a moved-from pointer). UBSan catches signed overflow, null dereference, etc.
#
# These are infrastructure, not optional debugging tools. They replace the
# runtime safety net that C# developers rely on (null reference exceptions,
# array bounds checking, etc.).
#
# Uses generator expressions for multi-config generator compatibility
# (Visual Studio, Xcode, Ninja Multi-Config).
target_compile_options(image_pipeline PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined -fno-omit-frame-pointer -g
    >
)
target_link_options(image_pipeline PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang,AppleClang>>:
        -fsanitize=address,undefined
    >
)
