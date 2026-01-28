// sudoku.hpp — Cpptude #001: Stack Allocation & Constraint Propagation
//
// CONSTRAINT: Zero heap allocations. Everything lives on the stack.
//
// This header demonstrates:
//   - std::array for fixed-size, stack-allocated containers
//   - Bitmasks for compact set representation
//   - constexpr for compile-time computation
//   - Value semantics (pass-by-value for cheap copies)

#ifndef CPPTUDE_SUDOKU_HPP
#define CPPTUDE_SUDOKU_HPP

#include <array>
#include <bit>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace cpptude {

// =============================================================================
// CORE TYPES
// =============================================================================

// C# INSTINCT: You'd use HashSet<int> to track which digits are possible.
// C++ REALITY: A bitmask is 2 bytes vs 50+ bytes for a hash set, and
//              operations are single CPU instructions (AND, OR, popcount).
//
// Bit layout: bits 1-9 represent digits 1-9. Bit 0 is unused.
// Example: 0b0000'0010'0110 means candidates are {1, 2, 5}
//          (bits 1, 2, and 5 are set)
using CandidateSet = std::uint16_t;

// All digits 1-9 are candidates: bits 1-9 set = 0b0000'0011'1111'1110
inline constexpr CandidateSet ALL_CANDIDATES = 0b0000'0011'1111'1110;

// Convert digit (1-9) to its bit representation
[[nodiscard]] constexpr CandidateSet digit_bit(int digit) noexcept {
    return static_cast<CandidateSet>(1U << digit);
}

// Count how many candidates remain (popcount of bits 1-9)
[[nodiscard]] constexpr int candidate_count(CandidateSet candidates) noexcept {
    return std::popcount(candidates);
}

// Get the single digit if exactly one candidate remains, else 0
[[nodiscard]] constexpr int solved_digit(CandidateSet candidates) noexcept {
    // std::countr_zero gives the index of the lowest set bit
    // If popcount != 1, this isn't a solved cell, return 0
    return (std::popcount(candidates) == 1) ? std::countr_zero(candidates) : 0;
}

// =============================================================================
// COMPILE-TIME LOOKUP TABLES
// =============================================================================

// C# INSTINCT: Compute peer relationships at runtime in a static constructor.
// C++ REALITY: constexpr computes these at compile time. The values are
//              embedded directly in the binary — zero runtime initialization.

// For each cell (0-80), which row/column/box does it belong to?
[[nodiscard]] constexpr int cell_row(int cell) noexcept { return cell / 9; }
[[nodiscard]] constexpr int cell_col(int cell) noexcept { return cell % 9; }
[[nodiscard]] constexpr int cell_box(int cell) noexcept {
    return (cell_row(cell) / 3) * 3 + (cell_col(cell) / 3);
}

// Compute all 20 peer indices for a given cell (same row, col, or box, excluding self)
struct PeerList {
    std::array<std::uint8_t, 20> peers;
};

[[nodiscard]] constexpr PeerList compute_peers_for(int cell) noexcept {
    PeerList result{};
    int count = 0;
    const int row = cell_row(cell);
    const int col = cell_col(cell);
    const int box = cell_box(cell);

    for (int other = 0; other < 81; ++other) {
        if (other == cell) continue;
        if (cell_row(other) == row || cell_col(other) == col || cell_box(other) == box) {
            result.peers[count++] = static_cast<std::uint8_t>(other);
        }
    }
    return result;
}

// All peer lists, computed at compile time
[[nodiscard]] constexpr std::array<PeerList, 81> compute_all_peers() noexcept {
    std::array<PeerList, 81> result{};
    for (int cell = 0; cell < 81; ++cell) {
        result[cell] = compute_peers_for(cell);
    }
    return result;
}

inline constexpr auto PEERS = compute_all_peers();

// Verify at compile time that our peer computation is correct
static_assert(PEERS[0].peers[0] == 1, "Cell 0's first row peer should be cell 1");
static_assert(PEERS[0].peers[8] == 9, "Cell 0's first column peer should be cell 9");

// =============================================================================
// THE BOARD
// =============================================================================

// C# INSTINCT: class Board with Cell[,] cells = new Cell[9,9] — 81 heap allocations
// C++ REALITY: A struct with std::array is stack-allocated. When you write
//              "Board board{};" the 216 bytes live right here in this stack frame.
//
// Memory layout:
// ┌────────────────────────────────────────┐
// │ cells[81]: 81 × 2 bytes = 162 bytes    │
// │ row_used[9]: 9 × 2 bytes = 18 bytes    │
// │ col_used[9]: 9 × 2 bytes = 18 bytes    │
// │ box_used[9]: 9 × 2 bytes = 18 bytes    │
// └────────────────────────────────────────┘
// Total: 216 bytes. Heap allocations: 0.

struct Board {
    // Each cell stores a bitmask of candidate digits (or single digit if solved)
    std::array<CandidateSet, 81> cells;

    // Track which digits are already used in each unit (for fast constraint checking)
    std::array<CandidateSet, 9> row_used;
    std::array<CandidateSet, 9> col_used;
    std::array<CandidateSet, 9> box_used;

    // Initialize empty board: all cells have all candidates
    constexpr Board() noexcept
        : cells{}
        , row_used{}
        , col_used{}
        , box_used{}
    {
        cells.fill(ALL_CANDIDATES);
        row_used.fill(0);
        col_used.fill(0);
        box_used.fill(0);
    }
};

// C# INSTINCT: sizeof doesn't exist in C# — you don't think about object sizes.
// C++ REALITY: sizeof tells you exactly how many bytes your type occupies.
//              This knowledge is power: you know what you're copying, what fits
//              in cache, what's expensive.
static_assert(sizeof(Board) == 216, "Board should be exactly 216 bytes");

// =============================================================================
// CORE OPERATIONS (internal, defined in sudoku.cpp)
// =============================================================================

// Solve the puzzle using constraint propagation + backtracking.
// Returns solved board, or nullopt if unsolvable.
//
// C# INSTINCT: Pass Board by reference to avoid copies.
// C++ REALITY: Board is 216 bytes. Copying it is essentially free (fits in L1 cache,
//              memcpy is heavily optimized). Pass by value makes backtracking trivial:
//              each recursive call gets its own copy to mutate.
[[nodiscard]] std::optional<Board> solve(Board board) noexcept;

// =============================================================================
// PUBLIC API
// =============================================================================

// Parse input string and solve. Returns solution string or nullopt.
// Input: 81 characters, digits 1-9 or '0'/'.' for empty cells
// Output: 81 characters, all digits 1-9
[[nodiscard]] std::optional<std::string> solve_sudoku(std::string_view input) noexcept;

// Convert solved board to string representation
[[nodiscard]] std::string board_to_string(const Board& board) noexcept;

// Pretty-print board for debugging
void print_board(const Board& board);

} // namespace cpptude

#endif // CPPTUDE_SUDOKU_HPP
