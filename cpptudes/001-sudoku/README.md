# Cpptude #001: Sudoku — Stack Allocation & Constraint Propagation

| Field | Value |
|-------|-------|
| **Primary KAP** | Stack Allocation / Value Semantics (Rating 5 - Critical) |
| **Secondary Concepts** | `std::array`, `constexpr`, bitwise operations, `std::optional` |
| **Difficulty** | Intermediate |
| **Constraint** | Zero heap allocations |

---

## The C# Instinct vs C++ Reality

**In C#**, you'd naturally write:

```csharp
class Cell {
    public int? Value { get; set; }
    public HashSet<int> Candidates { get; } = new();
}

class Board {
    private Cell[,] cells = new Cell[9, 9];  // 81 heap objects!
}
```

This creates **81+ heap allocations**, scattered across memory, managed by the GC.

**In C++**, we write:

```cpp
struct Board {
    std::array<uint16_t, 81> cells;     // 162 bytes, contiguous
    std::array<uint16_t, 9> row_used;   // 18 bytes
    std::array<uint16_t, 9> col_used;   // 18 bytes
    std::array<uint16_t, 9> box_used;   // 18 bytes
};
// Total: 216 bytes. On the stack. Zero heap. In L1 cache.
```

When you write `Board board{};`, those 216 bytes live **right here, in this stack frame**. No `new`. No GC. No pointer chasing.

---

## Building & Running

### Basic Build

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic sudoku.cpp main.cpp -o sudoku
./sudoku
```

### Build with Sanitizers (Recommended for Learning)

```bash
g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -Wall -Wextra sudoku.cpp main.cpp -o sudoku
./sudoku
```

The sanitizers will catch:
- Use-after-free
- Buffer overflows
- Undefined behavior (signed overflow, null dereference, etc.)

### Verify Zero Heap Allocations

```bash
# Using heaptrack (recommended)
heaptrack ./sudoku
heaptrack --analyze heaptrack.sudoku.*.gz

# Using valgrind
valgrind --tool=massif ./sudoku
ms_print massif.out.*
```

You should see **zero heap allocations** from our code (the standard library may allocate for I/O).

---

## The Code, Explained

### Part 1: Candidate Sets as Bitmasks

**C# Instinct:** Use `HashSet<int>` to track candidate digits.

**C++ Reality:** For small, bounded domains (digits 1-9), a bitmask is superior:

```cpp
using CandidateSet = std::uint16_t;

// Bit layout: bits 1-9 represent digits 1-9
// Example: 0b0000'0010'0110 = {1, 2, 5}
//          (bits 1, 2, and 5 are set)

// All digits possible: bits 1-9 set
constexpr CandidateSet ALL_CANDIDATES = 0b0000'0011'1111'1110;
```

**Why this matters:**

| Operation | HashSet | Bitmask |
|-----------|---------|---------|
| Memory | ~50 bytes | 2 bytes |
| "Contains 5?" | Hash lookup | `(bits & (1 << 5)) != 0` |
| "Remove 5" | Hash remove | `bits &= ~(1 << 5)` |
| "Count" | O(n) iteration | `std::popcount(bits)` |
| Allocation | Heap | Stack |

A bitmask operation is literally **one CPU instruction**. No function calls, no memory access, no cache misses.

### Part 2: The Board Structure

```cpp
struct Board {
    std::array<CandidateSet, 81> cells;
    std::array<CandidateSet, 9> row_used;
    std::array<CandidateSet, 9> col_used;
    std::array<CandidateSet, 9> box_used;
};

static_assert(sizeof(Board) == 216, "Board should be exactly 216 bytes");
```

**Memory layout visualization:**

```
Board board{};  // Lives on the stack, in this function's frame

Stack:
┌───────────────────────────────────────────┐
│ cells[0]:  candidates for cell 0 (2 bytes)│
│ cells[1]:  candidates for cell 1 (2 bytes)│
│ ...                                       │
│ cells[80]: candidates for cell 80         │
│ row_used[0..8]: 18 bytes                  │
│ col_used[0..8]: 18 bytes                  │
│ box_used[0..8]: 18 bytes                  │
└───────────────────────────────────────────┘
Total: 216 bytes, contiguous, cache-friendly

Heap:
(empty — we allocated nothing)
```

**C# Contrast:**

```
C# new Cell[9,9]:

Heap:
┌──────────┐  ┌──────────┐  ┌──────────┐
│ Cell obj │  │ Cell obj │  │ Cell obj │ ... × 81
│ (header) │  │ (header) │  │ (header) │
│ Value    │  │ Value    │  │ Value    │
│ HashSet──┼─→│ (more    │  │ (more    │
└──────────┘  │ heap)    │  │ heap)    │
              └──────────┘  └──────────┘

Result: 81+ allocations, scattered memory, cache misses, GC pressure
```

### Part 3: Compile-Time Lookup Tables

**C# Instinct:** Compute peer relationships at runtime, maybe in a static constructor.

**C++ Reality:** `constexpr` computes values at compile time:

```cpp
// This function runs at COMPILE TIME
constexpr PeerList compute_peers_for(int cell) noexcept {
    PeerList result{};
    int count = 0;
    // ... compute 20 peers for this cell ...
    return result;
}

// This array is embedded in the binary — zero runtime cost
inline constexpr auto PEERS = compute_all_peers();

