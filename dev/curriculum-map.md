# Curriculum Map: Scope and Sequence

This is the master reference for creating cpptude content. It defines the order in which cpptudes are delivered, what each session covers, what the learner must already know, and how mastery is assessed.

---

## Overview

### Target Audience

Experienced C# software engineers (3+ years professional experience) who want to become competent C++ engineers. These learners:

- Have strong programming fundamentals: algorithms, data structures, OOP, design patterns.
- Have deep C# expertise: LINQ, async/await, generics, IDisposable, garbage collection mental model.
- Have zero or near-zero C++ experience. Their mental model of memory, lifetime, and object semantics is shaped entirely by the CLR.
- Are working professionals with limited study time. They learn best when content is immediately applicable and respects their existing competence.

### Prerequisite Knowledge

Before starting cpptude #0, the learner should:

- Be able to write, compile, and run a "Hello, World" C++ program using their local toolchain (GCC, Clang, or MSVC).
- Have a working CMake installation (version 3.20+).
- Understand basic C++ syntax (variables, functions, if/else, loops, classes). This is the "syntax is easy" part that the README explicitly excludes from the curriculum's focus.

### Format

- **Delivery:** Once-per-week class sessions, 2-3 hours each.
- **Total duration:** 16 weeks (Setup Guide + 15 cpptudes).
- **Session structure:** Each session follows a consistent five-phase pattern (detailed below).
- **Homework:** Lightweight, optional, reinforcement-only. No new concepts in homework.
- **Primary KAP rule:** Each session introduces exactly one new Knowledge Acquisition Point. Secondary concepts may appear but do not receive primary instructional focus.

### Pacing Assumptions

- The 2-3 hour session length assumes an adult learner who is actively coding during the session, not passively watching.
- Weeks 1-5 (Tier 1 foundation) are slower-paced. These sessions establish the mental model shifts that everything else depends on. Expect the full 3 hours.
- Weeks 6-10 build fluency. The learner has the foundational mental model and is extending it. Sessions may complete in 2-2.5 hours.
- Weeks 11-15 are application and integration. The learner is applying established concepts to new domains. Sessions may complete in 2 hours.
- Week 16 (Advent of Code) is a capstone that may extend beyond 3 hours or be split across two sessions.

---

## Session Structure (Every Week)

Each cpptude session follows this five-phase structure. The phases are designed around andragogical principles: adults learn best when they understand why they are learning something, can connect it to their experience, and can apply it immediately.

### Phase 1: Warm-Up and Review (15-20 minutes)

- Brief review of the previous week's primary KAP. Ask the learner to explain the concept in their own words.
- Connect the previous week's concept to this week's topic. Explicitly state the prerequisite relationship: "Last week you learned X. This week, X is the foundation for Y."
- If homework was assigned, review it briefly. Homework is never graded -- it exists to prime the learner's thinking.

### Phase 2: Trap Encounter (30-45 minutes)

- Present the problem domain. The problem should be familiar from the learner's C# experience (e.g., file processing, event handling, data transformation).
- Let the learner attempt a solution using their C# instincts. The cpptude is designed so that the natural C# approach triggers a visible failure.
- The failure activates: compile error, sanitizer report, benchmark failure, or incorrect output. The learner sees concrete evidence that their instinct is wrong.
- This phase is the heart of the pedagogical strategy. Do not skip it or shortcut it. The learner must experience the failure firsthand.

### Phase 3: Concept Introduction (20-30 minutes)

- Explain the C++ axiom that the trap violated. Use the "C# Bridge" contrast format: "In C#, you expect X. In C++, the reality is Y, because Z."
- Provide the mechanical sympathy explanation: what the machine is doing, where memory lives, when objects are created and destroyed.
- Introduce new vocabulary (limit to 3-5 new terms per session to manage cognitive load).
- Show the correct C++ pattern. Explain why it works and how it prevents the failure the learner just experienced.

