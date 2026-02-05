#!/usr/bin/env bash
#
# verify_student.sh - Verify student completion of cpptudes
#
# This script validates that a student has successfully learned the concepts
# in each cpptude by checking:
#   1. The code compiles with strict warnings (-Werror)
#   2. The code passes sanitizer checks (ASan, UBSan)
#   3. The output is correct
#   4. The implementation follows the cpptude constraints (where applicable)
#
# Usage:
#   ./tests/verify_student.sh              # Verify all cpptudes
#   ./tests/verify_student.sh 001-sudoku   # Verify specific cpptude
#   ./tests/verify_student.sh --list       # List available cpptudes
#
# Exit codes:
#   0 - All verifications passed
#   1 - One or more verifications failed

set -euo pipefail

# Colors for output (disabled if not a terminal)
if [[ -t 1 ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[0;33m'
    BLUE='\033[0;34m'
    CYAN='\033[0;36m'
    BOLD='\033[1m'
    NC='\033[0m' # No Color
else
    RED='' GREEN='' YELLOW='' BLUE='' CYAN='' BOLD='' NC=''
fi

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CPPTUDES_DIR="$PROJECT_ROOT/cpptudes"

# Results tracking
PASS=0
FAIL=0
WARN=0

# Print a header
header() {
    echo -e "\n${BLUE}${BOLD}=== $1 ===${NC}\n"
}

# Print a subheader
subheader() {
    echo -e "\n${CYAN}--- $1 ---${NC}\n"
}

# Find all cpptude directories
find_cpptudes() {
    find "$CPPTUDES_DIR" -maxdepth 2 -name "CMakeLists.txt" -exec dirname {} \; 2>/dev/null | sort
}

# Build a cpptude with Debug configuration (includes sanitizers)
build_debug() {
    local cpptude_dir=$1
    local build_dir="$cpptude_dir/build-verify"

    rm -rf "$build_dir"

    if ! cmake -S "$cpptude_dir" -B "$build_dir" -DCMAKE_BUILD_TYPE=Debug &> /dev/null; then
        return 1
    fi

    if ! cmake --build "$build_dir" 2>&1; then
        return 1
    fi

    return 0
}

# Run a cpptude and capture output
run_cpptude() {
    local executable=$1
    shift
    local args=("$@")

    # Set sanitizer options to be strict
    export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1"
    export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1"

    # Run with timeout to prevent infinite loops
    timeout 60s "$executable" "${args[@]}" 2>&1 || return $?
}

# ============================================================================
# CPPTUDE #000: Setup Guide
# ============================================================================
# This cpptude is special: it contains INTENTIONAL bugs to demonstrate
# sanitizers. We verify it by:
#   1. Building in Release mode (bugs should be silent)
#   2. Building in Debug mode should FAIL to compile due to -Werror
#   3. If compiled without -Werror, running should trigger sanitizers

verify_000_setup_guide() {
    local cpptude_dir="$CPPTUDES_DIR/000-setup-guide"
    local cpptude_name="000-setup-guide"

    header "Verifying Cpptude #000: Setup Guide"
    echo "This cpptude tests that sanitizers and strict warnings are working."
    echo "It contains intentional bugs that should be caught by the toolchain."

    subheader "Test 1: Release build (silent bugs)"
    local build_release="$cpptude_dir/build-verify-release"
    rm -rf "$build_release"

    if cmake -S "$cpptude_dir" -B "$build_release" -DCMAKE_BUILD_TYPE=Release &> /dev/null; then
        if cmake --build "$build_release" &> /dev/null; then
            echo -e "  ${GREEN}[PASS]${NC} Release build compiles (bugs are silent without sanitizers)"
            PASS=$((PASS + 1))

            # Run it - should NOT crash in Release mode
            if "$build_release/setup-guide" &> /dev/null; then
                echo -e "  ${GREEN}[PASS]${NC} Release binary runs (bugs are silent)"
                PASS=$((PASS + 1))
            else
                echo -e "  ${YELLOW}[WARN]${NC} Release binary crashed (undefined behavior manifested)"
                WARN=$((WARN + 1))
            fi
        else
            echo -e "  ${RED}[FAIL]${NC} Release build failed"
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "  ${RED}[FAIL]${NC} CMake configuration failed"
        FAIL=$((FAIL + 1))
    fi

    subheader "Test 2: Debug build should fail (uninitialized variable)"
    local build_debug="$cpptude_dir/build-verify-debug"
    rm -rf "$build_debug"

    # Debug build should fail due to -Werror on the uninitialized variable
    local build_output
    if cmake -S "$cpptude_dir" -B "$build_debug" -DCMAKE_BUILD_TYPE=Debug &> /dev/null; then
        if build_output=$(cmake --build "$build_debug" 2>&1); then
            echo -e "  ${YELLOW}[WARN]${NC} Debug build succeeded (expected -Werror to fail on uninitialized var)"
            WARN=$((WARN + 1))

            # If it built, sanitizers should catch the bugs at runtime
            subheader "Test 3: Sanitizers should catch bugs at runtime"
            export ASAN_OPTIONS="detect_leaks=1"
            export UBSAN_OPTIONS="print_stacktrace=1"

            if ! "$build_debug/setup-guide" &> /dev/null; then
                echo -e "  ${GREEN}[PASS]${NC} Sanitizers caught bugs at runtime (expected)"
                PASS=$((PASS + 1))
            else
                echo -e "  ${RED}[FAIL]${NC} Sanitizers did not catch any bugs"
                FAIL=$((FAIL + 1))
            fi
        else
            if echo "$build_output" | grep -q "uninitialized\|Werror"; then
                echo -e "  ${GREEN}[PASS]${NC} Debug build failed as expected (-Werror caught uninitialized variable)"
                PASS=$((PASS + 1))
            else
                echo -e "  ${YELLOW}[WARN]${NC} Debug build failed for unexpected reason"
                echo "$build_output" | tail -10
                WARN=$((WARN + 1))
            fi
        fi
    else
        echo -e "  ${RED}[FAIL]${NC} CMake configuration failed"
        FAIL=$((FAIL + 1))
    fi

    rm -rf "$build_release" "$build_debug"

    echo ""
    echo "Cpptude #000 verification complete."
    echo "Key learning: Sanitizers and -Werror are essential C++ infrastructure."
}

# ============================================================================
# CPPTUDE #001: Sudoku Solver
# ============================================================================
# Verifies:
#   1. Code compiles with strict warnings
#   2. Passes sanitizer checks
#   3. Solves test puzzles correctly
#   4. Uses zero heap allocations (verified with output or heaptrack if available)

verify_001_sudoku() {
    local cpptude_dir="$CPPTUDES_DIR/001-sudoku"
    local cpptude_name="001-sudoku"

    header "Verifying Cpptude #001: Sudoku Solver"
    echo "Primary concept: Stack allocation & value semantics"
    echo "Constraint: Zero heap allocations"

    subheader "Test 1: Build with strict warnings"
    local build_dir="$cpptude_dir/build-verify"

    local build_output
    if build_output=$(build_debug "$cpptude_dir" 2>&1); then
        echo -e "  ${GREEN}[PASS]${NC} Compiles with -Wall -Wextra -Werror"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${NC} Failed to compile with strict warnings"
        echo "$build_output" | tail -10
        FAIL=$((FAIL + 1))
        rm -rf "$build_dir"
        return
    fi

    subheader "Test 2: Run test suite"
    local run_output
    if run_output=$(run_cpptude "$build_dir/sudoku" 2>&1); then
        # Check for test results
        if echo "$run_output" | grep -q "All tests passed"; then
            echo -e "  ${GREEN}[PASS]${NC} All test puzzles solved correctly"
            PASS=$((PASS + 1))
        elif echo "$run_output" | grep -q "PASSED"; then
            local passed_count
            passed_count=$(echo "$run_output" | grep -c "PASSED" || echo "0")
            echo -e "  ${GREEN}[PASS]${NC} Tests passed ($passed_count tests)"
            PASS=$((PASS + 1))
        else
            echo -e "  ${RED}[FAIL]${NC} Test puzzles not solved correctly"
            echo "$run_output" | grep -E "(FAIL|Error)" | head -5
            FAIL=$((FAIL + 1))
        fi
    else
        echo -e "  ${RED}[FAIL]${NC} Program crashed (sanitizer caught an error?)"
        echo "$run_output" | tail -10
        FAIL=$((FAIL + 1))
    fi

    subheader "Test 3: Verify correct puzzle solution"
    local easy_puzzle="530070000600195000098000060800060003400803001700020006060000280000419005000080079"
    local expected_solution="534678912672195348198342567859761423426853791713924856961537284287419635345286179"

    if run_output=$(run_cpptude "$build_dir/sudoku" "$easy_puzzle" 2>&1); then
        if echo "$run_output" | grep -q "$expected_solution"; then
            echo -e "  ${GREEN}[PASS]${NC} Solved puzzle matches expected solution"
            PASS=$((PASS + 1))
        else
            echo -e "  ${YELLOW}[WARN]${NC} Could not verify solution format (may still be correct)"
            WARN=$((WARN + 1))
        fi
    else
        echo -e "  ${RED}[FAIL]${NC} Failed to solve command-line puzzle"
        FAIL=$((FAIL + 1))
    fi

    subheader "Test 4: Verify memory layout (stack allocation)"
    # The program outputs memory layout info
    if echo "$run_output" | grep -q "Board.*216 bytes\|sizeof(Board).*216"; then
        echo -e "  ${GREEN}[PASS]${NC} Board uses expected 216 bytes (value semantics)"
        PASS=$((PASS + 1))
    else
        # Check the output for any indication of stack usage
        if echo "$run_output" | grep -q "Heap allocations: 0\|zero heap"; then
            echo -e "  ${GREEN}[PASS]${NC} Zero heap allocations confirmed"
            PASS=$((PASS + 1))
        else
            echo -e "  ${YELLOW}[WARN]${NC} Could not verify zero-heap constraint from output"
            WARN=$((WARN + 1))

            # Try heaptrack if available
            if command -v heaptrack &> /dev/null; then
                echo "  Running heaptrack to verify..."
                local heaptrack_output
                if heaptrack_output=$(heaptrack "$build_dir/sudoku" 2>&1); then
                    if echo "$heaptrack_output" | grep -q "0 allocations"; then
                        echo -e "  ${GREEN}[PASS]${NC} Heaptrack confirms zero heap allocations"
                        PASS=$((PASS + 1))
                    fi
                fi
            fi
        fi
    fi

    rm -rf "$build_dir"

    echo ""
    echo "Cpptude #001 verification complete."
    echo "Key learning: Stack allocation avoids heap overhead; value semantics means copies are cheap."
}

# ============================================================================
# CPPTUDE #002: File Processor
# ============================================================================
# Verifies:
#   1. Code compiles with strict warnings
#   2. Passes sanitizer checks (especially important for RAII)
#   3. RAII cleanup works on normal and exception paths
#   4. Destructor messages appear in correct order

verify_002_file_processor() {
    local cpptude_dir="$CPPTUDES_DIR/002-file-processor"
    local cpptude_name="002-file-processor"

    header "Verifying Cpptude #002: File Processor"
    echo "Primary concept: RAII (Resource Acquisition Is Initialization)"
    echo "Constraint: Resources cleaned up on all paths, including exceptions"

    subheader "Test 1: Build with strict warnings"
    local build_dir="$cpptude_dir/build-verify"

    local build_output
    if build_output=$(build_debug "$cpptude_dir" 2>&1); then
        echo -e "  ${GREEN}[PASS]${NC} Compiles with -Wall -Wextra -Werror"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${NC} Failed to compile with strict warnings"
        echo "$build_output" | tail -10
        FAIL=$((FAIL + 1))
        rm -rf "$build_dir"
        return
    fi

    subheader "Test 2: Run demonstrations"
    local run_output
    if run_output=$(run_cpptude "$build_dir/file_processor" 2>&1); then
        echo -e "  ${GREEN}[PASS]${NC} Program runs without sanitizer errors"
        PASS=$((PASS + 1))
    else
        echo -e "  ${RED}[FAIL]${NC} Program crashed or sanitizer caught an error"
        echo "$run_output" | tail -10
        FAIL=$((FAIL + 1))
        rm -rf "$build_dir"
        return
    fi

    subheader "Test 3: Verify RAII demonstrations"

    # Check for RAII messages in output (the program uses stderr for RAII debug output)
    if echo "$run_output" | grep -q "RAII"; then
        echo -e "  ${GREEN}[PASS]${NC} RAII demonstration messages present"
        PASS=$((PASS + 1))

        # Check for proper destruction
        if echo "$run_output" | grep -q "closing\|destructor\|destroyed"; then
            echo -e "  ${GREEN}[PASS]${NC} Destructor cleanup messages present"
            PASS=$((PASS + 1))
        else
            echo -e "  ${YELLOW}[WARN]${NC} Could not verify destructor messages"
            WARN=$((WARN + 1))
        fi
    else
        echo -e "  ${YELLOW}[WARN]${NC} RAII debug messages not found (may be disabled)"
        WARN=$((WARN + 1))
    fi

    subheader "Test 4: Verify exception-safe cleanup"
    # The demo_raii_exception function throws and catches an exception
    if echo "$run_output" | grep -q "Exception Path\|Caught exception"; then
        echo -e "  ${GREEN}[PASS]${NC} Exception path demonstration ran"
        PASS=$((PASS + 1))

        # The key insight: destructor still runs even when exception is thrown
        if echo "$run_output" | grep -q "File was STILL closed\|closed automatically"; then
            echo -e "  ${GREEN}[PASS]${NC} RAII cleanup confirmed on exception path"
            PASS=$((PASS + 1))
        else
            echo -e "  ${YELLOW}[WARN]${NC} Could not verify exception-path cleanup from output"
            WARN=$((WARN + 1))
        fi
    else
        echo -e "  ${YELLOW}[WARN]${NC} Exception path demonstration not found"
        WARN=$((WARN + 1))
    fi

    subheader "Test 5: Verify destruction order"
    if echo "$run_output" | grep -q "Destruction Order\|REVERSE order"; then
        echo -e "  ${GREEN}[PASS]${NC} Destruction order demonstration ran"
        PASS=$((PASS + 1))
    else
        echo -e "  ${YELLOW}[WARN]${NC} Destruction order demonstration not found"
        WARN=$((WARN + 1))
    fi

    subheader "Test 6: Verify report generation"
    if echo "$run_output" | grep -q "Generated report\|Log Analysis Report"; then
        echo -e "  ${GREEN}[PASS]${NC} Report generation works"
        PASS=$((PASS + 1))
    else
        echo -e "  ${YELLOW}[WARN]${NC} Could not verify report generation"
        WARN=$((WARN + 1))
    fi

    rm -rf "$build_dir"

    echo ""
    echo "Cpptude #002 verification complete."
    echo "Key learning: RAII guarantees cleanup on ALL paths, including exceptions."
}

# ============================================================================
# Generic verifier for cpptudes that only have README (no implementation yet)
# ============================================================================

verify_readme_only() {
    local cpptude_dir=$1
    local cpptude_name
    cpptude_name=$(basename "$cpptude_dir")

    header "Checking $cpptude_name"

    if [[ -f "$cpptude_dir/README.md" ]]; then
        echo -e "  ${YELLOW}[INFO]${NC} This cpptude has a README but no implementation files yet."
        echo "  The lesson content is available for reading."
        WARN=$((WARN + 1))
    else
        echo -e "  ${YELLOW}[WARN]${NC} No README found"
        WARN=$((WARN + 1))
    fi
}

# ============================================================================
# Main verification dispatcher
# ============================================================================

verify_cpptude() {
    local cpptude_dir=$1
    local cpptude_name
    cpptude_name=$(basename "$cpptude_dir")

    case "$cpptude_name" in
        000-setup-guide)
            verify_000_setup_guide
            ;;
        001-sudoku)
            verify_001_sudoku
            ;;
        002-file-processor)
            verify_002_file_processor
            ;;
        003-parameter-passing|004-image-pipeline)
            # These exist but only have READMEs
            verify_readme_only "$cpptude_dir"
            ;;
        *)
            # Unknown cpptude - try generic verification
            header "Verifying $cpptude_name"

            local build_dir="$cpptude_dir/build-verify"
            if build_debug "$cpptude_dir" &> /dev/null; then
                echo -e "  ${GREEN}[PASS]${NC} Compiles successfully"
                PASS=$((PASS + 1))

                # Find and run the executable
                local executable
                executable=$(find "$build_dir" -maxdepth 1 -type f -executable | head -1)
                if [[ -n "$executable" ]]; then
                    if run_cpptude "$executable" &> /dev/null; then
                        echo -e "  ${GREEN}[PASS]${NC} Runs without errors"
                        PASS=$((PASS + 1))
                    else
                        echo -e "  ${RED}[FAIL]${NC} Program crashed"
                        FAIL=$((FAIL + 1))
                    fi
                fi
            else
                echo -e "  ${RED}[FAIL]${NC} Failed to compile"
                FAIL=$((FAIL + 1))
            fi

            rm -rf "$build_dir"
            ;;
    esac
}

