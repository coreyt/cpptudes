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
| 1. Make Wrong Approaches Fail Visibly | A -- good C++ teaching |
| 2. One Primary KAP | A -- good C++ teaching |
| 3. The "C# Trap" Must Be Baited | B -- C# bridge |
| 4. Mechanical Sympathy Over Abstraction | A -- good C++ teaching |
| 5. Modern C++ First | A -- good C++ teaching |
| 6. No Heap Unless Justified | A -- good C++ teaching |
| 7. Compilation Must Be Reproducible | A -- good C++ teaching |
| 8. Sanitizers Are Infrastructure | A -- good C++ teaching |
| 9. Every Cpptude Specifies Bait/Failure/Tool | A + B -- both |

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

### 8. Sanitizers Are Infrastructure, Not Optional

ASan and UBSan are not debugging tools to reach for when something goes wrong. They are **always-on infrastructure** that every cpptude depends on to make failures visible.

- Every cpptude builds with `-fsanitize=address,undefined` in its debug configuration. This is non-negotiable.
- Specific cpptudes designate specific tools as their primary "aha" mechanism (see the Verification Tool column in `concept-coverage.md`).
- Sanitizers replace the runtime safety net that C# developers unconsciously rely on (exceptions for null dereference, bounds checking for arrays). Without sanitizers, the learner loses that safety net and does not know it.

| Tool | What It Surfaces | Example Cpptude Use |
|------|-----------------|---------------------|
| ASan | Use-after-free, heap buffer overflow, stack buffer overflow, memory leaks | Dangling references (String Parser), lambda captures (Event System) |
| UBSan | Signed integer overflow, null dereference, shift UB, alignment UB | Overflow Calculator, pointer exercises |
| `-Wall -Wextra -Werror` | Uninitialized variables, narrowing conversions, unused results | Every cpptude from #0 onward |
| Benchmarks (`std::chrono`) | Performance cliffs from cache misses, excessive allocation, copy overhead | Cache locality (Game of Life), copy vs move |
| Compiler Explorer (Godbolt) | Generated assembly showing real machine cost | Move vs copy, `constexpr` evaluation |

### 9. Every Cpptude Must Specify Bait, Failure Signal, and Verification Tool

Before implementation begins, every cpptude's design document must answer three questions:

1. **Bait:** What will the C# developer naturally try? This is the managed-runtime instinct that the cpptude is designed to surface. (Example: "Allocate the board on the heap with `new`.")
2. **Failure Signal:** What makes that approach fail visibly? The failure must be observable, not theoretical. (Example: "A custom allocator counts heap allocations; a `static_assert` or test asserts zero allocations.")
3. **Verification Tool:** Which tool surfaces the failure? (Example: "ASan catches use-after-free," "UBSan catches signed overflow," "benchmark threshold fails at 100ms.")

Without all three, a cpptude risks becoming "C++ syntax for C# developers" rather than "unlearning managed-runtime instincts." The bait/failure/tool triad operationalizes Principles 1 and 3.

See the Coverage Matrix in `concept-coverage.md` for the per-cpptude bait, failure signal, and verification tool assignments.

---

## Coding Standards

These standards apply to all cpptude code -- both the correct solution and any example code in the lesson text.

### `explicit` on Single-Argument Constructors

All single-argument constructors must be marked `explicit` unless implicit conversion is intentionally desired and documented. C++ implicitly converts via single-argument constructors; C# is generally stricter about type conversions. The `explicit` keyword prevents surprising implicit conversions.

```cpp
// GOOD: explicit prevents accidental conversion from int to Widget
class Widget {
    explicit Widget(int size);
};

// BAD: allows int to implicitly convert to Widget
class Widget {
    Widget(int size);  // unmarked — implicit conversion is allowed
};
```

Mention `explicit` in the lesson text the first time a cpptude defines a class with a single-argument constructor.

### Initialization Style

Use **brace initialization** (`{}`) as the default initialization syntax. Brace initialization prevents narrowing conversions and avoids the most vexing parse.

```cpp
int x{42};                    // preferred
std::string name{"hello"};    // preferred
std::vector<int> v{1, 2, 3};  // preferred

int y = 3.14;                 // compiles — silent narrowing
int z{3.14};                  // error — narrowing caught
```

Exceptions where parentheses are preferred:
- When brace initialization invokes `std::initializer_list` when you intend the size constructor: `std::vector<int> v(10)` (10 elements) vs `std::vector<int> v{10}` (one element: 10).
- When using `make_unique` or `make_shared` which forward arguments.

The first cpptude must include a sidebar explaining why C++ has multiple initialization syntaxes and which one this curriculum uses.

### Const Usage

Use `const` pervasively. Const correctness is not decoration -- it is a compiler-enforced contract that communicates intent about mutation.

- Mark local variables `const` unless they must be mutated.
- Pass large types by `const T&` (introduced in cpptude #3: Parameter Passing).
- Mark member functions `const` when they do not modify the object.
- The first cpptude introduces `const` with explicit lesson text. Every subsequent cpptude uses const rigorously.

---

## Quality Checklist

Before a Cpptude is complete, verify:

**Requirement A -- C++ teaching quality:**
- [ ] **Axiom is stated:** The correcting C++ principle is explicitly written
- [ ] **Code compiles:** With specified flags on all target compilers
- [ ] **Sanitizers pass:** ASan/UBSan report no issues on correct solution
- [ ] **Benchmark exists:** (if performance-related) Clear threshold separating naive from correct
- [ ] **Next Step challenge exists:** Extension that reinforces the primary KAP
- [ ] **Const is used rigorously:** Local variables, parameters, and member functions use `const` where applicable
- [ ] **`explicit` is used:** All single-argument constructors are marked `explicit` unless implicit conversion is intentional
- [ ] **Initialization uses braces:** Default initialization syntax is `{}` unless an exception applies

**Requirement B -- C# bridge:**
- [ ] **Trap works:** A C#-style implementation hits a visible failure
- [ ] **"C# Bridge" sections exist:** Explicit contrast with managed-world expectations for each code section
- [ ] **Bait is specified:** The design document names the C# instinct the cpptude targets
- [ ] **Failure signal is specified:** The design document names the observable failure
- [ ] **Verification tool is specified:** The design document names the tool that surfaces the failure

**Structural checks:**
- [ ] **Build model sidebar:** If this cpptude introduces multi-file structure for the first time, the lesson text explains the C++ compilation model in contrast to C#'s project model ("Each `.cpp` file is compiled alone. `#include` copy-pastes the header content. The linker connects the pieces.")
- [ ] **Const introduction:** If this is cpptude #1, const correctness is introduced with explicit lesson text. If this is cpptude #2 or #3, at least one new const usage is explained in context (const local, const reference parameter, const member function).

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