### Phase 4: Guided Practice (30-45 minutes)

- The learner implements the correct solution with instructor support.
- The guardrails (sanitizers, tests, benchmarks) now pass. The learner sees concrete evidence that the correct approach works.
- The instructor calls out secondary concepts as they arise ("Notice we're using `const` here -- that's the compiler-enforced contract we introduced in Week 1").
- This is where spiral reinforcement happens: prior concepts reappear naturally in the context of the new problem.

### Phase 5: Independent Practice and Next Step (20-30 minutes)

- The learner attempts the "Next Step" challenge independently. This is a variation or extension of the problem that reinforces the primary KAP without introducing new concepts.
- If the learner completes the challenge, optional homework is assigned (see Homework Guidelines below).
- Close with a preview of next week's topic: "Next week we'll build on Y to learn Z."

---

## Scope and Sequence

### Foundation Block: Weeks 0-5

These sessions establish the core mental model shifts. Every subsequent session depends on concepts introduced here. Do not skip or reorder sessions in this block.

| Week | Cpptude | Primary KAP (Rating) | Secondary Concepts | Prerequisites | C# Trap Targeted | Session Outline |
|:----:|---------|:--------------------:|--------------------|--------------:|------------------|-----------------|
| 0 | Setup Guide | Tooling | ASan, UBSan, `-Wall -Wextra -Werror` | None | "Runtime catches my errors" -- C# developers rely on exceptions for safety; C++ errors are often silent | Phase 1: Introductions, toolchain verification. Phase 2: Build trivial program, deliberately trigger use-after-free and signed overflow. Phase 3: Explain what ASan/UBSan are and why they replace the CLR safety net. Phase 4: Configure build system with sanitizer flags. Phase 5: Trigger and read three different sanitizer reports. |
| 1 | Sudoku | Stack allocation (5) | `std::array`, `constexpr`, bitwise ops, const introduction, brace initialization | Setup Guide | "All objects live on the heap" -- C# developers use `new` for everything because the GC manages lifetime | Phase 1: Review sanitizer setup. Phase 2: Learner solves Sudoku, naturally reaching for `new` to allocate the board. Guardrail catches heap allocation. Phase 3: Explain stack vs heap, scope = lifetime, introduce `const` and brace initialization. Phase 4: Implement stack-only solution. Phase 5: Next Step challenge (e.g., constexpr validation). |
| 2 | File Processor | RAII (5) | Exception safety, destructor guarantees, multi-file structure, const member functions, Build Model sidebar | Sudoku | "`IDisposable`/`using` is the cleanup pattern" -- C# developers treat resource cleanup as optional because the GC is the real safety net | Phase 1: Review stack allocation. Phase 2: Learner processes files using manual open/close, forgets to close on error path. ASan detects leak. Phase 3: Explain RAII, destructors, scope-based cleanup. Introduce Build Model sidebar (C++ compilation model vs C# project model). Phase 4: Implement RAII wrapper. Phase 5: Next Step (RAII for a different resource type). |
| 3 | Parameter Passing | Parameter passing (5) | `const T&`, value vs reference, `T&&` preview, const reference parameters | File Processor | "Value types pass by value, reference types pass by reference" -- C# has two modes; C++ has four for any type | Phase 1: Review RAII. Phase 2: Learner writes functions passing large structs by value everywhere. Benchmark shows copy overhead. Phase 3: Explain T, const T&, T&, preview T&&. Phase 4: Refactor function signatures with correct parameter modes. Phase 5: Next Step (choose parameter modes for a set of function signatures). |
| 4 | Image Pipeline | Move semantics (4) | `noexcept`, moved-from state, rvalue references | Parameter Passing | "Assignment makes two variables refer to the same thing" -- C# `=` on reference types creates an alias; C++ `std::move` transfers and empties | Phase 1: Review parameter passing, connect T&& preview to this week. Phase 2: Learner copies image buffers instead of moving them. Benchmark shows unnecessary allocation. Learner moves a resource, then tries to use the source. Phase 3: Explain move semantics, moved-from state, noexcept. Phase 4: Implement move-based pipeline. Phase 5: Next Step (verify moved-from state is safe but empty). |
| 5 | String Parser | Dangling references (5) | Use-after-free, reference lifetime | Image Pipeline | "References are always valid" -- C# references cannot dangle because the GC guarantees liveness | Phase 1: Review move semantics. Phase 2: Learner returns reference/string_view to a local variable. ASan fires on access. Phase 3: Explain dangling references, lifetime rules, why GC prevents this in C#. Phase 4: Fix by returning by value or extending lifetime. Phase 5: Next Step (identify dangling references in code review exercise). |

