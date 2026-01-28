# C++ Concept Coverage Plan

This document tracks which C++ concepts should be covered by Cpptudes, prioritized by criticality for C# developers transitioning to native code.

---

## Rating Scale

| Rating | Level | Description |
|--------|-------|-------------|
| **5** | Critical | The developer is **dangerous** without this. Memory safety, lifetime, undefined behavior. |
| **4** | Required | Essential for modern C++. Cannot write professional code without it. |
| **3** | Important | Differentiates competent from proficient. Unlocks performance and expressiveness. |
| **2** | Good-to-Know | Useful in specific domains. Not every project needs it. |
| **1** | Seasoning | Niche features, legacy patterns, or stylistic choices. |

---

## Tier 1: Critical (Rating 5)

These concepts address areas where **C# intuition causes C++ code to be unsafe, incorrect, or exhibit undefined behavior.**

### Memory & Lifetime

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **Stack vs Heap allocation** | All objects live on heap, GC-managed | Values live where declared; scope = lifetime | Sudoku |
| **RAII (Resource Acquisition Is Initialization)** | `using`/`IDisposable` is optional pattern | Destructor runs unconditionally at scope exit | File Processor |
| **Object lifetime & scope** | Objects live until GC collects | Objects die at `}` — references become invalid | Iterator Invalidation |
| **Dangling references** | References can't dangle (GC) | Returning `&local` is UB; references don't extend lifetime | String Parser |

### Value Semantics

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **Copy semantics** | `=` creates alias for reference types | `=` creates independent copy for value types | Data Pipeline |
| **Struct vs Class defaults** | `struct` = value type, `class` = reference type | Both are same; only default visibility differs | (integrated) |
| **Const correctness** | `readonly` is limited; mutation is normal | `const` is pervasive; immutability is the default assumption | (integrated) |

### Undefined Behavior

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **Signed integer overflow** | Wraps predictably | UB — compiler assumes it never happens | Overflow Calculator |
| **Null dereference** | `NullReferenceException` (recoverable) | UB — anything can happen | (integrated) |
| **Uninitialized variables** | Compiler enforces definite assignment | Reading uninitialized memory is UB | (sanitizer exercises) |

---

## Tier 2: Required (Rating 4)

These concepts are **essential for writing modern, professional C++** but don't immediately cause UB if misunderstood.

### Modern Ownership

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **`std::unique_ptr`** | `new` + hope GC cleans up | Single owner; transfer via `std::move` | Tree Builder |
| **`std::shared_ptr`** | All references are "shared" by default | Shared ownership has cost; prefer unique | Graph Algorithms |
| **Move semantics** | No equivalent; copies are cheap (refs) | Move transfers resources; leaves source valid-but-empty | Image Pipeline |

### Error Handling

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **`std::optional`** | `null` is always an option | Optional makes "might not exist" explicit in type | Config Parser |
| **`std::expected` (C++23)** | Exceptions for control flow | Expected carries error *or* value; no unwinding | Validator |
| **Exception safety guarantees** | Exceptions are recoverable always | Must design for basic/strong/nothrow guarantees | Transaction |

### String Handling

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **`std::string_view`** | Strings are immutable, interned, cheap to slice | Views don't own; underlying string must outlive view | Spell Corrector |
| **`std::string` allocation** | Strings are magic | `std::string` may heap-allocate; SSO is not guaranteed | Text Processor |

### Containers

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **`std::vector` growth** | `List<T>` is magic | Vector reallocates; iterators/refs invalidate on growth | Collection Builder |
| **Iterator invalidation** | Enumeration can't mutate anyway | Each container has specific invalidation rules | Iterator Lab |
| **`std::array` vs C-array** | Arrays are objects with bounds checking | `std::array` is fixed-size, stack-allocated, bounds-checked in debug | Sudoku |

### Functions & Lambdas

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **Lambda captures** | Closures capture by reference, GC keeps alive | Capture by value or ref is explicit; ref captures can dangle | Event System |
| **`std::function` overhead** | Delegates are cheap | `std::function` type-erases; has overhead vs template | Callback Patterns |

---

## Tier 3: Important (Rating 3)

