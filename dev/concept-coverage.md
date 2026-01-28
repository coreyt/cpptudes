# C++ Concept Coverage Plan

This document tracks which C++ concepts should be covered by Cpptudes.

## Dual Structure

This document serves both project requirements:

- **Requirement A (C++ teaching):** The "Concept," "C++ Axiom," and "Rating" columns define what C++ knowledge each cpptude must teach. The rating scale below rates **C++ concept importance** — how critical the concept is for writing correct, professional C++ code, regardless of the learner's background.
- **Requirement B (C# bridge):** The "C# Trap" column identifies where managed-runtime intuition conflicts with C++ reality, guiding how each cpptude baits and corrects C# habits.

A concept rated "5 - Critical" is critical **for C++** (e.g., a developer who doesn't understand object lifetime will write dangerous C++ code). The C# Trap column then explains *why* a C# developer is especially likely to get it wrong.

---

## Rating Scale (C++ Concept Importance)

| Rating | Level | Description |
|--------|-------|-------------|
| **5** | Critical | The developer is **dangerous** without this. Memory safety, lifetime, undefined behavior. |
| **4** | Required | Essential for modern C++. Cannot write professional code without it. |
| **3** | Important | Differentiates competent from proficient. Unlocks performance and expressiveness. |
| **2** | Good-to-Know | Useful in specific domains. Not every project needs it. |
| **1** | Seasoning | Niche features, legacy patterns, or stylistic choices. |

---

## Tier 1: Critical (Rating 5)

These are **foundational C++ concepts** where misunderstanding leads to unsafe, incorrect, or undefined behavior. The C# Trap column shows why C# developers are especially prone to getting these wrong.

### Memory & Lifetime

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **Stack vs Heap allocation** | Values live where declared; scope = lifetime | All objects live on heap, GC-managed | Sudoku |
| **RAII (Resource Acquisition Is Initialization)** | Destructor runs unconditionally at scope exit | `using`/`IDisposable` is optional pattern | File Processor |
| **Object lifetime & scope** | Objects die at `}` — references become invalid | Objects live until GC collects | Iterator Invalidation |
| **Dangling references** | Returning `&local` is UB; references don't extend lifetime | References can't dangle (GC) | String Parser |

### Value Semantics

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **Copy semantics** | `=` creates independent copy for value types | `=` creates alias for reference types | Data Pipeline |
| **Struct vs Class defaults** | Both are same; only default visibility differs | `struct` = value type, `class` = reference type | (integrated) |
| **Const correctness** | `const` is pervasive; immutability is the default assumption | `readonly` is limited; mutation is normal | (integrated) |

### Undefined Behavior

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **Signed integer overflow** | UB — compiler assumes it never happens | Wraps predictably | Overflow Calculator |
| **Null dereference** | UB — anything can happen | `NullReferenceException` (recoverable) | (integrated) |
| **Uninitialized variables** | Reading uninitialized memory is UB | Compiler enforces definite assignment | (sanitizer exercises) |

---

## Tier 2: Required (Rating 4)

These concepts are **essential for writing modern, professional C++** but don't immediately cause UB if misunderstood.

### Modern Ownership

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **`std::unique_ptr`** | Single owner; transfer via `std::move` | `new` + hope GC cleans up | Tree Builder |
| **`std::shared_ptr`** | Shared ownership has cost; prefer unique | All references are "shared" by default | Graph Algorithms |
| **Move semantics** | Move transfers resources; leaves source valid-but-empty | No equivalent; copies are cheap (refs) | Image Pipeline |

### Error Handling

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **`std::optional`** | Optional makes "might not exist" explicit in type | `null` is always an option | Config Parser |
| **`std::expected` (C++23)** | Expected carries error *or* value; no unwinding | Exceptions for control flow | Validator |
| **Exception safety guarantees** | Must design for basic/strong/nothrow guarantees | Exceptions are recoverable always | Transaction |

### String Handling

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **`std::string_view`** | Views don't own; underlying string must outlive view | Strings are immutable, interned, cheap to slice | Spell Corrector |
| **`std::string` allocation** | `std::string` may heap-allocate; SSO is not guaranteed | Strings are magic | Text Processor |

### Containers

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **`std::vector` growth** | Vector reallocates; iterators/refs invalidate on growth | `List<T>` is magic | Collection Builder |
| **Iterator invalidation** | Each container has specific invalidation rules | Enumeration can't mutate anyway | Iterator Lab |
| **`std::array` vs C-array** | `std::array` is fixed-size, stack-allocated, bounds-checked in debug | Arrays are objects with bounds checking | Sudoku |

### Functions & Lambdas

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **Lambda captures** | Capture by value or ref is explicit; ref captures can dangle | Closures capture by reference, GC keeps alive | Event System |
| **`std::function` overhead** | `std::function` type-erases; has overhead vs template | Delegates are cheap | Callback Patterns |

---

## Tier 3: Important (Rating 3)

These C++ concepts **unlock performance and expressiveness** but require foundation from Tiers 1-2.

### Templates & Generic Programming

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **Function templates** | Templates instantiate per-type; no runtime cost | Generics with type erasure | Generic Sort |
| **Concepts (C++20)** | Concepts constrain templates with clear errors | `where T : IComparable` | Constrained Algorithms |
| **CRTP** | Static polymorphism without vtable | No equivalent | Mixin Pattern |

### Performance & Cache

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **Cache locality** | Flat, contiguous data is orders of magnitude faster | Memory layout is runtime's problem | Game of Life |
| **`std::vector<T>` vs `std::vector<unique_ptr<T>>`** | Indirection destroys cache performance | Reference types are always indirect | Entity System |
| **`std::span`** | `std::span` is just pointer+size; no magic | `Span<T>` is special | Array Utilities |

### Ranges & Algorithms (C++20)

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **`std::ranges` pipelines** | Ranges compose views without allocation | LINQ is lazy and magical | Data Pipeline |
| **Views vs owning ranges** | Views borrow; lifetime of source matters | LINQ returns `IEnumerable` | Advent of Code |
| **STL algorithms** | Algorithms are separate from containers | LINQ methods on collections | Algorithm Golf |

### Compile-Time Programming

| C++ Concept | C++ Axiom *(Req A)* | C# Trap *(Req B)* | Cpptude Candidate |
|-------------|---------------------|---------------------|-------------------|
| **`constexpr` functions** | `constexpr` moves computation to compile time | No equivalent (limited `const`) | Lookup Tables |
| **`consteval` (C++20)** | Guaranteed compile-time evaluation | No equivalent | Compile-Time Sudoku |
| **`static_assert`** | Compile-time assertions catch errors early | No equivalent | (integrated) |

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