### Fluency Block: Weeks 6-10

These sessions build fluency with ownership, value semantics, and type design. The foundational mental model is established; these sessions extend and deepen it.

| Week | Cpptude | Primary KAP (Rating) | Secondary Concepts | Prerequisites | C# Trap Targeted | Session Outline |
|:----:|---------|:--------------------:|--------------------|--------------:|------------------|-----------------|
| 6 | Tree Builder | `unique_ptr` (4) | Raw pointers as non-owning observers, recursive ownership | String Parser | "`new` + hope GC cleans up" -- C# developers either fear all pointers or treat raw pointers like C# references | Phase 1: Review dangling references. Phase 2: Learner builds tree with `new`/`delete` or `shared_ptr` everywhere. ASan detects leak (manual) or benchmark shows overhead (shared_ptr). Phase 3: Explain unique_ptr, single ownership, raw pointer = non-owning observer. Phase 4: Implement tree with unique_ptr owning children, raw pointer for parent backlinks. Phase 5: Next Step (add tree traversal that uses non-owning observers). |
| 7 | Spell Corrector | `string_view` (4) | `std::unordered_map`, custom hash, reference lifetime | Tree Builder | "Strings are immutable and cheap to slice" -- C# developers `Substring()` freely because strings are GC-managed | Phase 1: Review unique_ptr and ownership. Phase 2: Learner stores string_views that outlive their source string. ASan detects dangling access. Phase 3: Explain string_view lifetime, contrast with C# string immutability. Phase 4: Implement with correct lifetime management. Phase 5: Next Step (implement with custom hash). |
| 8 | Data Pipeline | Copy semantics (5) | `std::ranges`, object slicing (secondary), lazy evaluation | Spell Corrector | "`=` creates alias for reference types" -- C# assignment of class instances creates a shared reference, not an independent copy | Phase 1: Review string_view. Phase 2: Learner assigns derived to base by value, expecting polymorphic behavior. Sliced object loses derived data. Phase 3: Explain copy semantics, value semantics, object slicing. Phase 4: Implement data pipeline with correct value semantics and ranges. Phase 5: Next Step (add range view composition). |
| 9 | Event System | Lambda captures (4) | Dangling captures, callback lifetime | Data Pipeline | "Closures capture by reference and GC keeps captured objects alive" -- C# closures promote captured variables to heap; C++ `[&]` captures dangle when scope exits | Phase 1: Review copy semantics. Connect to dangling references from Week 5. Phase 2: Learner stores callback with `[&]` capture; callback invoked after scope exits. ASan fires. Phase 3: Explain lambda capture modes, capture lifetime, contrast with C# closures. Phase 4: Implement with capture by value or shared ownership. Phase 5: Next Step (design a callback registration system with correct lifetime). |
| 10 | Rule of Zero | Rule of Zero (4) | Rule of Five (advanced), special member functions | Event System | "I wrote the constructor; I'm done" -- C# developers never think about copy/move/destroy because the runtime handles it | Phase 1: Review lambda captures. Phase 2: Learner writes class with destructor and copy constructor but forgets move operations. Vector copies instead of moves. Phase 3: Explain Rule of Zero ("use RAII members and the compiler writes correct special members"), then Rule of Five for the exception case. Phase 4: Refactor to Rule of Zero. Phase 5: Next Step (identify which classes need Rule of Five vs Rule of Zero). |

