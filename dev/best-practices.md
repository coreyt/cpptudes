# Cpptude Best Practices

## Project & User Requirements

Every design decision in this document flows from three requirements:

**A. Teach core C++ concepts through exercises.**
Cpptudes are first and foremost C++ etude exercises. The concepts selected, the code written, and the patterns demonstrated must be correct and idiomatic C++ on their own merits.

**B. Bridge specifically from C# to C++:**
- **B1.** Specific cpptudes address the C# to C++ transition.
- **B2.** Each cpptude explicitly highlights differences between C# and C++.

**User Requirement:** The user's goal is to transition from a C# developer to a C++ Software Engineer by completing cpptude exercises.

The design principles below serve these requirements as follows:

| Principle | Serves |
|-----------|--------|
| 1. Make Wrong Approaches Fail Visibly | A — good C++ teaching |
| 2. One Primary KAP | A — good C++ teaching |
| 3. The "C# Trap" Must Be Baited | B — C# bridge |
| 4. Mechanical Sympathy Over Abstraction | A — good C++ teaching |
| 5. Modern C++ First | A — good C++ teaching |
| 6. No Heap Unless Justified | A — good C++ teaching |
| 7. Compilation Must Be Reproducible | A — good C++ teaching |

---

## The Core Insight

> **Norvig's pytudes work because Python's idioms are non-obvious to someone who knows another language. The same is true for C++, but the stakes are higher:**
>
> - **Python:** Wrong idiom → slower or less elegant code
> - **C++:** Wrong idiom → **undefined behavior, memory leaks, security vulnerabilities**

This fundamental difference shapes every design decision in a Cpptude.

---

## Design Principles

### 1. Make Wrong Approaches Fail Visibly

Cpptudes must be designed with **guardrails that make the wrong approach fail loudly** rather than silently producing wrong results.

| Failure Mode | Technique |
|--------------|-----------|
| Compile-time | `static_assert`, `= delete`, concepts with clear error messages |
| Runtime (debug) | Address/UB sanitizers, assertions, debug iterators |
| Performance | Benchmarks with clear thresholds that expose naive approaches |

**Anti-pattern:** A Cpptude where the "C# way" compiles, runs, and produces correct output but is subtly wrong (leaks, UB, poor performance). The learner won't know they failed.

### 2. One Primary KAP (Knowledge Acquisition Point)

Each Cpptude teaches **one critical concept** deeply. Secondary concepts may appear but should not distract from the primary lesson.

**Good:** "This lesson teaches stack allocation. You'll also see `constexpr`, but that's not the focus."

**Bad:** "This lesson teaches stack allocation, move semantics, RAII, templates, and ranges."

### 3. The "C# Trap" Must Be Baited

The lesson should be structured so the learner's C# instincts **naturally lead them toward the wrong approach first**. Then, the guardrails activate.

Example structure:
1. Present problem (familiar from C#/Python context)
2. Learner attempts C#-style solution
3. Compilation fails / sanitizer fires / benchmark shows 100x slowdown
4. Lesson explains the C++ Axiom
5. Learner implements correct approach
6. Guardrails now pass

### 4. Mechanical Sympathy Over Abstraction

Cpptudes should expose **what the machine is actually doing**:
- Where does this live in memory? (stack, heap, static)
- When does this get copied vs moved?
- What does the compiler generate?
- What does the CPU cache see?

Avoid: "Just use `std::unique_ptr` and trust it."
Prefer: "Here's why `std::unique_ptr` generates the same code as raw pointer with manual delete, but without the bug surface."

### 5. Modern C++ First (C++20/23)

Target the latest standards. Legacy patterns should only appear in "historical context" sidebars, not as taught techniques.

- Use `std::span` not pointer+size
- Use `std::ranges` not raw iterator pairs
- Use concepts not SFINAE
- Use `std::expected` not exceptions-for-control-flow
- Use `std::format` not iostream/printf

### 6. No Heap Unless Justified

Many Cpptudes should include the constraint: **"Zero heap allocations."**

This forces learners to:
- Use `std::array` instead of `std::vector`
- Think about lifetime and scope
- Understand stack size limits
- Appreciate when the heap *is* necessary

### 7. Compilation Must Be Reproducible

Every Cpptude must specify:
- Exact compiler flags (`-std=c++20 -O2 -Wall -Wextra -fsanitize=address,undefined`)
- Minimum compiler versions (GCC 12+, Clang 15+, MSVC 19.30+)
- Any required dependencies

---

## Quality Checklist

Before a Cpptude is complete, verify:

**Requirement A — C++ teaching quality:**
- [ ] **Axiom is stated:** The correcting C++ principle is explicitly written
- [ ] **Code compiles:** With specified flags on all target compilers
- [ ] **Sanitizers pass:** ASan/UBSan report no issues on correct solution
- [ ] **Benchmark exists:** (if performance-related) Clear threshold separating naive from correct
- [ ] **Next Step challenge exists:** Extension that reinforces the primary KAP

**Requirement B — C# bridge:**
- [ ] **Trap works:** A C#-style implementation hits a visible failure
- [ ] **"C# Bridge" sections exist:** Explicit contrast with managed-world expectations for each code section

---

## Commentary Style Guide

### Code Comments: "Why" Not "What"

```cpp
// BAD: Increment counter
++counter;

// GOOD: Pre-increment avoids temporary copy (matters for iterators)
++counter;
```

### Contrast Blocks

Use explicit contrast with C# throughout:

```cpp
// C# INSTINCT: In C#, this would be a class instance on the heap.
// C++ REALITY: This is a value on the stack. When this scope exits, it's gone.
// No GC. No pointers. The bytes themselves live here.
Sudoku board{};
```

### Memory Diagrams

Include ASCII diagrams showing stack/heap layout when relevant:

```
Stack frame (solve_sudoku):
┌─────────────────────────┐
│ board: Sudoku [81 bytes]│  ← Lives HERE, on the stack
│ row_mask: uint16_t[9]   │
│ col_mask: uint16_t[9]   │
│ box_mask: uint16_t[9]   │
└─────────────────────────┘
Total stack usage: ~135 bytes
Heap allocations: 0
```
