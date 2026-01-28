// sudoku.cpp — Cpptude #001: Stack Allocation & Constraint Propagation
//
// Implementation of the Sudoku solver. See sudoku.hpp for type definitions
// and the educational commentary.
//
// BUILD:
//   g++ -std=c++20 -O2 -Wall -Wextra -Werror -Wpedantic sudoku.cpp main.cpp -o sudoku
//
// BUILD WITH SANITIZERS (recommended for learning):
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined -Wall -Wextra -Werror -Wpedantic sudoku.cpp main.cpp -o sudoku
//
// VERIFY ZERO HEAP ALLOCATIONS:
//   valgrind --tool=massif ./sudoku
//   heaptrack ./sudoku

#include "sudoku.hpp"

#include <iostream>
#include <iomanip>

namespace cpptude {

// =============================================================================
// CONSTRAINT PROPAGATION
// =============================================================================

// Eliminate a candidate digit from a cell.
//
// This is the heart of constraint propagation. When we eliminate a digit:
// 1. If the cell becomes empty (no candidates), we have a contradiction
// 2. If the cell has exactly one candidate left, it's solved — propagate to peers
// 3. Check if any unit now has only one place for this digit (hidden single)
//
// C# INSTINCT: Throw an exception on contradiction.
// C++ REALITY: Return bool. Exceptions have overhead and aren't appropriate for
//              expected control flow (backtracking expects many contradictions).

[[nodiscard]] constexpr bool eliminate(Board& board, int cell, int digit) noexcept {
    const CandidateSet bit = digit_bit(digit);

    // Already eliminated? Nothing to do.
    if ((board.cells[cell] & bit) == 0) {
        return true;
    }

    // Remove this digit from the cell's candidates
    board.cells[cell] &= ~bit;

    const CandidateSet remaining = board.cells[cell];

    // CONTRADICTION: No candidates left
    if (remaining == 0) {
        return false;
    }

    // NAKED SINGLE: If only one candidate remains, this cell is solved.
    // Eliminate that digit from all 20 peers.
    if (std::popcount(remaining) == 1) {
        const int solved = std::countr_zero(remaining);

        // Update unit tracking
        const int row = cell_row(cell);
        const int col = cell_col(cell);
        const int box = cell_box(cell);
        board.row_used[row] |= digit_bit(solved);
        board.col_used[col] |= digit_bit(solved);
        board.box_used[box] |= digit_bit(solved);

        // Propagate to peers
        for (const auto peer : PEERS[cell].peers) {
            if (!eliminate(board, peer, solved)) {
                return false;
            }
        }
    }

    return true;
}

// Assign a digit to a cell by eliminating all other candidates.
//
// C# INSTINCT: cell.Value = digit; cell.Candidates.Clear();
// C++ REALITY: Assignment means "eliminate everything except this digit."
//              The constraint propagation does the heavy lifting.

[[nodiscard]] constexpr bool assign(Board& board, int cell, int digit) noexcept {
    // Eliminate all digits except the one we're assigning
    for (int d = 1; d <= 9; ++d) {
        if (d != digit) {
            if (!eliminate(board, cell, d)) {
                return false;
            }
        }
    }
    return true;
}

// =============================================================================
// BACKTRACKING SEARCH
// =============================================================================

// Find the cell with the fewest candidates (Minimum Remaining Values heuristic).
// Returns -1 if all cells are solved.
//
// MRV is crucial for performance: by choosing the most constrained cell first,
// we detect contradictions earlier and prune the search tree more effectively.

[[nodiscard]] constexpr int find_mrv_cell(const Board& board) noexcept {
    int best_cell{-1};
    int best_count{10}; // More than any possible candidate count

    for (int cell = 0; cell < 81; ++cell) {
        const int count = candidate_count(board.cells[cell]);

        // Skip solved cells (count == 1)
        if (count > 1 && count < best_count) {
            best_count = count;
            best_cell = cell;

            // Can't do better than 2 candidates
            if (count == 2) break;
        }
    }

    return best_cell;
}

// Solve using constraint propagation + backtracking.
//
// C# INSTINCT: Pass Board by reference, clone when needed for backtracking.
// C++ REALITY: Pass by value. Each recursive call gets its own 216-byte copy.
//              This is FAST because:
//              - 216 bytes fits entirely in L1 cache
//              - memcpy is heavily optimized (often uses SIMD)
//              - No heap allocation, no pointer indirection
//              - Code is simpler: no manual save/restore logic

[[nodiscard]] std::optional<Board> solve(Board board) noexcept {
    // Find the most constrained unsolved cell
    const int cell = find_mrv_cell(board);

    // All cells solved? We're done!
    if (cell == -1) {
        return board;
    }

    // Try each candidate digit for this cell
    // C# INSTINCT: foreach (var digit in cell.Candidates)
    // C++ REALITY: Iterate over set bits in the bitmask
    CandidateSet candidates = board.cells[cell];

    while (candidates != 0) {
        // Extract lowest set bit's position (the next digit to try)
        const int digit = std::countr_zero(candidates);

        // Clear that bit so we don't try it again
        candidates &= candidates - 1;

        // Make a copy and try this assignment
        // C# INSTINCT: var copy = board.Clone(); — heap allocation!
        // C++ REALITY: Board copy = board; — stack copy, 216 bytes, trivially fast
        Board copy = board;

        if (assign(copy, cell, digit)) {
            // Assignment succeeded (no immediate contradiction)
            // Recurse to solve the rest
            if (auto result = solve(copy)) {
                return result;
            }
        }
        // Assignment failed or recursion found no solution — try next digit
    }

    // No digit worked for this cell — backtrack
    return std::nullopt;
}

// =============================================================================
// PUBLIC API
// =============================================================================

[[nodiscard]] std::optional<std::string> solve_sudoku(std::string_view input) noexcept {
    // Validate input length
    if (input.size() != 81) {
        return std::nullopt;
    }

    // Parse input into a board
    Board board{};

    for (int cell = 0; cell < 81; ++cell) {
        const char c = input[cell];

        if (c >= '1' && c <= '9') {
            // Given digit — assign it
            const int digit = c - '0';
            if (!assign(board, cell, digit)) {
                // Contradiction in input — invalid puzzle
                return std::nullopt;
            }
        } else if (c != '0' && c != '.') {
            // Invalid character
            return std::nullopt;
        }
        // '0' or '.' means empty — leave all candidates
    }

    // Solve!
    auto result = solve(board);

    if (!result) {
        return std::nullopt;
    }

    return board_to_string(*result);
}

[[nodiscard]] std::string board_to_string(const Board& board) noexcept {
    std::string result{};
    result.reserve(81);

    for (int cell = 0; cell < 81; ++cell) {
        const int digit = solved_digit(board.cells[cell]);
        result += static_cast<char>('0' + digit);
    }

    return result;
}

void print_board(const Board& board) {
    for (int row = 0; row < 9; ++row) {
        if (row > 0 && row % 3 == 0) {
            std::cout << "------+-------+------\n";
        }

        for (int col = 0; col < 9; ++col) {
            if (col > 0 && col % 3 == 0) {
                std::cout << " |";
            }

            const int cell = row * 9 + col;
            const int digit = solved_digit(board.cells[cell]);

            if (digit > 0) {
                std::cout << ' ' << digit;
            } else {
                std::cout << " .";
            }
        }
        std::cout << '\n';
    }
}

} // namespace cpptude