### Application Block: Weeks 11-15

These sessions apply the established foundation to performance, error handling, and undefined behavior. The learner is now operating with a corrected mental model and building professional-grade skills.

| Week | Cpptude | Primary KAP (Rating) | Secondary Concepts | Prerequisites | C# Trap Targeted | Session Outline |
|:----:|---------|:--------------------:|--------------------|--------------:|------------------|-----------------|
| 11 | Game of Life | Cache locality (3) | Flat memory, `vector<bool>` pitfalls | Rule of Zero | "Memory layout is the runtime's problem" -- C# developers never think about cache lines because the JIT/GC manages layout | Phase 1: Review Rule of Zero. Phase 2: Learner implements with pointer-chasing structure (linked list, like C# object graph). Benchmark shows 10-100x slower than flat array. Phase 3: Explain cache locality, L1/L2/L3 cost model, why contiguous data wins. Phase 4: Implement with flat `vector<uint8_t>`. Phase 5: Next Step (experiment with different memory layouts and measure). |
| 12 | Validator | `std::expected` (4) | Error handling without exceptions | Game of Life | "Exceptions are the error handling mechanism" -- C# uses exceptions for recoverable errors because the GC handles cleanup during unwinding | Phase 1: Review cache locality. Phase 2: Learner uses exceptions for validation errors in a hot path. Benchmark shows unwinding cost. Phase 3: Explain `std::expected`, value-based error handling, when exceptions are still appropriate. Phase 4: Implement validator with `std::expected`. Phase 5: Next Step (chain expected values through a pipeline). |
| 13 | Transaction | Exception safety (4) | Basic/strong/nothrow guarantees, RAII as safety net | Validator | "Exceptions are always recoverable; GC cleans up" -- C# developers assume exception propagation is safe because the GC handles resource cleanup | Phase 1: Review `std::expected`. Phase 2: Learner acquires resources without RAII, exception propagates, resources leak. ASan detects. Phase 3: Explain exception safety guarantees, frame RAII as the C++ equivalent of C#'s GC safety net. Phase 4: Implement with RAII and strong guarantee. Phase 5: Next Step (analyze existing code for exception safety level). |
| 14 | Overflow Calculator | Signed integer overflow (5) | UB mechanics, compiler optimization assumptions | Transaction | "Integer overflow wraps predictably" -- C# `unchecked` wraps; C++ treats signed overflow as UB and the compiler optimizes accordingly | Phase 1: Review exception safety. Phase 2: Learner writes arithmetic assuming overflow wraps. UBSan fires. Optimized build produces impossible results (compiler removed "impossible" branch). Phase 3: Explain UB as "the compiler assumes this never happens and optimizes as if it can't." Phase 4: Implement with overflow checks or unsigned types. Phase 5: Next Step (find UB in provided code snippets). |

### Capstone: Week 16

| Week | Cpptude | Primary KAP (Rating) | Secondary Concepts | Prerequisites | C# Trap Targeted | Session Outline |
|:----:|---------|:--------------------:|--------------------|--------------:|------------------|-----------------|
| 16 | Advent of Code | Integration | All prior concepts | All above | All prior traps -- learner must independently choose correct patterns without guidance | Phase 1: Review the full concept dependency chain. Phase 2: Learner tackles a multi-part problem that requires stack allocation, RAII, correct parameter passing, move semantics, lifetime management, and value semantics. No specific trap is baited -- the learner must apply judgment. Phase 3: Code review of the learner's solution, identifying where C# instincts surfaced and how they were (or were not) corrected. Phase 4-5: Refactoring and discussion of alternative approaches. |

---

## Concept Dependency Graph

