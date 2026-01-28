# Cpptude #001: Sudoku — Stack Allocation & Constraint Propagation

| Field | Value |
|-------|-------|
| **Primary KAP** | Stack Allocation / Value Semantics (Rating 5 - Critical) |
| **Secondary Concepts** | `std::array`, `constexpr`, bitwise operations, `std::optional` |
| **Difficulty** | Intermediate |
| **Constraint** | Zero heap allocations |

---

## Overview

This cpptude teaches **stack allocation and value semantics** — how C++ places data directly in the function's stack frame, how scope governs lifetime, and why this produces fast, predictable code with zero garbage collection.

The Sudoku solver is the vehicle: its 216-byte board fits entirely on the stack, copies are trivially fast, and the entire solve runs with zero heap allocations.

> **C# Bridge:** If you're coming from C#, this cpptude targets the instinct to `new` everything onto the heap. In C#, `new Cell[9, 9]` creates 81+ heap-allocated objects managed by the GC. In C++, `Board board{};` places 216 contiguous bytes on the stack — no allocator, no GC, no pointer chasing.

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

#### The C++ Concept

For small, bounded domains (digits 1-9), a bitmask is the natural representation in C++. Each bit maps to a digit, and set operations become single CPU instructions:

```cpp
using CandidateSet = std::uint16_t;

// Bit layout: bits 1-9 represent digits 1-9
// Example: 0b0000'0010'0110 = {1, 2, 5}
//          (bits 1, 2, and 5 are set)

// All digits possible: bits 1-9 set
constexpr CandidateSet ALL_CANDIDATES = 0b0000'0011'1111'1110;
```

Why this matters for performance:

| Operation | Hash-based set | Bitmask |
|-----------|----------------|---------|
| Memory | ~50 bytes | 2 bytes |
| "Contains 5?" | Hash lookup | `(bits & (1 << 5)) != 0` |
| "Remove 5" | Hash remove | `bits &= ~(1 << 5)` |
| "Count" | O(n) iteration | `std::popcount(bits)` |
| Allocation | Heap | Stack |

A bitmask operation is literally **one CPU instruction**. No function calls, no memory access, no cache misses.

#### The C# Bridge

In C#, you'd reach for `HashSet<int>` to track candidate digits. That's a heap-allocated, dynamically-sized collection — perfectly reasonable in managed code where the GC handles cleanup and the runtime optimizes allocation. In C++, for a domain this small, a bitmask is idiomatic: it's a value type, fits in a register, and requires zero allocation.

### Part 2: The Board Structure

#### The C++ Concept

In C++, `struct` members are laid out contiguously in memory. When you declare a local variable of struct type, the entire struct lives on the stack — no allocation, no indirection:

```cpp
struct Board {
    std::array<CandidateSet, 81> cells;
    std::array<CandidateSet, 9> row_used;
    std::array<CandidateSet, 9> col_used;
    std::array<CandidateSet, 9> box_used;
};

static_assert(sizeof(Board) == 216, "Board should be exactly 216 bytes");
```

Memory layout:

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

`static_assert` verifies the layout at compile time. If the struct size changes (e.g., someone adds a field), the build fails immediately.

#### The C# Bridge

In C#, `new Cell[9, 9]` creates 81 separate objects on the managed heap, each with its own object header and GC tracking:

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

The C++ version has one contiguous block of 216 bytes. The C# version has 81+ scattered heap objects. This difference is fundamental: C++ gives you direct control over memory layout.

### Part 3: Compile-Time Lookup Tables

#### The C++ Concept

`constexpr` functions execute at compile time. The compiler evaluates them and embeds the results directly into the binary as static data:

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

When you run the program, `PEERS` is already computed. It's data in the `.rodata` section, not code that runs. `static_assert` provides compile-time verification that the computed values are correct.

#### The C# Bridge

In C#, you'd compute peer relationships at runtime — perhaps in a static constructor or a `Lazy<T>`. The data is correct but has runtime initialization cost and lives on the heap. C++ `constexpr` moves this work entirely to the compiler: zero runtime cost, zero allocation, and compile-time verification via `static_assert`.