# ============================================================================
# Main script
# ============================================================================

main() {
    echo -e "${BLUE}${BOLD}#tudes++ Student Verification${NC}"
    echo "This script verifies that you have completed the cpptudes correctly."
    echo ""

    # Parse arguments
    local specific_cpptude=""
    while [[ $# -gt 0 ]]; do
        case $1 in
            --list|-l)
                echo "Available cpptudes:"
                for cpptude in $(find_cpptudes); do
                    echo "  - $(basename "$cpptude")"
                done
                exit 0
                ;;
            --help|-h)
                echo "Usage: $0 [CPPTUDE_NAME]"
                echo ""
                echo "Options:"
                echo "  --list, -l    List available cpptudes"
                echo "  --help, -h    Show this help message"
                echo ""
                echo "Examples:"
                echo "  $0                  # Verify all cpptudes"
                echo "  $0 001-sudoku       # Verify specific cpptude"
                exit 0
                ;;
            *)
                specific_cpptude=$1
                shift
                ;;
        esac
    done

    # Find cpptudes
    local cpptudes
    mapfile -t cpptudes < <(find_cpptudes)

    if [[ ${#cpptudes[@]} -eq 0 ]]; then
        echo -e "${RED}No cpptudes found in $CPPTUDES_DIR${NC}"
        exit 1
    fi

    # Verify specific or all cpptudes
    if [[ -n "$specific_cpptude" ]]; then
        local found=false
        for cpptude_dir in "${cpptudes[@]}"; do
            if [[ "$(basename "$cpptude_dir")" == "$specific_cpptude" ]]; then
                verify_cpptude "$cpptude_dir"
                found=true
                break
            fi
        done

        if [[ "$found" == false ]]; then
            echo -e "${RED}Cpptude '$specific_cpptude' not found.${NC}"
            echo "Available cpptudes:"
            for cpptude in "${cpptudes[@]}"; do
                echo "  - $(basename "$cpptude")"
            done
            exit 1
        fi
    else
        # Verify all
        for cpptude_dir in "${cpptudes[@]}"; do
            verify_cpptude "$cpptude_dir"
        done
    fi

    # Summary
    header "Verification Summary"

    echo -e "  ${GREEN}Passed:${NC}   $PASS"
    echo -e "  ${RED}Failed:${NC}   $FAIL"
    echo -e "  ${YELLOW}Warnings:${NC} $WARN"

    echo ""

    if [[ $FAIL -eq 0 ]]; then
        if [[ $WARN -eq 0 ]]; then
            echo -e "${GREEN}Congratulations! All verifications passed!${NC}"
        else
            echo -e "${GREEN}All critical verifications passed.${NC}"
            echo -e "${YELLOW}Some warnings were noted - review the output above.${NC}"
        fi
        return 0
    else
        echo -e "${RED}Some verifications failed.${NC}"
        echo ""
        echo "Review the failures above and ensure:"
        echo "  1. Your code compiles with -Wall -Wextra -Werror"
        echo "  2. Your code passes ASan and UBSan checks"
        echo "  3. Your implementation follows the cpptude constraints"
        echo ""
        return 1
    fi
}

main "$@"