This text-based graph shows prerequisite chains. An arrow (`-->`) means "must come before." Concepts at the same indentation level within a chain are sequential. Concepts in separate chains are independent (can be taught in either order relative to each other, though the sequencing recommendation above interleaves them for variety).

```
Primary Chain (Lifetime and Ownership):

  Setup Guide (#0)
    |
    v
  Stack Allocation (#1)
    |
    v
  RAII (#2)
    |
    v
  Parameter Passing (#3)
    |
    +--> Move Semantics (#4)
    |      |
    |      +--> unique_ptr (#6)
    |      |      |
    |      |      v
    |      |    string_view (#7)
    |      |
    |      +--> [feeds into Rule of Zero (#10)]
    |
    +--> Dangling References (#5)
           |
           +--> Lambda Captures (#9)
           |
           +--> [string_view (#7) also depends on this]


Secondary Chain (Value Semantics):

  Stack Allocation (#1)
    |
    v
  Copy Semantics (#8)
    |
    +--> Object Slicing (secondary in #8)
    |
    +--> Rule of Zero (#10)
           |
           v
         [requires both Move Semantics (#4) and Copy Semantics (#8)]


Tertiary Chain (Error Handling):

  RAII (#2)
    |
    v
  std::expected (#12)
    |
    v
  Exception Safety (#13)


Independent (sequenced by pedagogical variety, not strict dependency):

  Cache Locality (#11) -- requires general foundation, no specific prerequisite beyond #10
  Signed Integer Overflow (#14) -- requires UBSan from #0, no strict concept prerequisite
  Integration (#16) -- requires all above
```

---

## Homework Guidelines

### Purpose

Homework exists for one purpose: **reinforcement of the current week's primary KAP through reflection or low-stakes practice.** Homework never introduces new concepts. It never requires significant time investment. It respects the learner's status as a working professional with limited discretionary time.

### Format

Each week's homework is one of the following:

1. **Reflective annotation.** "Re-read your solution from today. Annotate each function signature with a comment explaining why you chose that parameter mode (value, const ref, ref, rvalue ref)." This reinforces the concept through metacognition -- the learner explains their own reasoning.

2. **Next Step challenge completion.** If the learner did not complete the Next Step challenge during the session, finishing it is the homework. This provides additional practice on the same concept without introducing new material.

3. **Code reading exercise.** "Read this short C++ code snippet (provided). Identify where the C# instinct would lead you astray and what the correct C++ behavior is." This builds the habit of reading C++ with a C++ mental model rather than a C# mental model.

### What Homework Is Not

- Homework is never mandatory. Missing homework does not block progress.
- Homework never introduces vocabulary, concepts, or patterns not covered in the session.
- Homework is never graded. It is self-directed reinforcement.
- Homework should take no more than 30 minutes.

### Example Homework by Week

| Week | Homework |
|:----:|----------|
| 0 | Read the ASan documentation page for one error type you triggered today. Write a one-sentence summary of what ASan checks for. |
| 1 | Re-read your Sudoku solution. Annotate where each object's lifetime begins and ends. |
| 2 | Re-read your File Processor solution. Identify every RAII boundary. What would leak if you removed the destructor? |
| 3 | Write the function signatures (not implementations) for five functions that process different types. Choose the correct parameter mode for each. |
| 4 | Re-read your Image Pipeline solution. Mark every point where a move occurs. What is the state of the source after each move? |
| 5 | Read the provided code snippet. Circle every dangling reference. Explain why each one dangles. |
| 6 | Re-read your Tree Builder solution. For every pointer, annotate whether it is owning or non-owning. |
| 7 | Read the provided code snippet. Identify which string_views will dangle and which are safe. |
| 8 | Re-read your Data Pipeline solution. Identify every copy. Which copies are intentional? Which could be eliminated? |
| 9 | Read the provided callback code. Identify which captures are safe and which will dangle. |
| 10 | Look at three of your own previous cpptude solutions. Do any of them violate Rule of Zero? Could any be simplified? |
| 11 | Try the Next Step challenge: experiment with a different memory layout and measure the performance difference. |
| 12 | Try the Next Step challenge: chain `std::expected` values through a three-stage pipeline. |
| 13 | Read the provided code snippet. Classify each function as providing the basic, strong, or nothrow guarantee. |
| 14 | Read the provided code snippet. Find all instances of undefined behavior. |