### Part 4: Backtracking with Value Semantics

#### The C++ Concept

In C++, passing a struct by value creates an independent copy. For small structs (like our 216-byte Board), this is trivially fast and eliminates the need for explicit clone/copy methods:

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

Why copying 216 bytes is fast:

1. **L1 cache:** 216 bytes fits entirely in L1 cache (typically 32-64 KB)
2. **memcpy optimization:** Compilers turn `Board copy = board;` into highly optimized SIMD instructions
3. **No indirection:** The bytes are right there, not behind a pointer
4. **Predictable:** CPU can prefetch because access pattern is obvious

Benchmark: copying a 216-byte struct is typically **< 10 nanoseconds**.

#### The C# Bridge

In C#, you'd pass `Board` by reference and explicitly clone when needed:

```csharp
Board Solve(Board board) {
    // Need to try different values...
    var copy = board.Clone();  // Heap allocation!
    // ...
}
```

Each `Clone()` allocates on the heap. In C++, `Board copy = board;` is a stack copy — 216 bytes memcpy'd with no allocator involved. C++ value semantics make copies cheap and automatic for small types, eliminating the need for Clone patterns.

### Part 5: std::optional for "No Solution"

#### The C++ Concept

`std::optional<T>` encodes "might not have a value" directly in the type system. The caller must handle both cases — the type forces it:

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

Why not exceptions? Backtracking expects many "failures" — they're not exceptional. Exceptions have runtime overhead (stack unwinding), and `noexcept` tells the compiler no exceptions will be thrown, enabling optimizations.

#### The C# Bridge

In C#, you'd return `null` or throw an exception:

```csharp
Board? Solve(Board board) {
    // ...
    return null;  // No solution
}
```

C# nullable reference types (`Board?`) are similar in intent, but `null` is pervasive in C# — any reference type can be null whether you intended it or not. C++ `std::optional` is opt-in: a `Board` can never be "null," and only `std::optional<Board>` communicates the possibility of absence.

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

### Deterministic Timing

Stack-allocated data has deterministic lifetime: it is created when the scope is entered and destroyed when the scope exits. There is no background process managing memory, no unpredictable pauses, and no fragmentation.

This is why stack allocation matters for C++: it gives you **predictable, fast, automatically-managed memory** for data whose lifetime matches a scope.

> **C# Bridge:** In C#, the GC can pause your program at any time to collect garbage. In a game or real-time system, this causes stutters. The C++ solver allocates nothing on the heap, has deterministic timing, and can solve millions of puzzles with zero GC pauses — because there is no GC.

---

## Key Takeaways

After completing this Cpptude, you should understand:

### 1. Stack Allocation is the Default

```cpp
Board board{};  // On the stack. Dies when scope exits.
```

No `new`. No `delete`. No GC. The bytes live right here. In C++, stack allocation is the default for local variables — use it unless you have a reason not to.

### 2. `std::array` is Your Friend

```cpp
std::array<int, 9> arr;  // Fixed size, stack allocated, bounds-checked in debug
```

Not `int arr[9]` (C-style, no bounds checking, decays to pointer).
Not `std::vector` (heap allocated, dynamic size).

`std::array` gives you fixed-size, stack-allocated storage with bounds checking in debug builds.

### 3. Small Structs Should Be Copied

```cpp
Board copy = board;  // 216 bytes, trivially fast
```

Don't fear copies of small data. Fear pointer indirection and heap allocation. Value semantics — passing and copying small structs — is idiomatic C++.

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

If a value can be known at compile time, let the compiler compute it. Zero runtime cost, compile-time verification.

### 6. `std::optional` Encodes Absence in the Type

```cpp
std::optional<Board> result = solve(board);
```

Use `std::optional` when a value might not exist. The type system forces callers to handle both cases.

> **C# Bridge:** The managed world gives you GC, nullable references, and HashSet out of the box. The C++ equivalents — stack allocation, `std::optional`, bitmasks — are more explicit but give you direct control over memory layout, allocation, and lifetime. That control is the point.

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