These concepts **unlock performance and expressiveness** but require foundation from Tiers 1-2.

### Templates & Generic Programming

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **Function templates** | Generics with type erasure | Templates instantiate per-type; no runtime cost | Generic Sort |
| **Concepts (C++20)** | `where T : IComparable` | Concepts constrain templates with clear errors | Constrained Algorithms |
| **CRTP** | No equivalent | Static polymorphism without vtable | Mixin Pattern |

### Performance & Cache

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **Cache locality** | Memory layout is runtime's problem | Flat, contiguous data is orders of magnitude faster | Game of Life |
| **`std::vector<T>` vs `std::vector<unique_ptr<T>>`** | Reference types are always indirect | Indirection destroys cache performance | Entity System |
| **`std::span`** | `Span<T>` is special | `std::span` is just pointer+size; no magic | Array Utilities |

### Ranges & Algorithms (C++20)

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **`std::ranges` pipelines** | LINQ is lazy and magical | Ranges compose views without allocation | Data Pipeline |
| **Views vs owning ranges** | LINQ returns `IEnumerable` | Views borrow; lifetime of source matters | Advent of Code |
| **STL algorithms** | LINQ methods on collections | Algorithms are separate from containers | Algorithm Golf |

### Compile-Time Programming

| Concept | The C# Trap | The C++ Axiom | Cpptude Candidate |
|---------|-------------|---------------|-------------------|
| **`constexpr` functions** | No equivalent (limited `const`) | `constexpr` moves computation to compile time | Lookup Tables |
| **`consteval` (C++20)** | No equivalent | Guaranteed compile-time evaluation | Compile-Time Sudoku |
| **`static_assert`** | No equivalent | Compile-time assertions catch errors early | (integrated) |

---

## Tier 4: Good-to-Know (Rating 2)

| Concept | Notes | Cpptude Candidate |
|---------|-------|-------------------|
| **Variadic templates** | Parameter packs, fold expressions | Printf Implementation |
| **SFINAE** | Pre-C++20 constraint technique | (historical sidebar) |
| **Type traits** | `std::is_same`, `std::decay`, etc. | Type Inspector |
| **Allocators** | Custom memory allocation strategies | Pool Allocator |
| **Coroutines (C++20)** | Generators, async patterns | Async Pipeline |

---

## Tier 5: Seasoning (Rating 1)

| Concept | Notes |
|---------|-------|
| **Placement new** | Constructing in pre-allocated memory |
| **`volatile`** | Hardware/signal interaction (not threading!) |
| **Attributes** | `[[nodiscard]]`, `[[likely]]`, etc. |
| **Specialized casts** | `reinterpret_cast`, `const_cast` |
| **Bit manipulation** | `std::bit_cast`, `<bit>` header |

---

## Coverage Matrix

Track which concepts are covered by which Cpptudes:

| Cpptude | Primary KAP (Rating) | Secondary Concepts |
|---------|---------------------|-------------------|
| Sudoku | Stack allocation (5) | `std::array`, `constexpr`, bitwise ops |
| Game of Life | Cache locality (3) | Flat memory, `std::vector<bool>` pitfalls |
| Spell Corrector | `std::string_view` (4) | `std::unordered_map`, custom hash |
| Tree Builder | `std::unique_ptr` (4) | Move semantics, recursive ownership |
| File Processor | RAII (5) | Exception safety, destructor guarantees |
| Validator | `std::expected` (4) | Error handling without exceptions |
| *TBD* | Move semantics (4) | |
| *TBD* | Lambda captures (4) | |
| *TBD* | `std::ranges` (3) | |

---

## Sequencing Recommendation

Suggested order for learners:

1. **Sudoku** — Stack allocation, value semantics foundation
2. **File Processor** — RAII, destructor guarantees
3. **Tree Builder** — `unique_ptr`, ownership transfer
4. **Spell Corrector** — `string_view`, reference lifetime
5. **Game of Life** — Cache locality, flat memory
6. **Validator** — `expected`, modern error handling
7. **Data Pipeline** — `ranges`, lazy evaluation
8. **Advent of Code** — Integration of all concepts