---

## Assessment Approach

### How the Learner Knows They Learned the Concept

The cpptude guardrails **are** the assessment. Each cpptude has a built-in bait/failure/verification cycle (see `best-practices.md` Principle 9 and the Coverage Matrix in `concept-coverage.md`). The learner demonstrates mastery by:

1. **Experiencing the failure.** The learner's C# instinct triggers a visible failure (compile error, sanitizer report, benchmark failure). This demonstrates that the learner has encountered the concept in a way that their prior mental model cannot explain.

2. **Implementing the correct solution.** The guardrails pass. The sanitizer is clean. The benchmark meets the threshold. This demonstrates that the learner can apply the correct C++ pattern.

3. **Completing the Next Step challenge.** The learner extends the solution independently, demonstrating transfer -- the ability to apply the concept in a slightly different context without guidance.

4. **Explaining the concept in their own words** (Phase 1 review of the following week). The learner articulates the C++ axiom and the C# trap it corrects. This demonstrates understanding, not just execution.

### How the Instructor Knows the Concept Was Acquired

- **Guardrail pass/fail is binary.** If the learner's solution passes the guardrails, the mechanical skill is demonstrated.
- **Phase 1 review is diagnostic.** If the learner cannot explain last week's concept, the instructor should revisit it before proceeding. This is not a failure -- it is a signal that the concept needs more time.
- **Homework annotations reveal depth.** If the learner's annotations show correct reasoning about lifetime, ownership, or value semantics, the concept is internalized. If annotations are vague or incorrect, the instructor should address the gap in the next session's warm-up.

### There Are No Separate Tests

Cpptudes are competency-based, not exam-based. The guardrails provide continuous assessment at every session. A separate test would add cognitive overhead without adding diagnostic value -- the guardrails already provide concrete, specific feedback on whether the learner has acquired each concept.

---

## Milestone Checkpoints

These are natural pause points where the learner should have a solid foundation before continuing. If the learner is struggling at a milestone, the instructor should revisit earlier material rather than pushing forward. Advancing without foundation creates a cascade of confusion that becomes harder to remediate.

### Milestone 1: After Week 5 (String Parser) -- "Lifetime Fluent"

**What the learner should know:**
- Objects live on the stack by default. Scope determines lifetime.
- RAII means destructors run at scope exit, unconditionally. This is the cleanup mechanism.
- C++ has four parameter passing modes. `const T&` is the most common for read-only access to large types.
- Move semantics transfer ownership. The source becomes empty. This is fundamentally different from C# assignment.
- References can dangle. The GC does not exist in C++. The programmer is responsible for lifetime.

**Checkpoint activity:** Give the learner a short code snippet with three lifetime bugs (dangling reference, use-after-move, missing RAII). The learner should identify all three and explain what the correct pattern is for each. If the learner misses any, revisit the relevant cpptude before proceeding to Week 6.

**Why this is a milestone:** Everything after Week 5 assumes the learner has internalized deterministic lifetime. Ownership (unique_ptr), string_view, copy semantics, and lambda captures all build on the mental model established in Weeks 1-5. A learner who is still thinking in GC terms will struggle with every subsequent concept.

### Milestone 2: After Week 10 (Rule of Zero) -- "Ownership Fluent"

**What the learner should know:**
- `unique_ptr` expresses single ownership. Raw pointers express non-owning observation.
- `string_view` borrows. The source must outlive the view.
- `=` in C++ creates an independent copy for value types. Object slicing occurs when copying polymorphic types by value.
- Lambda captures follow the same lifetime rules as references. `[&]` is dangerous if the lambda outlives the scope.
- Rule of Zero: if all your members are RAII types, the compiler writes correct special members. You usually do not need to write any of them.

