#!/usr/bin/env bash
#
# setup.sh - Environment verification and setup for cpptudes
#
# This script verifies that the required tools are installed and properly
# configured for the cpptudes curriculum. Run this before starting the course.
#
# Usage: ./setup.sh
#
# Exit codes:
#   0 - All requirements met
#   1 - One or more requirements missing (see output for details)

set -euo pipefail

# Colors for output (disabled if not a terminal)
if [[ -t 1 ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[0;33m'
    BLUE='\033[0;34m'
    NC='\033[0m' # No Color
else
    RED='' GREEN='' YELLOW='' BLUE='' NC=''
fi

# Counters
PASS=0
FAIL=0
WARN=0

# Track if at least one compiler has full sanitizer support
HAS_SANITIZER_SUPPORT=false

# Print a header
header() {
    echo -e "\n${BLUE}=== $1 ===${NC}\n"
}

# Check if a command exists
# Usage: check_command <cmd> [min_version] [version_cmd] [optional]
# If optional=true, missing command is a warning, not a failure
check_command() {
    local cmd=$1
    local min_version=${2:-""}
    local version_cmd=${3:-"$cmd --version"}
    local optional=${4:-false}

    if ! command -v "$cmd" &> /dev/null; then
        if [[ "$optional" == true ]]; then
            echo -e "  ${YELLOW}[WARN]${NC} $cmd not found (optional)"
            WARN=$((WARN + 1))
        else
            echo -e "  ${RED}[FAIL]${NC} $cmd not found"
            FAIL=$((FAIL + 1))
        fi
        return 1
    fi

    local version
    version=$($version_cmd 2>&1 | head -n1 || true)
    echo -e "  ${GREEN}[PASS]${NC} $cmd found: $version"
    PASS=$((PASS + 1))
    return 0
}

# Check for a specific compiler version
check_compiler() {
    local compiler=$1
    local min_major=$2

    if ! command -v "$compiler" &> /dev/null; then
        echo -e "  ${YELLOW}[WARN]${NC} $compiler not found (optional, but recommended)"
        WARN=$((WARN + 1))
        return 1
    fi

    local version
    version=$($compiler --version 2>&1 | head -n1)

    # Extract major version number
    local major
    major=$(echo "$version" | grep -oP '\d+' | head -n1 || echo "0")

    if [[ $major -ge $min_major ]]; then
        echo -e "  ${GREEN}[PASS]${NC} $compiler found (version $major >= $min_major): $version"
        PASS=$((PASS + 1))
    else
        echo -e "  ${YELLOW}[WARN]${NC} $compiler version $major < $min_major (recommended): $version"
        WARN=$((WARN + 1))
    fi
    return 0
}

# Check CMake version
check_cmake() {
    if ! command -v cmake &> /dev/null; then
        echo -e "  ${RED}[FAIL]${NC} cmake not found"
        FAIL=$((FAIL + 1))
        return 1
    fi

    local version
    version=$(cmake --version | head -n1)
    local major minor
    major=$(echo "$version" | grep -oP '\d+\.\d+' | head -n1 | cut -d. -f1)
    minor=$(echo "$version" | grep -oP '\d+\.\d+' | head -n1 | cut -d. -f2)

    if [[ $major -gt 3 ]] || [[ $major -eq 3 && $minor -ge 20 ]]; then
        echo -e "  ${GREEN}[PASS]${NC} cmake found (>= 3.20): $version"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${NC} cmake version < 3.20: $version"
        FAIL=$((FAIL + 1))
    fi
}

# Check C++ standard support
check_cpp20_support() {
    local compiler=$1

    if ! command -v "$compiler" &> /dev/null; then
        return 1
    fi

    # Create a temporary file to test C++20 support
    local tmpfile
    tmpfile=$(mktemp --suffix=.cpp)

    cat > "$tmpfile" << 'EOF'
#include <concepts>
#include <ranges>
#include <span>
#include <array>

template<typename T>
concept Printable = requires(T t) { { t } -> std::convertible_to<int>; };

int main() {
    std::array<int, 5> arr{1, 2, 3, 4, 5};
    auto view = arr | std::views::filter([](int x) { return x > 2; });
    return 0;
}
EOF

    if $compiler -std=c++20 -c "$tmpfile" -o /dev/null 2>/dev/null; then
        echo -e "  ${GREEN}[PASS]${NC} $compiler supports C++20"
        PASS=$((PASS + 1))
        rm -f "$tmpfile"
        return 0
    else
        echo -e "  ${RED}[FAIL]${NC} $compiler does not fully support C++20"
        FAIL=$((FAIL + 1))
        rm -f "$tmpfile"
        return 1
    fi
}

# Check sanitizer support
check_sanitizers() {
    local compiler=$1

    if ! command -v "$compiler" &> /dev/null; then
        return 1
    fi

    local tmpfile
    tmpfile=$(mktemp --suffix=.cpp)

    cat > "$tmpfile" << 'EOF'
int main() { return 0; }
EOF

    local tmpout
    tmpout=$(mktemp)

    local asan_ok=false
    local ubsan_ok=false

    # Check ASan
    if $compiler -std=c++20 -fsanitize=address "$tmpfile" -o "$tmpout" 2>/dev/null; then
        echo -e "  ${GREEN}[PASS]${NC} $compiler supports AddressSanitizer (ASan)"
        PASS=$((PASS + 1))
        asan_ok=true
    else
        if [[ "$HAS_SANITIZER_SUPPORT" == true ]]; then
            echo -e "  ${YELLOW}[WARN]${NC} $compiler does not support AddressSanitizer (another compiler does)"
            WARN=$((WARN + 1))
        else
            echo -e "  ${RED}[FAIL]${NC} $compiler does not support AddressSanitizer"
            FAIL=$((FAIL + 1))
        fi
    fi

    # Check UBSan
    if $compiler -std=c++20 -fsanitize=undefined "$tmpfile" -o "$tmpout" 2>/dev/null; then
        echo -e "  ${GREEN}[PASS]${NC} $compiler supports UndefinedBehaviorSanitizer (UBSan)"
        PASS=$((PASS + 1))
        ubsan_ok=true
    else
        if [[ "$HAS_SANITIZER_SUPPORT" == true ]]; then
            echo -e "  ${YELLOW}[WARN]${NC} $compiler does not support UndefinedBehaviorSanitizer (another compiler does)"
            WARN=$((WARN + 1))
        else
            echo -e "  ${RED}[FAIL]${NC} $compiler does not support UndefinedBehaviorSanitizer"
            FAIL=$((FAIL + 1))
        fi
    fi

    # Track if this compiler has full support
    if [[ "$asan_ok" == true && "$ubsan_ok" == true ]]; then
        HAS_SANITIZER_SUPPORT=true
    fi

    rm -f "$tmpfile" "$tmpout"
}

# Main script
main() {
    echo -e "${BLUE}#tudes++ Environment Verification${NC}"
    echo "This script checks that your development environment is ready for cpptudes."

    header "Required: C++ Compilers"

    # Check for at least one compiler
    local have_compiler=false

    if check_compiler "g++" 12; then
        check_cpp20_support "g++"
        check_sanitizers "g++"
        have_compiler=true
    fi

    echo ""

    if check_compiler "clang++" 15; then
        check_cpp20_support "clang++"
        check_sanitizers "clang++"
        have_compiler=true
    fi

    if [[ "$have_compiler" == false ]]; then
        echo -e "  ${RED}[FAIL]${NC} No suitable C++ compiler found (need g++ 12+ or clang++ 15+)"
        FAIL=$((FAIL + 1))
    fi

    header "Required: Build Tools"
    check_cmake
    check_command "make"
    check_command "ninja" "" "ninja --version" || true  # Optional but recommended

    header "Required: Git"
    check_command "git"

    # Check git configuration
    if git config --get user.name &> /dev/null; then
        echo -e "  ${GREEN}[PASS]${NC} git user.name configured: $(git config --get user.name)"
        PASS=$((PASS + 1))
    else
        echo -e "  ${YELLOW}[WARN]${NC} git user.name not configured (run: git config --global user.name \"Your Name\")"
        WARN=$((WARN + 1))
    fi

    if git config --get user.email &> /dev/null; then
        echo -e "  ${GREEN}[PASS]${NC} git user.email configured: $(git config --get user.email)"
        PASS=$((PASS + 1))
    else
        echo -e "  ${YELLOW}[WARN]${NC} git user.email not configured (run: git config --global user.email \"you@example.com\")"
        WARN=$((WARN + 1))
    fi

    header "Optional: Debugging & Analysis"
    check_command "gdb" "" "gdb --version" true || true
    check_command "valgrind" "" "valgrind --version" true || true
    check_command "heaptrack" "" "heaptrack --version" true || true

    header "Quick Build Test"

    # Try to build cpptude #000 as a sanity check
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local cpptude_000="$script_dir/cpptudes/000-setup-guide"

    if [[ -d "$cpptude_000" ]]; then
        echo "  Building cpptude #000 (Setup Guide) as a test..."

        local build_dir
        build_dir=$(mktemp -d)

        if cmake -S "$cpptude_000" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release &> /dev/null; then
            if cmake --build "$build_dir" &> /dev/null; then
                echo -e "  ${GREEN}[PASS]${NC} cpptude #000 builds successfully"
                PASS=$((PASS + 1))
            else
                echo -e "  ${RED}[FAIL]${NC} cpptude #000 failed to build"
                FAIL=$((FAIL + 1))
            fi
        else
            echo -e "  ${RED}[FAIL]${NC} CMake configuration failed for cpptude #000"
            FAIL=$((FAIL + 1))
        fi

        rm -rf "$build_dir"
    else
        echo -e "  ${YELLOW}[WARN]${NC} cpptude #000 not found, skipping build test"
        WARN=$((WARN + 1))
    fi

    # Summary
    header "Summary"

    echo -e "  ${GREEN}Passed:${NC}   $PASS"
    echo -e "  ${RED}Failed:${NC}   $FAIL"
    echo -e "  ${YELLOW}Warnings:${NC} $WARN"

    echo ""

    if [[ $FAIL -eq 0 ]]; then
        echo -e "${GREEN}Your environment is ready for cpptudes!${NC}"
        echo ""
        echo "Next steps:"
        echo "  1. Read tude++_environment_setup.md for a detailed guide"
        echo "  2. Start with cpptude #000: cd cpptudes/000-setup-guide"
        echo "  3. Run: cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build"
        echo ""
        return 0
    else
        echo -e "${RED}Your environment is missing required components.${NC}"
        echo ""
        echo "Fix the FAIL items above before starting the cpptudes."
        echo "See tude++_environment_setup.md for installation instructions."
        echo ""
        return 1
    fi
}

main "$@"
