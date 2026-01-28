cmake_minimum_required(VERSION 3.20)
project(cpptude-000-setup-guide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Cpptude #000: Setup Guide — Build Configuration
# =============================================================================
#
# This CMakeLists.txt demonstrates two build modes:
#
#   1. "Naive" build (Release): compiles the buggy code without sanitizers.
#      The bugs are silent. This is the WRONG way to develop C++.
#
#      cmake -B build-naive -DCMAKE_BUILD_TYPE=Release
#      cmake --build build-naive
#      ./build-naive/setup-guide
#
#   2. "Safe" build (Debug): compiles with ASan, UBSan, and -Werror.
#      The bugs are caught. This is the RIGHT way to develop C++.
#
#      cmake -B build-safe -DCMAKE_BUILD_TYPE=Debug
#      cmake --build build-safe
#      ./build-safe/setup-guide
#
# NOTE: The Debug build with -Werror will refuse to compile the uninitialized
# variable bug. This is intentional — it demonstrates that -Werror gives you
# C#-equivalent compile-time safety for this category of bug.
#
# To see all three bugs in action (including the uninitialized read at
# runtime), build in Release mode first. Then switch to Debug to see the
# sanitizers and -Werror catch the bugs.
# =============================================================================

add_executable(setup-guide main.cpp)

# Warnings that every cpptude uses — these are non-negotiable.
target_compile_options(setup-guide PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# --- Debug: Sanitizers + Werror (the "safe" build) ---
# ASan catches memory errors (use-after-free, buffer overflow, leaks).
# UBSan catches undefined behavior (signed overflow, null deref, shift UB).
# -Werror turns warnings into errors (like C#'s compile-time checks).
# -fno-omit-frame-pointer ensures sanitizer stack traces are readable.
# -O1 is recommended by ASan documentation for useful stack traces.
target_compile_options(setup-guide PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-O1>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    $<$<CONFIG:Debug>:-Werror>
)

target_link_options(setup-guide PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# --- Release: No sanitizers, no -Werror (the "naive" build) ---
# This deliberately omits safety tooling to demonstrate silent failure.
# In real projects, you would NEVER ship without having run sanitizers.
target_compile_options(setup-guide PRIVATE
    $<$<CONFIG:Release>:-O2>
)