**Checkpoint activity:** Give the learner a class design problem: "Design a `Document` class that owns a `std::vector<Page>`, has a title (`std::string`), and supports an `on_modified` callback. Specify the special member functions (or explain why you do not need to write them). Choose the correct parameter modes for the constructor and all public methods." The learner should produce a correct design using Rule of Zero, `const T&` for read access, and value or `std::function` for the callback. If the learner reaches for `shared_ptr`, manual `new`/`delete`, or raw pointer ownership, revisit Weeks 6-10.

**Why this is a milestone:** Weeks 11-15 are application-focused. They assume the learner can independently make correct ownership, lifetime, and value semantics decisions. A learner who still needs guidance on these decisions is not ready for performance optimization or error handling patterns.

### Milestone 3: After Week 14 (Overflow Calculator) -- "Production Ready"

**What the learner should know:**
- All Tier 1 and Tier 2 concepts. The learner can write correct, idiomatic C++ code that handles resources, errors, and edge cases.
- Cache locality matters. Contiguous data structures outperform pointer-chasing structures by orders of magnitude.
- `std::expected` is the modern approach to recoverable errors. Exceptions are for exceptional conditions.
- Exception safety guarantees (basic, strong, nothrow) are design decisions, not accidents.
- Undefined behavior means the compiler assumes it cannot happen and optimizes accordingly. UB is not "wrong output" -- it is "no defined behavior at all."

**Checkpoint activity:** Code review. Give the learner a 50-100 line C++ program written by a "C# developer who just learned the syntax." The program should contain at least five distinct issues (heap overuse, missing RAII, dangling reference, unnecessary copy, UB). The learner should identify all issues, explain why each is wrong, and provide the correct C++ pattern. If the learner misses more than one issue, they are not ready for the capstone.

**Why this is a milestone:** The capstone (Week 16) removes all guardrails. The learner must independently apply the full set of concepts without guidance. A learner who cannot identify issues in code review cannot be trusted to avoid them in their own code.

---

## Spiral Reinforcement Map

Prior concepts do not disappear after their primary session. They reappear as secondary concerns in later sessions. This table shows where each primary concept resurfaces.

| Concept (Primary Week) | Reinforced In |
|------------------------|---------------|
| Stack allocation (#1) | Every cpptude -- all solutions prefer stack allocation unless heap is justified |
| RAII (#2) | File Processor (#2), Transaction (#13), any cpptude using file/network/lock resources |
| Parameter passing (#3) | Every function signature in every subsequent cpptude |
| Move semantics (#4) | Tree Builder (#6), Rule of Zero (#10), any cpptude that transfers ownership |
| Dangling references (#5) | Spell Corrector (#7), Event System (#9), any cpptude involving reference lifetime |
| `unique_ptr` (#6) | Any cpptude involving heap-allocated owned resources |
| `string_view` (#7) | Data Pipeline (#8), any cpptude processing text |
| Copy semantics (#8) | Rule of Zero (#10), any cpptude involving assignment or containers |
| Lambda captures (#9) | Any cpptude using callbacks or functional patterns |
| `const` (introduced #1, deepened #2-#3) | Every cpptude -- rigorous const usage is a Quality Checklist requirement |

---

## Document Cross-References

- **`best-practices.md`** -- Design principles, coding standards, and Quality Checklist for every cpptude.
- **`concept-coverage.md`** -- Tier tables, Coverage Matrix with bait/failure/tool per cpptude, C# Trap Severity ratings, sequencing constraints, and deferred topics.
- **`curr-designer-criticism-response.md`** -- Rationale for every structural decision in this curriculum, including verdicts on the original criticisms and the prioritized action list that produced these documents.
