# C++ Concept Coverage Plan

This document tracks which C++ concepts should be covered by Cpptudes.

## Dual Structure

This document serves both project requirements:

- **Requirement A (C++ teaching):** The "Concept," "C++ Axiom," and "Rating" columns define what C++ knowledge each cpptude must teach. The rating scale below rates **C++ concept importance** -- how critical the concept is for writing correct, professional C++ code, regardless of the learner's background.
- **Requirement B (C# bridge):** The "C# Trap" column identifies where managed-runtime intuition conflicts with C++ reality, guiding how each cpptude baits and corrects C# habits. The "C# Trap Severity" column quantifies how likely a C# developer is to be trapped by each concept.

A concept rated "5 - Critical" is critical **for C++** (e.g., a developer who doesn't understand object lifetime will write dangerous C++ code). The C# Trap column then explains *why* a C# developer is especially likely to get it wrong. The C# Trap Severity column rates *how likely* the trap is to fire for a C# developer, on a 1-5 scale.

---

## Rating Scales

### C++ Concept Importance

| Rating | Level | Description |
|--------|-------|-------------|
| **5** | Critical | The developer is **dangerous** without this. Memory safety, lifetime, undefined behavior. |
| **4** | Required | Essential for modern C++. Cannot write professional code without it. |
| **3** | Important | Differentiates competent from proficient. Unlocks performance and expressiveness. |
| **2** | Good-to-Know | Useful in specific domains. Not every project needs it. |
| **1** | Seasoning | Niche features, legacy patterns, or stylistic choices. |

### C# Trap Severity

| Severity | Description |
|----------|-------------|
| **5** | Virtually guaranteed to trap a C# developer. The C# instinct directly produces incorrect C++ code. |
| **4** | Very likely to trap. The C# mental model is wrong in a way the developer will not notice without guidance. |
| **3** | Moderate. C# habits cause confusion but the developer may self-correct with effort. |
| **2** | Low. C# habits are mildly misleading but the concept is learnable without strong habit conflict. |
| **1** | Unlikely to trap. C# habits are neutral or the concept has no C# equivalent to conflict with. |

Combined priority for cpptude development order within each tier:
- **High C++ importance + High C# trap severity** = dedicated cpptude, developed first.
- **High C++ importance + Low C# trap severity** = important, can be integrated.
- **Low C++ importance + High C# trap severity** = needs explicit attention despite lower tier.
- **Low C++ importance + Low C# trap severity** = can be deferred.

---

## Tier 0: Tooling

ASan and UBSan are **infrastructure**, not topics. Every cpptude depends on them to make failures visible. Without sanitizers, many C++ errors are silent -- which is precisely the problem the curriculum is designed to solve.

| Component | Purpose | When Introduced |
|-----------|---------|-----------------|
| ASan (`-fsanitize=address`) | Use-after-free, heap buffer overflow, stack buffer overflow, memory leaks | Setup Guide (#0) |
| UBSan (`-fsanitize=undefined`) | Signed overflow, null dereference, shift UB, alignment UB | Setup Guide (#0) |
| `-Wall -Wextra -Werror` | Uninitialized variables, narrowing conversions, unused results | Setup Guide (#0) |
| Benchmarks (`std::chrono`) | Performance cliffs from cache misses, excessive allocation | Game of Life (#11) |
| Compiler Explorer (Godbolt) | Generated assembly showing real machine cost | Image Pipeline (#4), optional throughout |

**Cpptude #0: Setup Guide.** A lightweight setup exercise (not a full cpptude with a problem domain) that walks the learner through:
1. Building a trivial program with ASan/UBSan enabled.
2. Deliberately triggering a use-after-free and seeing the ASan report.
3. Deliberately triggering signed integer overflow and seeing the UBSan report.
4. Understanding what the sanitizer output means.

The goal is to get the learner's environment configured so that subsequent cpptudes can rely on the tooling being present. See `best-practices.md` Principle 8 for the full rationale.

---

## Tier 1: Critical (Rating 5)

These are **foundational C++ concepts** where misunderstanding leads to unsafe, incorrect, or undefined behavior. The C# Trap column shows why C# developers are especially prone to getting these wrong.

### Memory & Lifetime

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Stack vs Heap allocation** | Values live where declared; scope = lifetime | All objects live on heap, GC-managed | **5** | Sudoku |
| **RAII (Resource Acquisition Is Initialization)** | Destructor runs unconditionally at scope exit | `using`/`IDisposable` is optional pattern | **4** | File Processor |
| **Parameter passing modes** | `T`, `const T&`, `T&`, `T&&` -- four modes for any type | Two modes: value types by value, reference types by reference-handle | **5** | Parameter Passing |
| **Object lifetime & scope** | Objects die at `}` -- references become invalid | Objects live until GC collects | **5** | Iterator Invalidation |
| **Dangling references** | Returning `&local` is UB; references don't extend lifetime | References can't dangle (GC) | **5** | String Parser |

### Value Semantics

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Copy semantics** | `=` creates independent copy for value types | `=` creates alias for reference types | **5** | Data Pipeline |
| **Struct vs Class defaults** | Both are same; only default visibility differs | `struct` = value type, `class` = reference type | **3** | (integrated) |
| **Const correctness** | `const` is pervasive; immutability is the default assumption | `readonly` is limited; mutation is normal | **4** | (integrated -- see Const Integration Plan below) |

### Undefined Behavior

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate | UB Surfaced By |
|-------------|---------------------|---------------------|:---:|-------------------|----------------|
| **Signed integer overflow** | UB -- compiler assumes it never happens | Wraps predictably | **2** | Overflow Calculator | UBSan |
| **Null dereference** | UB -- anything can happen | `NullReferenceException` (recoverable) | **3** | (integrated in ownership cpptudes) | ASan |
| **Uninitialized variables** | Reading uninitialized memory is UB | Compiler enforces definite assignment | **2** | (caught by `-Werror` from cpptude #0) | `-Wall -Werror` |

---

## Tier 2: Required (Rating 4)

These concepts are **essential for writing modern, professional C++** but don't immediately cause UB if misunderstood.

### Modern Ownership

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Move semantics** | Move transfers resources; leaves source valid-but-empty | No equivalent; `=` creates alias, never invalidates source | **5** | Image Pipeline |
| **`std::unique_ptr`** | Single owner; transfer via `std::move` | `new` + hope GC cleans up | **4** | Tree Builder |
| **Raw pointers as non-owning observers** | `T*` = non-owning, may be null; `T&` = non-owning, never null | Fear of all pointers or treating them like C# references | **4** | Tree Builder (secondary KAP) |
| **`std::shared_ptr`** | Shared ownership has cost; prefer unique | All references are "shared" by default | **3** | Graph Algorithms |

### Type Design

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Rule of Zero** | Use RAII members; compiler generates correct special members | Never think about copy/move/destroy -- runtime handles it | **3** | Rule of Zero |
| **Object slicing** | Assigning Derived to Base by value strips derived part | All base-to-derived assignment creates reference, never copies | **5** | Data Pipeline (secondary KAP) |

### Error Handling

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **`std::optional`** | Optional makes "might not exist" explicit in type | `null` is always an option | **2** | Config Parser |
| **`std::expected` (C++23)** | Expected carries error *or* value; no unwinding | Exceptions for control flow | **2** | Validator |
| **Exception safety guarantees** | Must design for basic/strong/nothrow guarantees | Exceptions are recoverable always; GC is safety net | **4** | Transaction |

### String Handling

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **`std::string_view`** | Views don't own; underlying string must outlive view | Strings are immutable, interned, cheap to slice | **4** | Spell Corrector |
| **`std::string` allocation** | `std::string` may heap-allocate; SSO is not guaranteed | Strings are magic | **3** | Text Processor |

### Containers

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **`std::vector` growth and iterator invalidation** | Vector reallocates on growth; all iterators/refs/pointers invalidate | `List<T>` grows silently; modification during enumeration throws `InvalidOperationException` | **4** | (covered by Tier 1 Iterator Invalidation cpptude; vector growth is the primary case) |
| **`std::array` vs C-array** | `std::array` is fixed-size, stack-allocated, bounds-checked in debug | Arrays are objects with bounds checking | **2** | Sudoku |

### Functions & Lambdas

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Lambda captures** | Capture by value or ref is explicit; ref captures can dangle | Closures capture by reference, GC keeps alive | **5** | Event System |
| **`std::function` overhead** | `std::function` type-erases; has overhead vs template | Delegates are cheap | **2** | Callback Patterns |

---

## Tier 3: Important (Rating 3)

These C++ concepts **unlock performance and expressiveness** but require foundation from Tiers 1-2.

### Templates & Generic Programming

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Function templates** | Templates instantiate per-type; no runtime cost | Generics with type erasure | **2** | Generic Sort |
| **Concepts (C++20)** | Concepts constrain templates with clear errors | `where T : IComparable` | **2** | Constrained Algorithms |
| **CRTP** | Static polymorphism without vtable | No equivalent | **1** | Mixin Pattern |

### Performance & Cache

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **Cache locality** | Flat, contiguous data is orders of magnitude faster | Memory layout is runtime's problem | **4** | Game of Life |
| **`std::vector<T>` vs `std::vector<unique_ptr<T>>`** | Indirection destroys cache performance | Reference types are always indirect | **4** | Entity System |
| **`std::span`** | `std::span` is just pointer+size; no magic | `Span<T>` is special | **2** | Array Utilities |

### Ranges & Algorithms (C++20)

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **`std::ranges` pipelines** | Ranges compose views without allocation | LINQ is lazy and magical | **3** | Data Pipeline |
| **Views vs owning ranges** | Views borrow; lifetime of source matters | LINQ returns `IEnumerable` | **3** | Advent of Code |
| **STL algorithms** | Algorithms are separate from containers | LINQ methods on collections | **2** | Algorithm Golf |

### Compile-Time Programming

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | C# Trap Severity | Cpptude Candidate |
|-------------|---------------------|---------------------|:---:|-------------------|
| **`constexpr` functions** | `constexpr` moves computation to compile time | No equivalent (limited `const`) | **1** | Lookup Tables |
| **`consteval` (C++20)** | Guaranteed compile-time evaluation | No equivalent | **1** | Compile-Time Sudoku |
| **`static_assert`** | Compile-time assertions catch errors early | No equivalent | **1** | (integrated) |

---

## Tier 4: Good-to-Know (Rating 2)

| Concept | Notes | C# Trap Severity | Cpptude Candidate |
|---------|-------|:---:|-------------------|
| **Variadic templates** | Parameter packs, fold expressions | **1** | Printf Implementation |
| **SFINAE** | Pre-C++20 constraint technique | **1** | (historical sidebar) |
| **Type traits** | `std::is_same`, `std::decay`, etc. | **1** | Type Inspector |
| **Allocators** | Custom memory allocation strategies | **1** | Pool Allocator |
| **Coroutines (C++20)** | Generators, async patterns | **3** | Async Pipeline |

---

## Tier 5: Seasoning (Rating 1)

| Concept | Notes | C# Trap Severity |
|---------|-------|:---:|
| **Placement new** | Constructing in pre-allocated memory | **1** |
| **`volatile`** | Hardware/signal interaction (not threading!) | **3** |
| **Attributes** | `[[nodiscard]]`, `[[likely]]`, etc. | **1** |
| **Specialized casts** | `reinterpret_cast`, `const_cast` | **1** |
| **Bit manipulation** | `std::bit_cast`, `<bit>` header | **1** |

Note on `volatile`: C# `volatile` provides acquire/release semantics. C++ `volatile` does not. This is a moderate C# trap severity (3) despite the concept's low C++ importance, because a C# developer who encounters `volatile` in C++ will assume it has threading semantics.

---

## Coverage Matrix

Track which concepts are covered by which cpptudes. The Bait, Failure Signal, and Verification Tool columns operationalize the project's core pedagogical strategy (see `best-practices.md` Principles 1, 3, and 9).

| # | Cpptude | Primary KAP (Rating) | Secondary Concepts | Prerequisites | Bait | Failure Signal | Verification Tool |
|:-:|---------|---------------------|--------------------|---------------|------|---------------|-------------------|
| 0 | Setup Guide | Tooling (--) | ASan, UBSan, compiler warnings | None | -- | -- | -- |
| 1 | Sudoku | Stack allocation (5) | `std::array`, `constexpr`, bitwise ops, const introduction | Setup Guide | Allocate board with `new` on the heap | Heap allocation count > 0 | Custom allocator or test asserting zero heap allocations |
| 2 | File Processor | RAII (5) | Exception safety, destructor guarantees, multi-file structure | Sudoku | Manual open/close with `try`/`finally` pattern; forget to close on error path | Resource leak when exception propagates | ASan (leak detection); test that triggers exception and verifies cleanup |
| 3 | Parameter Passing | Parameter passing modes (5) | `const` reference parameter, value vs reference, `T&&` preview | File Processor | Pass large struct by value everywhere (C# reference-type habit) | Benchmark shows copy overhead; or pass-by-value mutation doesn't affect caller | Benchmark threshold; test verifying caller state after call |
| 4 | Image Pipeline | Move semantics (4) | `noexcept`, moved-from state, rvalue references | Parameter Passing | Assign/copy a resource-owning object expecting both to remain valid (C# alias semantics) | Use moved-from container (empty vector, nullptr unique_ptr) | ASan (use-after-move); test asserting source is empty after move |
| 5 | String Parser | Dangling references (5) | Use-after-free, reference lifetime | Image Pipeline | Return `string_view` or reference to local variable (C# style: return anything, GC keeps it alive) | Use-after-free on access | ASan (stack-use-after-return with `ASAN_OPTIONS=detect_stack_use_after_return=1`) |
| 6 | Tree Builder | `std::unique_ptr` (4) | Raw pointers as non-owning observers, recursive ownership | String Parser | Use `new`/`delete` manually or `shared_ptr` everywhere | Memory leak (manual) or unnecessary overhead (shared_ptr ref counting) | ASan (leak detection); benchmark showing shared_ptr overhead |
| 7 | Spell Corrector | `std::string_view` (4) | `std::unordered_map`, custom hash, reference lifetime | Tree Builder | Slice strings freely, store string_view beyond source lifetime | Dangling view access | ASan (heap-use-after-free) |
| 8 | Data Pipeline | Copy semantics (5) | `std::ranges`, object slicing (secondary KAP), lazy evaluation | Spell Corrector | Assign derived to base by value; expect polymorphic behavior (C# alias semantics) | Sliced object loses derived data; copies where aliases were expected | Test asserting derived data is preserved (fails with slicing); benchmark showing copy cost |
| 9 | Event System | Lambda captures (4) | Dangling captures, callback lifetime | Data Pipeline | Capture local by reference in stored callback (C# closure keeps captured variable alive) | Dangling reference in callback; corrupted data or crash on invocation | ASan (stack-use-after-return) |
| 10 | Rule of Zero | Rule of Zero (4) | Rule of Five (advanced challenge), special member functions | Event System | Write destructor + copy constructor but forget move operations (incomplete Rule of Five) | Performance regression (vector copies instead of moves due to throwing move); or resource leak | Benchmark showing copy-vs-move performance; ASan (leak in incomplete Rule of Five) |
| 11 | Game of Life | Cache locality (3) | Flat memory, `std::vector<bool>` pitfalls | Rule of Zero | Use pointer-chasing structure (linked list of cells, like C# object graph) | 10-100x performance difference vs flat array | Benchmark with clear threshold (e.g., must complete in < 50ms) |
| 12 | Validator | `std::expected` (4) | Error handling without exceptions | Game of Life | Use exceptions for control flow (C# habit) | Exception unwinding cost in hot path | Benchmark comparing exception vs `std::expected` performance |
| 13 | Transaction | Exception safety guarantees (4) | Basic/strong/nothrow guarantees, `std::expected` as modern alternative | Validator | Acquire resource without RAII, assume exception is always recoverable | Resource leak on exception; partially modified state | ASan (leak detection); test triggering exception mid-operation and verifying invariants |
| 14 | Overflow Calculator | Signed integer overflow (5) | UB mechanics, compiler optimization assumptions | Transaction | Assume integer overflow wraps (C# `unchecked` default behavior) | UBSan fires; or optimized build produces impossible results | UBSan; comparison of debug vs release output |
| 15 | Advent of Code | Integration (--) | All prior concepts | All above | -- | -- | All tools |

### Iterator Invalidation Cpptude

The "Iterator Invalidation" cpptude (listed in Tier 1, Object lifetime & scope) covers `std::vector` growth invalidation as its primary case. This is the single cpptude for iterator invalidation -- there is no separate "Iterator Lab" or "Collection Builder" cpptude. The `std::vector` growth entry in Tier 2 Containers references this cpptude. The cpptude should be sequenced after RAII and dangling references because iterator invalidation is fundamentally a lifetime problem (a handle becomes invalid when the thing it refers to is destroyed or moved).

---

## Const Correctness Integration Plan

Const correctness is a *cross-cutting concern* taught through distributed practice rather than a dedicated cpptude. The integration plan ensures const is taught deliberately rather than incidentally.

| Cpptude | Const Usage Introduced | Lesson Text Requirement |
|---------|----------------------|------------------------|
| #1 Sudoku | `const` local variables | Explicit sidebar: "In C++, `const` is a compiler-enforced contract, not decoration. Mark variables `const` unless they must be mutated." |
| #2 File Processor | `const` member functions | Explain why accessor methods are marked `const`: "This tells the compiler and the reader that calling this method will not change the object." |
| #3 Parameter Passing | `const T&` parameters | Core lesson content: "Pass large types by `const T&` to avoid copying while preventing mutation. This is the most common parameter mode in C++." |
| All subsequent | Rigorous `const` usage | Every cpptude uses `const` correctly. The Quality Checklist in `best-practices.md` verifies this. |

---

## UB Concept Mapping

Each undefined behavior concept is surfaced by a specific cpptude rather than a combined "UB Showcase." This ensures UB encounters arise naturally from the learner's C# instincts in context.

| UB Concept | Surfaced By | How |
|-----------|------------|-----|
| Signed integer overflow | Overflow Calculator (#14) | Learner assumes overflow wraps (C# `unchecked`); UBSan fires; optimized build produces impossible results |
| Null dereference | Tree Builder (#6), ownership cpptudes | Learner dereferences raw pointer without null check; ASan/segfault surfaces the error |
| Uninitialized variables | Every cpptude from #0 | `-Wall -Werror` catches at compile time; ASan catches at runtime if warning is missed |
| Use-after-free | String Parser (#5), Event System (#9) | Learner returns reference to local or captures local by reference; ASan catches access |
| Iterator invalidation | Iterator Invalidation (Tier 1) | Learner modifies container during iteration or after reallocation; ASan catches dangling iterator |

---

## Build Model Sidebar Requirement

The first cpptude that uses multiple translation units (likely File Processor, #2) must include a sidebar explaining the C++ compilation model in contrast to C#'s project model:

> "In C#, the compiler sees your entire project. In C++, each `.cpp` file is compiled independently -- it knows only what has been `#include`d. The linker connects the compiled pieces afterward. This is why you get 'undefined reference' errors that have no C# equivalent."

The sidebar should include a brief mental model diagram and reference C++20 modules as the future direction. Subsequent cpptudes may reference this sidebar rather than repeating it.

---

## Sequencing Recommendation

The revised sequence respects the dependency chain identified through criticism review: stack allocation, RAII, parameter passing, move semantics, dangling references, ownership (unique_ptr), string_view, copy semantics, lambda captures, Rule of Zero, performance, error handling.

```
 #0  Setup Guide         -- ASan/UBSan setup, first sanitizer demo
 #1  Sudoku              -- Stack allocation, value semantics foundation, const introduction
 #2  File Processor      -- RAII, destructor guarantees, multi-file structure
 #3  Parameter Passing   -- T, const T&, T&, T&& (introduce rvalue references)
 #4  Image Pipeline      -- Move semantics, noexcept, moved-from state
 #5  String Parser       -- Dangling references, use-after-free via ASan
 #6  Tree Builder        -- unique_ptr, raw pointers as non-owning observers
 #7  Spell Corrector     -- string_view, reference lifetime
 #8  Data Pipeline       -- Copy semantics, ranges, object slicing (secondary KAP)
 #9  Event System        -- Lambda captures, dangling captures
#10  Rule of Zero        -- Special member function design, Rule of Five (advanced)
#11  Game of Life        -- Cache locality, flat memory
#12  Validator           -- std::expected, modern error handling
#13  Transaction         -- Exception safety guarantees
#14  Overflow Calculator -- Signed integer overflow, UB aha moment
#15  Advent of Code      -- Integration of all concepts
```

**Critical sequencing constraints:**
- RAII (#2) before dangling references (#5) -- learner must understand deterministic destruction
- Parameter passing (#3) before move semantics (#4) -- learner must understand `T`, `const T&`, `T&` before `T&&`
- Move semantics (#4) before `unique_ptr` (#6) -- transferring ownership requires `std::move`
- Dangling references (#5) before lambda captures (#9) -- captures are a specialization of dangling references
- Copy and move semantics (#4, #8) before Rule of Zero (#10) -- learner must understand what the compiler generates
- RAII (#2) and Validator (#12) before Transaction (#13) -- exception safety builds on both RAII and `std::expected`

---

## Deferred Topics

The following topics are **explicitly out of scope** for the initial curriculum. They are not gaps -- they are design decisions with documented rationale.

### Concurrency (threads, atomics, memory ordering)

**Rationale for deferral:** Concurrency in C++ requires solid understanding of ownership, lifetime, and value semantics. Data races in C++ are undefined behavior (not merely "wrong results" as in C#). Teaching concurrency before the foundations are solid would be irresponsible.

**Prerequisites for a future concurrency module:** All Tier 1 and Tier 2 concepts.

**Warning for C# developers:** C# developers are comfortable with concurrency (`async`/`await`, `Task`, `lock`, `ConcurrentDictionary`). Their C# concurrency instincts are **actively dangerous** in C++:
- C# `volatile` provides acquire/release semantics. C++ `volatile` does not.
- C# `lock` maps to `Monitor.Enter`/`Exit` with full memory barriers. C++ `std::mutex` has similar semantics but no automatic scope management without `std::lock_guard`.
- C# data races produce wrong results. C++ data races are UB -- the program's behavior is completely undefined.

**Do not attempt multithreaded C++ code until a concurrency module exists.** The C# instinct that "I know concurrency" will produce code that compiles, appears to work in testing, and fails catastrophically under load.

### Deep polymorphism and OOP patterns

**Rationale for deferral:** Modern C++ favors value semantics, RAII, and composition over inheritance. Object slicing is covered as a trap to avoid (Data Pipeline, secondary KAP), but deep inheritance hierarchies and virtual dispatch patterns are not part of the initial curriculum. C# developers will be tempted to reach for inheritance as their primary abstraction tool -- the curriculum should redirect them toward composition and value semantics.

### C++20 modules

**Rationale for deferral:** Module support across compilers is still maturing. The curriculum uses the traditional header/source model with a compilation model sidebar. Modules should be adopted when GCC, Clang, and MSVC all provide reliable module support.
