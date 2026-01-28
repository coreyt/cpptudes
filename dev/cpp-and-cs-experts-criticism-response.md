  ---
  Criticism 1 — In-Line Review

  Overall Alignment: Strong

  [cpp-expert] Agreed. The dual-requirement structure (Req A: C++ correctness, Req B: C# bridge) is sound and the three README problems map cleanly to Tier 1
  concepts. No notes here. [/cpp-expert]

  [csharp-expert] Agreed. The dual structure is the right call. C# developers don't fail at C++ because the syntax is hard — they fail because their instincts
   are wrong. The three specific problems cited (reaching for new, trusting GC, treating copies as aliases) are genuinely the top three reflex errors I'd
  expect from an experienced C# developer writing their first C++ code. These aren't strawmen. [/csharp-expert]

  ---
  Critical Gaps

  Point 1 — Dangling references should be cpptude #2 or #3

  [cpp-expert] Correct on the criticality. Dangling references are the single most common source of use-after-free in C++ code written by developers coming
  from managed languages. However, I'd push back on making it #2 or #3. The recommended sequence currently places RAII (#2, File Processor) before any
  lifetime-of-reference material. That's the right pedagogical order: you need to understand deterministic destruction before you can understand why a
  reference dangles. Dangling references should slot in at #4 (before the current Spell Corrector, or combined with String Parser), not earlier. The
  dependency chain is: stack allocation → RAII/scope → dangling references → string_view. [/cpp-expert]

  Point 1 — Dangling references (Rating 5) — should be cpptude #2 or #3

  [csharp-expert] The criticality rating is correct, but for a reason the criticism doesn't fully articulate. In C#, references cannot dangle. This isn't a
  rule developers learn — it's a property of the runtime that they've never had to think about. A C# developer has written thousands of methods that return
  references to objects, pass references to callbacks, store references in collections — and none of them have ever become invalid unexpectedly. The GC
  guarantees this.

  The result is that C# developers have zero instinct for "will this reference still be valid?" They don't ask the question because they've never needed to.
  This makes dangling references not just a "critical concept" but an invisible one — the learner doesn't know to look for it.

  That said, I agree with the cpp-expert's sequencing argument: you need RAII/scope first. The learner needs to understand "objects die at }" before they can
  understand "...and that means your reference to it is now garbage." [/csharp-expert]


  Point 2 — Move semantics must come before Tree Builder

  [cpp-expert] This is correct and is a real sequencing defect. Tree Builder lists unique_ptr as primary KAP and move semantics as secondary, but you cannot
  transfer unique_ptr ownership without understanding std::move. A learner encountering auto node = std::move(child) in Tree Builder without prior exposure to
   move semantics will be confused or will cargo-cult the pattern. Move semantics needs a dedicated cpptude before Tree Builder. The coverage matrix already
  has a TBD slot for move semantics — it just needs to be ordered before Tree Builder (current #3). Recommended fix: insert a move semantics cpptude (e.g.,
  Image Pipeline) between File Processor (#2) and Tree Builder (#3). [/cpp-expert]

  [csharp-expert] Agreed on the sequencing defect, but I want to flag something about how move semantics should be taught to C# developers.

  In C#, there is no concept of "moving" an object. When you write var x = y; for a reference type, both x and y point to the same object. Nobody "loses"
  anything. The closest analogy is transferring a StreamReader and then not using the original handle — but C# doesn't enforce that at the language level.

  The C# instinct that will break here is: "assignment makes two variables refer to the same thing." In C++, auto x = std::move(y); makes x the owner and
  empties y. A C# developer will expect y to still be usable. The trap should demonstrate this: let the learner move a resource and then try to use the
  source. Make the moved-from state visible (e.g., a container that becomes empty, a unique_ptr that becomes nullptr). [/csharp-expert]

  ---
  Medium Priority

  Point 3 — Const correctness as "integrated" may be insufficient

  [cpp-expert] I partially disagree. const correctness in C++ is pervasive — it appears in every function signature, every member function, every parameter.
  That's precisely why it works better as an integrated concept than as a standalone cpptude. A dedicated cpptude for const risks being artificial (what
  problem domain naturally teaches const?). The right approach is: every cpptude from #1 onward uses const rigorously, and the lesson text calls it out
  explicitly the first time. However, the coverage document should state this strategy clearly rather than just saying "(integrated)" with no plan. Add a note
   like: "Const correctness is introduced in Cpptude #1 and reinforced in every subsequent cpptude. Each cpptude's lesson text explains at least one const
  usage in context." [/cpp-expert]

  [csharp-expert] The C# perspective here is important: C# has readonly fields, const compile-time constants, and init-only properties (C# 9+), but none of
  these are comparable to C++ const. In C#:

  - readonly only applies to fields, not parameters or return types
  - You can't mark a method as "this doesn't mutate the object" (no const member functions)
  - You can't pass a parameter as "you may read but not write this" (no const T&)
  - Mutation is the default in C# — you opt in to immutability with readonly struct or records

  The result: C# developers will habitually write mutating code and won't understand why C++ code is covered in const. They'll see it as noise. The lesson
  needs to explain: "In C++, const is the primary mechanism for communicating intent about mutation. It's not decoration — it's a compiler-enforced contract."

  I agree with treating this as "integrated" rather than standalone, but each cpptude must explicitly call out its const usage the first few times. The
  learner needs to build the habit of reading const as meaningful, not as C++ boilerplate. [/csharp-expert]

  Point 4 — UB concepts need a dedicated cpptude or "UB Showcase"

  [cpp-expert] I disagree with a combined "UB Showcase" cpptude. The problem with a UB showcase is that it becomes a zoo of disconnected examples rather than
  a coherent exercise solving a real problem — which violates Principle 2 (One Primary KAP). UB is better taught through the sanitizer toolchain integrated
  into specific problems. Signed integer overflow is well-placed in "Overflow Calculator." Null dereference should surface naturally in pointer/ownership
  cpptudes. Uninitialized variables should be caught by compiler warnings (-Werror) and sanitizers in every cpptude. What is needed is what Criticism 2 calls
  a "Tooling & Verification" parallel track — ensure every cpptude's build specifies -fsanitize=address,undefined so UB surfaces reliably. [/cpp-expert]


  Point 4 — Undefined Behavior concepts need coverage

  [csharp-expert] The C# trap here is profound: C# developers have never experienced undefined behavior. In C#, every runtime error is defined:

  - Null dereference → NullReferenceException
  - Array out of bounds → IndexOutOfRangeException
  - Integer overflow → wraps (unchecked) or throws OverflowException (checked)
  - Stack overflow → StackOverflowException

  C# developers have a mental model where "wrong code produces wrong results or throws an exception." The concept that wrong code can do literally anything —
  pass tests, work in debug, crash in release, format your hard drive (the classic joke) — is alien. They will not believe you until they see it.

  I agree with the cpp-expert that a "UB zoo" cpptude is the wrong format. But each UB concept needs a moment where the learner's C# instinct ("this will
  throw an exception") is explicitly contradicted. The signed integer overflow cpptude is the best vehicle: show that INT_MAX + 1 doesn't wrap or throw — the
  compiler assumes it never happens and optimizes accordingly. That's the "aha moment" for UB. [/csharp-expert]


  ---
  Point 5 — Lambda captures (TBD, high priority)
  Required Gaps (Rating 4)

  [csharp-expert] This is a critical C# trap and the criticism undersells it. In C#, closures capture variables by reference and the GC keeps the captured
  objects alive. This is automatic and invisible. A C# developer writes:

  Func<int> MakeCounter() {
      int count = 0;
      return () => ++count; // count is captured — GC keeps it alive
  }

  This works perfectly in C# because the GC extends the lifetime of count (it gets promoted to a compiler-generated class on the heap). The equivalent C++
  code with [&count] is undefined behavior — count is destroyed when MakeCounter returns, and the lambda holds a dangling reference.

  The worst part: the C++ code will often appear to work because the stack memory hasn't been reused yet. The C# developer will think they've written correct
  code. Only ASan or a stress test will reveal the bug. This is exactly the kind of trap the curriculum is designed to surface.

  The "Event System" candidate is excellent because event handlers stored for later invocation are the exact production scenario where this bug appears.
  [/csharp-expert]

  [cpp-expert] Agreed. Dangling lambda captures are a particularly nasty trap for C# developers because C# closures capture by reference and the GC keeps the
  captured object alive. In C++, a lambda that captures a local by reference and then outlives the scope is UB — and it's invisible UB (no crash, just
  corrupted data). This needs a dedicated cpptude. The "Event System" candidate in the coverage doc is a good fit: event handlers that store callbacks are
  exactly where dangling captures appear in production code. [/cpp-expert]



  Point 6 — Iterator invalidation appears in two places

  [csharp-expert] The C# context: in C#, you cannot modify a collection during enumeration. If you try, you get InvalidOperationException. C# developers know
  this rule and work around it (usually by calling .ToList() first or using a separate list).

  The C++ trap is different and worse: you can modify a container during iteration, and the compiler won't stop you. The iterator just becomes invalid
  silently. Worse, std::vector can invalidate iterators even if you're not iterating — any operation that causes reallocation (push_back, insert, resize)
  invalidates all iterators, pointers, and references.

  The C# developer's instinct — "I'll get an exception if I do something wrong" — is completely wrong here. This is UB territory, and the learner needs to see
   it fail under ASan. I agree with the cpp-expert that this should be a Tier 1 concept since it's fundamentally about lifetime. [/csharp-expert]
   
  [cpp-expert] This is a real inconsistency in the coverage document. Iterator invalidation appears in Tier 1 ("Object lifetime & scope" → Iterator
  Invalidation) and Tier 2 ("Iterator invalidation" → Iterator Lab, "std::vector growth" → Collection Builder). The fix: Iterator invalidation is
  fundamentally a lifetime problem — a reference/iterator becomes dangling because the container mutated. It belongs in Tier 1 as a lifetime concept. The Tier
   2 entries should be deduplicated. One cpptude that covers vector reallocation invalidation as a specific case of lifetime violation is sufficient.
  [/cpp-expert]

  Point 7 — Exception safety guarantees need a cpptude

  [csharp-expert] The C# perspective on this is nuanced. C# developers do use exceptions extensively, but with a completely different mental model:

  - Exceptions are always recoverable (catch and continue)
  - finally blocks handle cleanup
  - The GC cleans up objects even if an exception propagates
  - There's no concept of "basic guarantee" vs "strong guarantee" — objects are always in a valid state because the GC handles resource management

  In C++, an exception propagating through code that has acquired resources (heap memory, file handles, locks) without RAII wrappers will leak those
  resources. The C# developer will write:

  var resource = AcquireResource();
  DoSomethingThatMightThrow(); // If this throws, GC cleans up resource
  UseResource(resource);

  The equivalent C++ code leaks. The lesson must make the connection: "In C#, the GC is your safety net. In C++, RAII is your safety net, and if you don't use
   it, exceptions become dangerous."

  I agree with the cpp-expert that this should follow RAII in the sequence and should note the modern preference for std::expected over exceptions.
  [/csharp-expert]

  [cpp-expert] The coverage document already has "Transaction" as the candidate for exception safety guarantees. The gap is that it's unassigned and has no
  sequencing. I'd place it after RAII (File Processor) — you need to understand destructors before you can understand the strong exception guarantee. However,
   I want to flag a tension with Principle 5 (Modern C++ First): modern C++ increasingly prefers std::expected and std::optional over exceptions for
  recoverable errors. The exception safety cpptude should teach the guarantees (basic/strong/nothrow) as essential knowledge for working with existing
  codebases, while noting that new code should prefer value-based error handling. [/cpp-expert]

  ---
  Missing Concepts

  Point 8 — Reference semantics and parameter passing (T, const T&, T&, T&&)

  [cpp-expert] This is a genuine, critical gap. I'd go further than the criticism: this is arguably the most important missing concept. Every function
  signature in C++ requires choosing between value, const-ref, ref, and rvalue-ref. C# developers have two modes: value types pass by value, reference types
  pass by reference (handle). The C++ model where any type can be passed in any of these four ways is deeply unfamiliar. This needs a Rating 5 concept in Tier
   1, not just a Rating 5 recommendation. A cpptude that forces the learner to choose the correct parameter passing for different scenarios (small types by
  value, large types by const-ref, output parameters by ref, sink parameters by value then move) would be high value. [/cpp-expert]

  [csharp-expert] This is the most important gap from the C# perspective, and I want to explain why it's so confusing.

  In C#, there are exactly two mental models:
  1. Value types (int, struct): passed by value. A copy is made. You'd use ref to pass by reference.
  2. Reference types (class, string, arrays): the reference is passed by value, but the object is shared. Mutation is visible to the caller.

  In C++, there are four modes — T, const T&, T&, T&& — and they apply to any type. A C# developer looking at a C++ function signature like:

  void process(const std::string& name, std::vector<int>& results, std::string title);

  ...will be confused about all three parameters. Why is name different from title? Both are strings. Why does results not have const? What's the & doing on
  name but not title?

  The C# mental model maps roughly to:
  - T → "I'm getting my own copy" (like value type passing)
  - const T& → no direct equivalent (closest is in parameter in C# 7.2, which most devs don't use)
  - T& → ref parameter
  - T&& → no equivalent at all

  This concept should be introduced early and revisited in every subsequent cpptude. Each function signature is a teaching moment. [/csharp-expert]

  Point 9 — Raw pointers as non-owning observers

  [cpp-expert] Agreed, with a nuance. The modern C++ guidance is: raw T* means non-owning observer, unique_ptr<T> means owning. This is the Herb Sutter / C++
  Core Guidelines convention. C# developers will either avoid all pointers (losing the ability to express non-owning relationships) or use them as owning
  pointers (reverting to manual new/delete). This concept should be introduced alongside unique_ptr in the Tree Builder cpptude rather than in a separate
  cpptude. When teaching ownership, you inherently need to contrast owning vs. non-owning. [/cpp-expert]

  [csharp-expert] C# developers will have one of two reactions to raw pointers:

  1. Fear: "Pointers are unsafe. I should use smart pointers for everything." This leads to shared_ptr overuse (every relationship becomes shared ownership)
  and the inability to express "I'm looking at this but don't own it."
  2. Familiarity: "Oh, this is like a C# reference — it points to an object." This leads to using new with raw pointers and manual delete, recreating the
  pre-GC C++ memory management that modern C++ has moved past.

  Both reactions are wrong. The correct C++ idiom — T* means "non-owning observer, may be null" and T& means "non-owning observer, never null" — has no direct
   C# equivalent because C# doesn't distinguish between owning and non-owning references. All C# references are non-owning from the developer's perspective
  (the GC owns everything).

  I agree with the cpp-expert that this should be taught alongside unique_ptr rather than in isolation. The lesson is: "In C++, you must always know who owns
  an object. unique_ptr = I own it. T* or T& = someone else owns it and I'm just looking." [/csharp-expert]


  Point 10 — Rule of Zero/Three/Five

  [cpp-expert] This is critical and genuinely missing. The Rule of Zero is the most important: if you use RAII types (string, vector, unique_ptr) as members,
  the compiler-generated special member functions are correct. The Rule of Five only applies when you write a custom resource-managing class. C# developers
  have no mental model for this because the GC handles it. However, I'd prioritize Rule of Zero over Rule of Five. Most C++ code should follow Rule of Zero. A
   cpptude that teaches "don't write destructors, copy constructors, or assignment operators — use RAII members and the compiler writes them for you" is more
  useful than one that teaches how to write all five. The Five should be an advanced follow-up or secondary KAP. [/cpp-expert]

  [csharp-expert] C# developers have no mental model for this because they've never needed one. In C#:

  - Copy is implicit for value types (and usually trivial — small structs)
  - Assignment for reference types is always aliasing, not copying
  - ICloneable exists but is rarely implemented
  - Destructors (~ClassName) exist but are discouraged — the GC handles cleanup
  - There is no "move" operation

  When a C# developer writes a C++ class, they won't think about any of the special member functions. They'll write:

  class Config {
      std::string name;
      std::vector<int> values;
  public:
      Config(std::string n, std::vector<int> v) : name(n), values(v) {}
      // C# dev stops here — "I wrote the constructor, I'm done"
  };

  The good news: for this example, Rule of Zero means the compiler-generated copy/move/destructor are all correct. The C# developer accidentally did the right
   thing. The lesson should start from this: "If all your members are RAII types, you don't need to write anything. The compiler does the right thing." Then
  introduce the cases where you need Rule of Five (custom resource management). [/csharp-expert]


  Point 11 — Virtual functions, vtables, object slicing

  [cpp-expert] Object slicing is the critical trap here, not vtables themselves. A C# developer writes Base b = derived; expecting polymorphic behavior; in
  C++ this slices. This deserves coverage but can be handled within a cpptude on value semantics (Data Pipeline) as a secondary KAP or as a standalone if
  polymorphism is in scope. I agree with the criticism's lower priority — if the curriculum focuses on modern value-semantic C++, deep polymorphism coverage
  can be deferred. [/cpp-expert]

  [csharp-expert] In C#, polymorphism is the primary abstraction mechanism. Almost every non-trivial C# codebase uses interfaces and virtual methods
  extensively. The C# developer's default class hierarchy looks like:

  interface IProcessor { void Process(Data data); }
  class FastProcessor : IProcessor { ... }
  class SafeProcessor : IProcessor { ... }
  // Usage: IProcessor p = GetProcessor(); p.Process(data);

  This always works in C# because IProcessor p holds a reference — no slicing, no copying. The equivalent C++ code with Processor p = GetProcessor(); slices.
  The C# developer won't see it coming because they've never had to think about it.

  I agree this can be deferred if the curriculum focuses on value semantics, but it must be covered before the learner tries to use inheritance in C++. A
  warning in an early cpptude — "if you're reaching for inheritance, stop and ask why" — would help. [/csharp-expert]


  Point 12 — Header/source separation and forward declarations

  [cpp-expert] This is a real friction point for C# developers but is build-system knowledge, not language semantics knowledge. It shouldn't have its own
  cpptude (violates Principle 2 — no real problem domain). Instead, it should be addressed through project structure: starting from cpptude #2 or #3, the code
   should be split across multiple translation units, and the lesson text should explain why. The build system (CMake) should be visible and explained. This
  is a "learn by doing" concept, not a "teach through a dedicated exercise" concept. [/cpp-expert]

  [csharp-expert] This will be a significant source of frustration for C# developers, but it's a tooling frustration, not a conceptual one. In C#:

  - You write using MyNamespace; and all types in that namespace are visible
  - The build system figures out compilation order
  - There are no forward declarations, header guards, or include order issues
  - You never see a linker error

  In C++, the first time a developer gets "undefined reference to MyClass::doThing()" because they forgot to link the .cpp file, or "redefinition of MyClass"
  because they forgot header guards, they'll be deeply confused. These errors don't exist in their mental model.

  I agree with the cpp-expert that this doesn't merit a standalone cpptude. But it needs explicit attention in the lesson text the first time multiple files
  appear. Something like: "In C#, the compiler sees your entire project. In C++, each .cpp file is compiled independently and knows only what's been
  #included. The linker connects them afterward." [/csharp-expert]

  Point 13 — noexcept and exception specifications

  [cpp-expert] Important for move constructors (a throwing move constructor disables std::vector optimization) but not worth a dedicated cpptude. Cover it in
  the move semantics cpptude as a secondary KAP: "Mark your move constructor noexcept or std::vector will copy instead of move." That's a single, memorable
  lesson point. [/cpp-expert]

  [csharp-expert] C# has no equivalent. There's no way to declare that a C# method won't throw, and no optimization that depends on it. C# developers won't
  understand why noexcept matters or why it affects performance (the std::vector move-vs-copy distinction). Agree this is best taught as a secondary point in
  the move semantics cpptude rather than standalone. [/csharp-expert]


  ---
  Recommendations

-  "Add missing Rating 5 concepts"

  [cpp-expert] Agreed. Parameter passing is the most critical addition. [/cpp-expert]

-  "Fix sequencing: Move semantics before Tree Builder"

  [cpp-expert] Agreed without reservation. This is a bug in the current plan. [/cpp-expert]

-  "Add Concept Prerequisites column"

  [cpp-expert] Good suggestion. The dependency graph is: stack allocation → RAII → parameter passing → dangling references → string_view → move semantics →
  unique_ptr. Making this explicit prevents sequencing errors. [/cpp-expert]

  [csharp-expert] Strong agree from the C# perspective. C# developers are used to a flatter learning curve — you can learn LINQ without understanding memory
  management, you can use async/await without understanding thread pools. C++ concepts are deeply layered, and prerequisites aren't obvious to someone coming
  from a language where most concepts are independent. An explicit dependency graph helps the learner understand why they need to learn things in this order.
  [/csharp-expert]
















  ---
  Criticism 2 — In-Line Review

  Alignment Assessment

  [cpp-expert] Agreed — the structural and philosophical alignment is strong. The dual structure works. [/cpp-expert]

  ---
  Gap 1 — No explicit "bait/guardrail/failure mode" design per cpptude (Critical)

  [cpp-expert] This is the most important criticism across all three reviews. The best-practices document (Principle 1 and Principle 3) already describes this
   pattern, but the concept-coverage document doesn't operationalize it per cpptude. The suggested structure — Trap Attempt → Failure → Correct Pattern →
  Verification Tooling — is exactly right and should be added to the coverage matrix as additional columns. For each cpptude, specify:

  - Bait: What does the C# developer naturally write? (e.g., new Sudoku() on the heap)
  - Failure signal: What makes it fail? (e.g., custom allocator that counts heap allocations + static_assert or test that asserts zero allocations)
  - Tool: What tooling surfaces the failure? (e.g., ASan, -Werror, benchmark threshold)

  Without this, each cpptude author has to independently design the trap mechanism, which leads to inconsistency. [/cpp-expert]


  [csharp-expert] This is the single most important criticism for the C# audience. Let me explain why.

  C# developers are competent programmers. They've written production code, shipped products, debugged complex systems. When they approach a C++ exercise,
  they will naturally apply their existing expertise. If the exercise doesn't have guardrails that make their C# approach fail visibly, they will write
  C#-style C++ that compiles and runs — and they'll think they've learned C++.

  The bait/guardrail/failure pattern is essential because the wrong approach often works:

  - Heap-allocating everything with new? Compiles, runs, produces correct output. Leaks memory, but the learner never sees it.
  - Returning a reference to a local? Often works because the stack memory hasn't been reused yet.
  - Ignoring const? Compiles fine without -Werror.

  Without explicit guardrails, the curriculum becomes "C++ syntax for C# developers" rather than "unlearning managed-runtime instincts." The whole thesis
  collapses. [/csharp-expert]


  ---
  Gap 2 — Tooling for "mechanical sympathy" is not part of coverage (High)

  [cpp-expert] Strongly agree. The best-practices document specifies -fsanitize=address,undefined in Principle 7 (Compilation Must Be Reproducible), but the
  concept-coverage document mentions sanitizers exactly once as "(sanitizer exercises)." The tooling should be first-class:

  - Every cpptude should build with ASan+UBSan in debug mode. This is non-negotiable.
  - Specific cpptudes should use specific tools as their "aha" mechanism:
    - Dangling references → ASan catches use-after-free
    - Signed integer overflow → UBSan catches at runtime
    - Cache locality → perf stat or simple std::chrono benchmarks showing 10-100x difference
    - Move vs copy → Compiler Explorer showing generated assembly

  The "Tier 0: Tooling" framing is a good idea. I'd implement it as a short cpptude #0 that sets up the build environment and demonstrates what ASan/UBSan
  catch. [/cpp-expert]

  [csharp-expert] C# developers are not accustomed to thinking about what the machine is doing. The CLR abstracts the machine:

  - JIT compilation is invisible
  - GC pauses are (usually) invisible
  - Memory layout is the runtime's decision
  - Cache locality is not a concept developers interact with

  The tooling gap matters because C# developers need evidence to override their instincts. Telling them "heap allocation is expensive" is unconvincing — in
  C#, new is fast (bump allocator on Gen0) and they've allocated millions of objects without visible cost. You need to show them, with benchmarks or profiler
  output, that C++ heap allocation has different characteristics.

  ASan and UBSan are particularly important because they replace the runtime safety net that C# developers rely on (exceptions for null deref, bounds checking
   for arrays). Without sanitizers, the C# developer loses their safety net and doesn't know it. [/csharp-expert]



  ---
  Gap 3 — Missing classic C#→C++ instinct collisions (High)

  3.1 — Rule of 0/3/5

  [cpp-expert] Covered in my response to Criticism 1, Point 10. Agree it's P1, but emphasize Rule of Zero over Rule of Five. [/cpp-expert]

  [csharp-expert] Covered above in my response to Criticism 1, Point 10. The key C# angle: C# developers have never designed a type's copy/move/destroy
  behavior because the runtime handles it. [/csharp-expert]


  3.2 — Pass-by-value vs const-ref vs ref lifetime

  [cpp-expert] Covered in my response to Criticism 1, Point 8. This is the single biggest gap. Promote to P0. [/cpp-expert]

  [csharp-expert] Covered above in my response to Criticism 1, Point 8. This is the number one source of confusion. [/csharp-expert]


  3.3 — Polymorphism pitfalls: object slicing, virtual destructors

  [cpp-expert] Object slicing is P1 for C# developers because they use inheritance heavily. Virtual destructors are important but follow naturally from
  understanding ownership. One specific note: the override keyword in C++11 and later catches many vtable bugs at compile time, so C++ is less dangerous here
  than it used to be. Slicing is the main trap. [/cpp-expert]

  [csharp-expert] Covered above in Point 11. The critical point: C# developers use inheritance and interfaces heavily, and every instinct they have about
  polymorphism will cause problems in C++ (slicing, missing virtual destructors, vtable cost). [/csharp-expert]




  3.4 — Initialization sharp edges: narrowing, brace-init, most vexing parse

  [cpp-expert] I'd lower this to P3 for this curriculum. The most vexing parse is increasingly rare with brace initialization. Narrowing conversions are
  caught by -Wconversion (which should be in the standard compiler flags). These are real friction points but not where C# developers write dangerous code —
  they write annoying code. Cover in lesson text sidebars, not dedicated cpptudes. [/cpp-expert]

  Note about staying in modern value + RAII C++:

  [cpp-expert] This is the right call for the early curriculum. Modern C++ strongly favors value semantics, RAII, and composition over inheritance. Teaching
  polymorphism-heavy OOP patterns would actually be teaching a style that modern C++ is moving away from. Cover slicing as a trap to avoid, not as a pattern
  to use. [/cpp-expert]

  [csharp-expert] This does matter for C# developers, more than the cpp-expert suggested. In C#, initialization is straightforward:

  var x = new MyClass(42);      // Always works
  var y = new MyClass { Value = 42 }; // Object initializer
  int[] arr = { 1, 2, 3 };      // Array initializer

  In C++, the learner will encounter:
  - MyClass x(42) vs MyClass x{42} vs MyClass x = 42 — all different
  - Most vexing parse: MyClass x() declares a function, not an object
  - Narrowing: int x{3.14} is an error with braces but not with parens
  - Aggregate initialization rules that differ from C# object initializers

  These aren't dangerous (no UB), but they cause a high volume of "why doesn't this compile?" moments. I'd rate this P2 for frustration reduction — not worth
  a cpptude, but the lesson text should address brace initialization early and establish a consistent style. [/csharp-expert]


  ---
  Gap 4 — Concurrency / memory model is absent

  [cpp-expert] Should be explicitly listed as a non-goal for the initial curriculum, with a planned Tier 2 or Tier 3 extension. Reasoning: concurrency in C++
  requires understanding the memory model (std::memory_order), which is genuinely hard and requires solid foundation in everything else first. Data races in
  C++ are UB (unlike C# where they're "merely" non-deterministic), so teaching concurrency before lifetime and ownership are solid would be irresponsible. Add
   a note to the coverage doc: "Concurrency (threads, atomics, memory ordering) is deferred to a future extension. Prerequisite: all Tier 1 and Tier 2
  concepts." [/cpp-expert]

  [csharp-expert] C# developers have extensive experience with concurrency — async/await, Task, Parallel.ForEach, lock, ConcurrentDictionary. But their mental
   model is built on the CLR's memory model, which is stronger than C++:

  - C#'s volatile provides acquire/release semantics (C++ volatile does not)
  - lock maps to Monitor.Enter/Exit with full memory barriers
  - async/await is a language-level abstraction over state machines
  - Interlocked methods have clear, documented behavior

  In C++, data races are UB (not just "wrong behavior" — undefined behavior). A C# developer who writes lock-free code using C++ volatile instead of
  std::atomic will produce code that compiles, appears to work in testing, and fails catastrophically under load.

  I agree with deferring this, but the deferral must be explicit: "Do not write multithreaded C++ code until you've completed the concurrency module." C#
  developers will be tempted to reach for std::thread early because they're comfortable with concurrency — but their C# concurrency instincts are actively
  dangerous in C++. [/csharp-expert]

  ---
  Gap 5 — Tier 4/5 breadth dilutes the C# transition theme

  [cpp-expert] The Tier 4/5 concepts (SFINAE, allocators, placement new, volatile) are correctly placed as low priority and should stay. They don't dilute the
   curriculum because no one will reach them until they've completed Tiers 1-3. However, the suggestion to add a "C# Trap Severity" axis has merit. Some Tier
  3 concepts have high C# trap severity (e.g., string_view lifetime) while some Tier 1 concepts have lower C# trap severity (e.g., signed integer overflow —
  C# developers won't naturally trigger this). A secondary sort within each tier by C# trap severity would help prioritize cpptude development order.
  [/cpp-expert]

  [csharp-expert] The "C# Trap Severity" axis suggestion is valuable. Some concepts are critical C++ knowledge but not a C# trap (e.g., SFINAE — a C#
  developer won't naturally try SFINAE-style patterns because C# generics don't work that way). Other concepts are moderate C++ knowledge but a major C# trap
  (e.g., string_view lifetime — C# developers slice strings freely because string is immutable and GC-managed).

  From the C# perspective, the priority ordering should be:

  1. High C++ importance + High C# trap: Parameter passing, dangling references, move semantics → Must be dedicated cpptudes
  2. High C++ importance + Low C# trap: constexpr, static_assert → Important but can be integrated
  3. Low C++ importance + High C# trap: Object slicing, string_view lifetime → Need explicit attention despite lower C++ tier
  4. Low C++ importance + Low C# trap: SFINAE, allocators, placement new → Can be deferred indefinitely

  Adding this axis would clarify development priority. [/csharp-expert]
  
  
  ---
  Criticism 3 — In-Line Review

  Executive Summary

  [cpp-expert] Agree with the overall assessment. The compilation model gap is a real concern, though I disagree on the proposed solution (see below).
  [/cpp-expert]

  ---
  Alignment Analysis

  [cpp-expert] The table is correct. All four README goals map cleanly to coverage plan items. No notes. [/cpp-expert]

  ---
  Gap A — The Compilation Model (Linker, ODR, Headers)

  [cpp-expert] This is a real friction point, but I disagree with Rating 4/5 criticality. Here's why:

  1. Modern C++ is reducing this friction. C++20 modules (import std;) are intended to replace the header/include model. Teaching the header model in detail
  is teaching a legacy pattern. The curriculum should mention it, but not build a cpptude around it.
  2. Build systems abstract most of this. A well-configured CMake project handles translation units and linking. The learner needs to understand that code is
  split across files and why they get linker errors, but they don't need deep ODR expertise.
  3. It's not a C++ concept — it's a C++ toolchain concern. It doesn't fit the "one primary KAP" model because there's no algorithm or data structure to
  solve.

  My recommendation: address this through project structure starting in cpptude #2. Split the code across .hpp/.cpp files and explain the build in the lesson
  text. Don't create a dedicated cpptude for it. If C++20 modules are available on the target compilers, use modules instead and note the header model as
  historical context.

  One caveat: if the target compilers don't yet support modules reliably (GCC modules are still experimental in some configurations as of 2025), then a brief
  "Compilation Model" sidebar in an early cpptude is warranted. [/cpp-expert]

  [csharp-expert] This is the right call for the C# audience. The C# build model is radically simpler:

  - One project file (.csproj) defines what's included
  - using directives make namespaces available
  - No header files, no forward declarations, no include guards
  - The compiler sees the entire project simultaneously
  - Linker errors effectively don't exist

  The first time a C# developer encounters #include, they'll treat it like using — but it's textual inclusion, not symbol import. The first time they get a
  linker error, they'll have no framework for understanding it.

  That said, I agree with the cpp-expert that this is better taught through project structure than a dedicated cpptude. What matters for the C# developer is a
   clear mental model: "Each .cpp file is compiled alone. #include copy-pastes the header content. The linker connects the pieces." A one-paragraph
  explanation with a diagram in an early cpptude's lesson text is sufficient. A dedicated cpptude on the compilation model would bore the learner — there's no
   interesting problem to solve. [/csharp-expert]



  ---
  Gap B — Object Slicing

  [cpp-expert] Agree this is a dangerous C# trap, but I disagree with Rating 5/5. Object slicing only occurs when you use inheritance with value semantics —
  i.e., pass a Derived by value where a Base is expected. In modern C++, the guidance is:

  - Use inheritance sparingly (prefer composition)
  - When using polymorphism, use Base& or unique_ptr<Base>, not Base by value
  - The compiler does not warn about slicing (this is the real problem)

  Rating 4 is more appropriate. It should be covered as a secondary KAP in the copy semantics cpptude (Data Pipeline) or in a polymorphism cpptude if one is
  added. The fix is simple to teach: "Never pass polymorphic types by value." Making it a standalone cpptude is overkill. [/cpp-expert]

  [csharp-expert] From the C# perspective, this is more dangerous than the cpp-expert suggested. Here's why: C# developers use polymorphism as their primary
  tool for abstraction. In a typical C# codebase, you'll find:

  - Interface-based dependency injection everywhere
  - virtual methods in base classes overridden by subclasses
  - Collections of base-type references (List<IProcessor>)
  - Method parameters typed as interfaces or base classes

  Every one of these patterns involves assigning a derived type to a base-type variable. In C#, this is always safe. In C++, if done by value, it slices.

  The danger isn't that an experienced C++ developer would make this mistake — it's that an experienced C# developer will reach for inheritance-based patterns
   as their first instinct and get sliced without warning. I'd rate this higher than the cpp-expert did for this specific audience — not necessarily Tier 1,
  but the first cpptude that touches any form of type hierarchy must address it explicitly.

  Where I agree with the cpp-expert: this can be a secondary KAP in the copy semantics (Data Pipeline) cpptude rather than a standalone. The lesson is: "In
  C++, prefer composition over inheritance. If you must use inheritance, always use references or pointers to base, never values." [/csharp-expert]


  ---
  Gap C — Implicit Conversions & explicit

  [cpp-expert] Agree with Rating 3/5. The explicit keyword should be mentioned in any cpptude that defines a single-argument constructor. It's a one-line fix
  (explicit before the constructor) and a one-paragraph explanation. Not worth a cpptude, but should appear in the coding style expectations from the start.
  Add it to the best-practices document as a rule: "All single-argument constructors must be explicit unless implicit conversion is intentionally desired."
  [/cpp-expert]

  [csharp-expert] C# is more strict about implicit conversions than C++, which makes this an interesting case. In C#:

  - Implicit conversions exist but must be explicitly defined (implicit operator)
  - Most type conversions require explicit casts
  - The compiler is generally protective against surprising conversions

  In C++, a single-argument constructor is an implicit conversion unless marked explicit. A C# developer won't expect this because C# doesn't have this
  behavior — you'd need to explicitly define implicit operator to get it.

  However, the surprise is mild compared to lifetime and ownership issues. I agree with Rating 3 — mention explicit in passing and add it to coding standards,
   but don't dedicate a cpptude to it. [/csharp-expert]


  ---
  Recommendations

  "Add Object Slicing to Tier 1"

  [cpp-expert] Disagree with Tier 1 placement. Slicing is a real trap but it's a specific intersection of value semantics + inheritance. It belongs in Tier 2
  alongside polymorphism concepts. Tier 1 should remain focused on concepts that cause UB or resource leaks — slicing causes neither. It causes logic bugs,
  which are serious but not "dangerous" in the Tier 1 sense. [/cpp-expert]

  [csharp-expert] I'd place it at the boundary of Tier 1/Tier 2 specifically because of the C# audience. For a general C++ curriculum it's Tier 2 (the
  cpp-expert is right that modern C++ minimizes inheritance). For a C#-targeted curriculum, it's closer to Tier 1 because C# developers will actively seek out
   inheritance patterns. The C# Trap Severity is 5 even if the C++ Concept Importance is 3-4. [/csharp-expert]


  "Add Translation Units & Linking to Tier 2"

  [cpp-expert] Disagree with a dedicated tier placement. Address through project structure, not a dedicated cpptude. See Gap A response above. [/cpp-expert]

  [csharp-expert] Agree with addressing it through project structure rather than a dedicated tier entry. But I'd go further: the first cpptude that uses
  multiple files needs a "Build Model" sidebar section that explains the C++ compilation model in contrast to C#'s project model. Make it a reusable
  explanation that later cpptudes can reference. [/csharp-expert]
  


  ---
  [cpp-expert]
  Summary of Actionable Items (prioritized)

  From the cpp-expert perspective, here are the changes to the coverage document that the three criticisms collectively support, in priority order:

  1. Add parameter passing (T, const T&, T&, T&&) as a Rating 5 concept in Tier 1. All three criticisms identified this gap.
  2. Fix move semantics sequencing. Insert before Tree Builder. All three criticisms flagged this.
  3. Add per-cpptude trap/guardrail/failure columns to the coverage matrix. Criticism 2 nailed this as the structural gap.
  4. Add Rule of Zero as a Tier 2 concept. Two of three criticisms flagged it.
  5. Add a Tooling parallel track specifying which sanitizer/tool surfaces each cpptude's lesson. Criticism 2.
  6. Assign the dangling references cpptude (String Parser) to position #4 in the sequence.
  7. Deduplicate iterator invalidation in the coverage matrix.
  8. Explicitly defer concurrency as a documented non-goal for the initial curriculum.
  9. Add object slicing as a Tier 2 concept (secondary KAP in Data Pipeline or standalone).
  [/cpp-expert]

  [csharp-expert]
  Summary — C# Perspective Priority List

  From the C# expert's perspective, the criticisms collectively identify these priorities for maximizing impact on the target audience:

  1. Parameter passing modes — The #1 confusion source. C# has two modes; C++ has four. Every function signature is a puzzle.
  2. Per-cpptude trap/guardrail/failure design — Without this, C# developers will write C#-style C++ that appears to work. The whole curriculum depends on
  this.
  3. Lambda capture lifetime — C# closures are GC-safe. C++ captures are not. The bug is invisible without ASan.
  4. Move semantics sequencing — Must precede unique_ptr. C# has no equivalent concept.
  5. Dangling references — C# developers have never experienced an invalid reference. They have no instinct for this.
  6. Tooling track (ASan/UBSan) — Replaces the CLR safety net that C# developers unconsciously rely on.
  7. Object slicing warning — C# developers will reach for inheritance. They need an early, explicit warning.
  8. Rule of Zero — Reassures C# developers that they don't need to learn Rule of Five for most types.
  9. Explicit deferral of concurrency — C# developers are confident in their concurrency skills, making C++ concurrency more dangerous for them.
  [/csharp-expert]

