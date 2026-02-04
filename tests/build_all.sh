#!/usr/bin/env bash
#
# build_all.sh - Build verification for all cpptudes
#
# This script builds all cpptudes to verify they compile correctly.
# It tests both Debug (with sanitizers) and Release configurations.
#
# Usage:
#   ./tests/build_all.sh           # Build all cpptudes
#   ./tests/build_all.sh --debug   # Build only Debug configuration
#   ./tests/build_all.sh --release # Build only Release configuration
#   ./tests/build_all.sh --clean   # Clean build directories first
#
# Exit codes:
#   0 - All cpptudes built successfully
#   1 - One or more cpptudes failed to build

set -euo pipefail

# Colors for output (disabled if not a terminal)
if [[ -t 1 ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[0;33m'
    BLUE='\033[0;34m'
    BOLD='\033[1m'
    NC='\033[0m' # No Color
else
    RED='' GREEN='' YELLOW='' BLUE='' BOLD='' NC=''
fi

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CPPTUDES_DIR="$PROJECT_ROOT/cpptudes"

# Options
BUILD_DEBUG=true
BUILD_RELEASE=true
CLEAN_FIRST=false
VERBOSE=false

# Results tracking
declare -A RESULTS_DEBUG
declare -A RESULTS_RELEASE
TOTAL_PASS=0
TOTAL_FAIL=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_RELEASE=false
            shift
            ;;
        --release)
            BUILD_DEBUG=false
            shift
            ;;
        --clean)
            CLEAN_FIRST=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug     Build only Debug configuration"
            echo "  --release   Build only Release configuration"
            echo "  --clean     Clean build directories before building"
            echo "  --verbose   Show detailed build output"
            echo "  --help      Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Print a header
header() {
    echo -e "\n${BLUE}${BOLD}=== $1 ===${NC}\n"
}

# Special cpptudes that are expected to fail Debug builds
# 000-setup-guide contains intentional bugs to demonstrate -Werror and sanitizers
EXPECTED_DEBUG_FAILURES=("000-setup-guide")

# Check if a cpptude is expected to fail Debug build
is_expected_debug_failure() {
    local cpptude_name=$1
    for expected in "${EXPECTED_DEBUG_FAILURES[@]}"; do
        if [[ "$cpptude_name" == "$expected" ]]; then
            return 0
        fi
    done
    return 1
}

# Build a single cpptude
# Returns 0 on success, 1 on failure
build_cpptude() {
    local cpptude_dir=$1
    local build_type=$2
    local cpptude_name
    cpptude_name=$(basename "$cpptude_dir")

    local build_dir="$cpptude_dir/build-${build_type,,}"

    # Clean if requested
    if [[ "$CLEAN_FIRST" == true && -d "$build_dir" ]]; then
        rm -rf "$build_dir"
    fi

    # Configure
    local cmake_output
    if [[ "$VERBOSE" == true ]]; then
        echo "  Configuring $cpptude_name ($build_type)..."
        if ! cmake -S "$cpptude_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type"; then
            return 1
        fi
    else
        if ! cmake_output=$(cmake -S "$cpptude_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE="$build_type" 2>&1); then
            echo "$cmake_output"
            return 1
        fi
    fi

    # Build
    local build_output
    if [[ "$VERBOSE" == true ]]; then
        echo "  Building $cpptude_name ($build_type)..."
        if ! cmake --build "$build_dir"; then
            return 1
        fi
    else
        if ! build_output=$(cmake --build "$build_dir" 2>&1); then
            echo "$build_output"
            return 1
        fi
    fi

    return 0
}

# Find all cpptude directories (directories with CMakeLists.txt)
find_cpptudes() {
    find "$CPPTUDES_DIR" -maxdepth 2 -name "CMakeLists.txt" -exec dirname {} \; 2>/dev/null | sort
}

# Main script
main() {
    echo -e "${BLUE}${BOLD}#tudes++ Build Verification${NC}"
    echo "Building all cpptudes to verify they compile correctly."

    # Find cpptudes
    local cpptudes
    mapfile -t cpptudes < <(find_cpptudes)

    if [[ ${#cpptudes[@]} -eq 0 ]]; then
        echo -e "${RED}No cpptudes found in $CPPTUDES_DIR${NC}"
        exit 1
    fi

    echo ""
    echo "Found ${#cpptudes[@]} cpptude(s):"
    for cpptude in "${cpptudes[@]}"; do
        echo "  - $(basename "$cpptude")"
    done

    # Build Debug configuration
    if [[ "$BUILD_DEBUG" == true ]]; then
        header "Building Debug Configuration (with sanitizers)"

        for cpptude_dir in "${cpptudes[@]}"; do
            local cpptude_name
            cpptude_name=$(basename "$cpptude_dir")

            printf "  %-30s " "$cpptude_name"

            if build_cpptude "$cpptude_dir" "Debug"; then
                echo -e "${GREEN}[PASS]${NC}"
                RESULTS_DEBUG["$cpptude_name"]="pass"
                TOTAL_PASS=$((TOTAL_PASS + 1))
            else
                # Check if this failure was expected
                if is_expected_debug_failure "$cpptude_name"; then
                    echo -e "${YELLOW}[EXPECTED]${NC} (intentional bugs for learning)"
                    RESULTS_DEBUG["$cpptude_name"]="expected"
                    TOTAL_PASS=$((TOTAL_PASS + 1))
                else
                    echo -e "${RED}[FAIL]${NC}"
                    RESULTS_DEBUG["$cpptude_name"]="fail"
                    TOTAL_FAIL=$((TOTAL_FAIL + 1))
                fi
            fi
        done
    fi

    # Build Release configuration
    if [[ "$BUILD_RELEASE" == true ]]; then
        header "Building Release Configuration (optimized)"

        for cpptude_dir in "${cpptudes[@]}"; do
            local cpptude_name
            cpptude_name=$(basename "$cpptude_dir")

            printf "  %-30s " "$cpptude_name"

            if build_cpptude "$cpptude_dir" "Release"; then
                echo -e "${GREEN}[PASS]${NC}"
                RESULTS_RELEASE["$cpptude_name"]="pass"
                TOTAL_PASS=$((TOTAL_PASS + 1))
            else
                echo -e "${RED}[FAIL]${NC}"
                RESULTS_RELEASE["$cpptude_name"]="fail"
                TOTAL_FAIL=$((TOTAL_FAIL + 1))
            fi
        done
    fi

    # Summary
    header "Build Summary"

    echo -e "  ${GREEN}Passed:${NC} $TOTAL_PASS"
    echo -e "  ${RED}Failed:${NC} $TOTAL_FAIL"

    echo ""

    if [[ $TOTAL_FAIL -eq 0 ]]; then
        echo -e "${GREEN}All cpptudes built successfully!${NC}"
        return 0
    else
        echo -e "${RED}Some cpptudes failed to build.${NC}"
        echo ""
        echo "Failed builds:"

        if [[ "$BUILD_DEBUG" == true ]]; then
            for cpptude in "${!RESULTS_DEBUG[@]}"; do
                if [[ "${RESULTS_DEBUG[$cpptude]}" == "fail" ]]; then
                    echo "  - $cpptude (Debug)"
                fi
            done
        fi

        if [[ "$BUILD_RELEASE" == true ]]; then
            for cpptude in "${!RESULTS_RELEASE[@]}"; do
                if [[ "${RESULTS_RELEASE[$cpptude]}" == "fail" ]]; then
                    echo "  - $cpptude (Release)"
                fi
            done
        fi

        echo ""
        echo "Run with --verbose for detailed output."
        return 1
    fi
}

main "$@"
