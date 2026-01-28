# Cpptude Lesson Specification: Sudoku

## Metadata

| Field | Value |
|-------|-------|
| **Lesson Number** | 001 |
| **Title** | Sudoku: Stack Allocation & Constraint Propagation |
| **Primary KAP** | Stack allocation / Value semantics (Rating 5 - Critical) |
| **Secondary Concepts** | `std::array`, `constexpr`, bitwise operations, `std::optional` |
| **Difficulty** | Intermediate |
| **Pytude Reference** | [norvig/pytudes - Sudoku.ipynb](https://github.com/norvig/pytudes/blob/main/ipynb/Sudoku.ipynb) |

---

## The Spec (Language-Agnostic)

### Problem Definition

**Input:** A string of 81 characters representing a 9×9 Sudoku grid, where:
- Digits `1-9` represent filled cells
- `0` or `.` represents empty cells
- Reading order: left-to-right, top-to-bottom

**Output:** The solved grid as an 81-character string, or indication that no solution exists.

**Constraints:**
- Standard Sudoku rules: each row, column, and 3×3 box must contain digits 1-9 exactly once
- Input is guaranteed to be valid (no initial conflicts)
- Exactly one solution exists (for well-formed puzzles)

### Example

```
Input:  "530070000600195000098000060800060003400803001700020006060000280000419005000080079"
Output: "534678912672195348198342567859761423426853791713924856961537284287419635345286179"
```

---

## The C# Trap

### What C# Instincts Will Suggest

A C# developer will naturally reach for:

```csharp
// C# INSTINCT: Classes on the heap, nullable references
class Cell {
    public int? Value { get; set; }
    public HashSet<int> Candidates { get; } = new();
}

class Board {
    private Cell[,] cells = new Cell[9, 9];  // 81 heap allocations!

    public Board() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                cells[r, c] = new Cell();  // Each cell is a heap object
    }
}
```

**Problems this creates:**
1. **81+ heap allocations** for a simple 9×9 grid
2. **Pointer chasing** — `cells[r,c]` dereferences a pointer to reach the Cell
3. **Cache hostility** — Cell objects scattered across heap
4. **GC pressure** — solving many puzzles churns memory
5. **Recursive backtracking** creates new `Board` copies on heap

### How We Block This

1. **Constraint: Zero heap allocations** — All state must fit in stack-allocated types
2. **Constraint: No `std::vector`** — Forces `std::array` usage
3. **Constraint: No `new`/`make_unique`** — Enforced by code review / grep in CI
4. **Verification: Run under ASan/heap profiler** — Any allocation fails the exercise

### The Bait & Switch

The learner will try:
```cpp
// ATTEMPT 1: Direct translation
std::vector<std::vector<std::optional<int>>> board(9, std::vector<std::optional<int>>(9));
// BLOCKED: std::vector allocates on heap
```

```cpp
// ATTEMPT 2: Smart pointers
auto board = std::make_unique<std::array<std::array<int, 9>, 9>>();
// BLOCKED: make_unique allocates on heap
```

```cpp
// ATTEMPT 3: Recursive copy
Board solve(Board board) {  // Copies entire board each call
    // ...
    return solve(board);    // More copies!
}
// BLOCKED: Performance benchmark shows 1000x slowdown
```

---

## Targeted C++ Axioms

### Primary: Stack Allocation (Rating 5)

> **The C# Instinct:** Objects are allocated on the heap and managed by GC. Creating an object means `new`.
>
> **The C++ Reality:** Value types (including `struct`/`class` without `new`) live on the stack. The stack is:
> - **Fast:** No allocation overhead, just move the stack pointer
> - **Scoped:** Lifetime tied to enclosing `{}` block
> - **Cache-friendly:** Contiguous memory, predictable access patterns
> - **Limited:** Typically 1-8 MB; don't put megabytes on the stack
>
> **The C++ Axiom:** Prefer stack allocation. Use the heap only when:
> 1. Size is unknown at compile time
> 2. Size exceeds stack limits
> 3. Object must outlive current scope
> 4. Polymorphism requires pointer indirection

### Secondary: `std::array` vs C-arrays (Rating 4)

> **The C# Instinct:** Arrays are objects with `.Length`, bounds checking, and passed by reference.
>
> **The C++ Reality:** C-style arrays (`int arr[9]`) decay to pointers, lose size information, and have no bounds checking. They are a legacy footgun.
>
> **The C++ Axiom:** Use `std::array<T, N>` for fixed-size arrays. It:
> - Preserves size in the type (`std::array<int, 9>`)
> - Bounds-checks in debug mode
> - Is a value type (copies element-by-element)
> - Has zero overhead vs C-array in release mode

### Secondary: Bitwise Operations for Candidate Sets (Rating 3)

> **The C# Instinct:** Use `HashSet<int>` for candidate tracking. It's clean and readable.
>
> **The C++ Reality:** A `HashSet` equivalent (`std::unordered_set<int>`) allocates heap memory and has overhead per operation. For small, bounded domains (like Sudoku digits 1-9), a bitmask is:
> - **Stack-allocated:** Single `uint16_t`
> - **Cache-friendly:** 2 bytes vs 50+ bytes for a hash set
> - **Fast:** AND/OR/XOR are single CPU instructions
>
> **The C++ Axiom:** For small enumerable domains, represent sets as bitmasks. Digit `d` present ↔ bit `d` is set.

### Secondary: `constexpr` for Compile-Time Tables (Rating 2)

> **The C# Instinct:** Lookup tables are initialized at runtime, perhaps in a static constructor.
>
> **The C++ Reality:** `constexpr` allows computation at compile time. The peer/unit relationships in Sudoku are fixed — compute them once, embed in the binary.
>
> **The C++ Axiom:** If a value can be computed at compile time, use `constexpr`. Zero runtime cost.

---

## Implementation Strategy

### Data Representation

```cpp
// 81 cells, each cell is a bitmask of possible digits (bits 1-9)
// Bit 0 unused; bit d set means digit d is a candidate
using CandidateSet = uint16_t;  // 0b0000'0011'1111'1110 = all digits possible

struct Board {
    std::array<CandidateSet, 81> cells;

    // Precomputed constraint masks
    std::array<uint16_t, 9> row_solved;  // Which digits are solved in each row
    std::array<uint16_t, 9> col_solved;  // Which digits are solved in each column
    std::array<uint16_t, 9> box_solved;  // Which digits are solved in each box
};
```

**Memory layout:**
```
Board (stack-allocated):
┌────────────────────────────────────┐
│ cells[81]: 81 × 2 bytes = 162 bytes│
│ row_solved[9]: 18 bytes            │
│ col_solved[9]: 18 bytes            │
│ box_solved[9]: 18 bytes            │
└────────────────────────────────────┘
Total: 216 bytes on stack. Zero heap.
```

### Algorithm Approach

1. **Constraint Propagation:** When a cell is solved, eliminate that digit from all peers (same row, column, box). Use bitmask AND operations.

2. **Naked Singles:** If a cell has only one candidate (popcount = 1), it's solved.

3. **Hidden Singles:** If a digit appears in only one cell within a unit, that cell is solved.

4. **Backtracking:** When stuck, pick the cell with fewest candidates (MRV heuristic), try each candidate, recurse. Use **in-place mutation with undo** rather than copying the board.

### Key Functions

```cpp
// All constexpr — computed at compile time
constexpr auto PEERS = compute_peers();      // peers[cell] = array of 20 peer indices
constexpr auto UNITS = compute_units();      // units[cell] = array of 3 unit indices
constexpr auto UNIT_CELLS = compute_unit_cells(); // unit_cells[unit] = array of 9 cell indices

// Core operations
[[nodiscard]] constexpr bool eliminate(Board& board, int cell, int digit) noexcept;
[[nodiscard]] constexpr bool propagate(Board& board) noexcept;
[[nodiscard]] constexpr std::optional<Board> solve(Board board) noexcept;

// Entry point
[[nodiscard]] std::optional<std::string> solve_sudoku(std::string_view input) noexcept;
```

### Backtracking Strategy: Mutate-and-Undo vs Copy

**Option A: Copy board at each decision point**
```cpp
std::optional<Board> solve(Board board) {  // Pass by value = copy
    // Try each candidate...
    if (auto result = solve(board)) return result;  // Another copy
}
```
- Simple but creates many copies
- Each copy: 216 bytes × stack depth (up to ~60)
- Total stack: ~13 KB — acceptable

**Option B: Mutate in place, undo on backtrack**
```cpp
bool solve(Board& board) {  // Pass by reference
    // Save state before mutation
    auto saved_cell = board.cells[idx];
    // Try each candidate, undo if fails
    board.cells[idx] = saved_cell;  // Restore
}
```
- More complex but minimal memory
- Must carefully track what to undo

**Decision:** Use **Option A (copy)** for clarity. The 216-byte board is small enough that copies are cheap, and the code is cleaner. This is a good example of **"don't over-optimize prematurely"** — the copy fits in cache and memcpy is fast.

---

## Requires Expert Input

The following decisions need human review before proceeding to implementation:

1. **Algorithm Fidelity vs. C++ Idiom**
   - Norvig's Python version uses dict-based grid with string keys (`'A1'`, `'C7'`).
   - The C++ version uses flat array with integer indices.
   - **Assumed:** Divergence is acceptable because it teaches cache-friendly flat indexing.
   - **Needs validation:** Is this divergence pedagogically correct, or should we match Norvig's structure more closely?

2. **Bitmask Complexity**
   - Bitmask operations may be unfamiliar to C# developers.
   - **Assumed:** This is a valuable secondary lesson worth the complexity.
   - **Alternative:** Use `std::bitset<9>` for readability, accepting slight overhead.
   - **Needs validation:** Which approach better serves the learning goals?

3. **`constexpr` Depth**
   - How much of the solver should be `constexpr`? Could make the entire solver compile-time capable.
   - **Assumed:** Make lookup tables `constexpr`, keep solver runtime for simplicity.
   - **Alternative:** Full `consteval` solver as an advanced challenge.
   - **Needs validation:** Is compile-time solving a distraction or a valuable extension?

4. **Error Handling**
   - Invalid input handling: `std::optional` return vs. exceptions vs. assert.
   - **Assumed:** Use `std::optional<std::string>` for "no solution" and undefined behavior for malformed input (with debug asserts).
   - **Needs validation:** Should we demonstrate `std::expected` instead since it's a Tier 2 concept?

---

## Verification Criteria

### Functional
- [ ] Solves all 95 "hard" puzzles from Norvig's test set
- [ ] Solves "world's hardest Sudoku" in < 100ms
- [ ] Returns `std::nullopt` for unsolvable puzzles

### Memory
- [ ] Zero heap allocations (verified via ASan/heaptrack)
- [ ] Stack usage < 64 KB for any puzzle

### Performance
- [ ] Solves easy puzzles in < 1ms
- [ ] Solves hard puzzles in < 100ms
- [ ] Benchmark comparison: C++ vs equivalent C# solution (expect 10-100x speedup)

### Compilation
- [ ] Compiles clean with `-Wall -Wextra -Wpedantic`
- [ ] No warnings on GCC 12+, Clang 15+, MSVC 19.30+
- [ ] `constexpr` tables validated with `static_assert`

---

## For the C# Developer: Key Takeaways

After completing this Cpptude, you should understand:

1. **Stack allocation is the default** — Objects without `new` live on the stack and die at scope exit.

2. **`std::array` is your friend** — Fixed-size, stack-allocated, bounds-checked in debug, zero overhead in release.

3. **Small data structures should be copied** — A 216-byte struct copies faster than managing pointers.

4. **Bitwise operations are not scary** — For small domains, a bitmask is cleaner and faster than a hash set.

5. **`constexpr` moves work to compile time** — Lookup tables computed by the compiler, not at runtime.

6. **No GC means no GC pauses** — This solver runs in constant memory with deterministic timing.

---

## Next Step Challenge

After completing the basic solver, extend it:

**Challenge: Sudoku Generator**

Write a function that generates a valid Sudoku puzzle with a unique solution and exactly N given digits.

```cpp
[[nodiscard]] std::string generate_puzzle(int given_count, std::mt19937& rng) noexcept;
```

This exercises:
- Random number generation (`<random>` header)
- The solver as a subroutine (uniqueness checking)
- Constraint: Still zero heap allocations