// The compiler verifies our logic is correct!
static_assert(PEERS[0].peers[0] == 1, "Cell 0's first peer should be cell 1");
```

When you run the program, `PEERS` is already computed. It's data in the `.rodata` section, not code that runs.

### Part 4: Backtracking with Value Semantics

**C# Instinct:** Pass `Board` by reference, manually clone when needed.

```csharp
Board Solve(Board board) {
    // Need to try different values...
    var copy = board.Clone();  // Heap allocation!
    // ...
}
```

**C++ Reality:** Pass by value. The copy happens automatically and is trivially fast:

```cpp
std::optional<Board> solve(Board board) {  // Pass by value = copy
    // board is a local copy, 216 bytes on the stack

    for (int digit : candidates) {
        Board copy = board;  // Another copy, still on stack, still fast

        if (assign(copy, cell, digit)) {
            if (auto result = solve(copy)) {  // Recursive call with copy
                return result;
            }
        }
        // copy goes out of scope here — automatic cleanup, no delete
    }

    return std::nullopt;
}
```

**Why copying 216 bytes is fast:**

1. **L1 cache:** 216 bytes fits entirely in L1 cache (typically 32-64 KB)
2. **memcpy optimization:** Compilers turn `Board copy = board;` into highly optimized SIMD instructions
3. **No indirection:** The bytes are right there, not behind a pointer
4. **Predictable:** CPU can prefetch because access pattern is obvious

Benchmark: copying a 216-byte struct is typically **< 10 nanoseconds**.

### Part 5: std::optional for "No Solution"

**C# Instinct:** Return `null` or throw an exception.

```csharp
Board? Solve(Board board) {
    // ...
    return null;  // No solution
}
```

**C++ Reality:** `std::optional<T>` makes "might not have a value" explicit in the type:

```cpp
[[nodiscard]] std::optional<Board> solve(Board board) noexcept {
    // ...
    return std::nullopt;  // No solution — not an error, just a fact
}

// Caller must handle both cases:
if (auto result = solve(board)) {
    // *result is the solved board
} else {
    // No solution exists
}
```

**Why not exceptions?**

- Backtracking expects many "failures" — they're not exceptional
- Exceptions have runtime overhead (stack unwinding)
- `noexcept` tells the compiler no exceptions will be thrown, enabling optimizations

---

## Deep Dive: Why Stack Allocation Matters

### Memory Access Latency

| Location | Latency |
|----------|---------|
| L1 cache | ~1 ns |
| L2 cache | ~3 ns |
| L3 cache | ~10 ns |
| Main RAM | ~100 ns |
| Heap allocation | ~100-1000 ns |

Our 216-byte `Board`:
- Fits in L1 cache
- Accessed sequentially (cache-friendly)
- No allocation overhead

C# `Cell[9,9]`:
- 81 objects scattered in heap
- Each access may be a cache miss
- GC must track all 81 objects

### GC-Free Means Predictable

In C#, the GC can pause your program at any time to collect garbage. In a game or real-time system, this causes stutters.

Our C++ solver:
- Allocates nothing on the heap
- Has deterministic timing
- Can solve millions of puzzles with zero GC pauses (because there's no GC)

---

## For the C# Developer: Key Takeaways

After completing this Cpptude, you should understand:

### 1. Stack Allocation is the Default

```cpp
Board board{};  // On the stack. Dies when scope exits.
```

No `new`. No `delete`. No GC. The bytes live right here.

### 2. `std::array` is Your Friend

```cpp
std::array<int, 9> arr;  // Fixed size, stack allocated, bounds-checked in debug
```

Not `int arr[9]` (C-style, no bounds checking, decays to pointer).
Not `std::vector` (heap allocated, dynamic size).

### 3. Small Structs Should Be Copied

```cpp
Board copy = board;  // 216 bytes, trivially fast
```

Don't fear copies of small data. Fear pointer indirection and heap allocation.

### 4. Bitmasks Beat Hash Sets for Small Domains

```cpp
uint16_t candidates = 0b0000'0010'0110;  // {1, 2, 5}
candidates &= ~(1 << 2);                  // Remove 2: {1, 5}
```

For domains < 64 elements, a bitmask is faster, smaller, and simpler.

### 5. `constexpr` Moves Work to Compile Time

```cpp
constexpr auto PEERS = compute_all_peers();  // Computed by compiler
```

If a value can be known at compile time, let the compiler compute it.

### 6. No GC Means No GC Pauses

This solver runs in constant memory with deterministic timing. You control exactly when memory is allocated and freed.

---

## Design Decisions (Assumed)

The following design choices were made without explicit expert guidance:

1. **Flat array vs. dict-based grid:** We use `cells[81]` instead of Norvig's dict with string keys (`'A1'`, `'C7'`). This teaches cache-friendly flat indexing but diverges from the Python structure.

2. **Raw bitmasks vs. `std::bitset<9>`:** We use `uint16_t` for maximum transparency and minimal abstraction. `std::bitset` would be more readable but hides the bit manipulation we're trying to teach.

3. **Pass-by-value backtracking:** We copy the entire board at each recursive call instead of mutate-and-undo. This is pedagogically simpler and fast enough (216 bytes copies in ~10ns).

4. **`std::optional` vs. `std::expected`:** We use `std::optional<Board>` for "no solution" instead of `std::expected` with error information. This keeps the focus on the primary KAP (stack allocation).

---

## Next Step Challenge

**Challenge: Sudoku Generator**

Write a function that generates a valid Sudoku puzzle with a unique solution:

```cpp
[[nodiscard]] std::string generate_puzzle(
    int given_count,      // How many cells to reveal (17-30 typical)
    std::mt19937& rng     // Random number generator
) noexcept;
```

Requirements:
- Generated puzzle must have exactly one solution
- Use the solver to verify uniqueness
- **Constraint: Still zero heap allocations**

This exercises:
- `<random>` header for random number generation
- Using the solver as a subroutine
- Removing clues while maintaining unique solvability
