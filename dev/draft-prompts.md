### Prompt 1: The Knowledge Base Generator

**Goal:** Generate the "C# to C++ Transition Guide" with weighted priorities.

> **Role:** You are a Principal Systems Engineer and Technical Educator specializing in cross-training senior developers from Managed Runtimes (.NET/CLR) to Native Systems (C++ ISO Standards 20/23). You have deep knowledge of compiler theory, memory models, and the "Uncanny Valley" of syntax between C# and C++.
> **Task:** Conduct a deep reasoning and research exercise to generate a **"C# to C++ Transition Guide."** This document will serve as the "Ground Truth" for a series of coding exercises.
> **Instructions:**
> 1. **Search & Analyze:** Investigate specific friction points where C# intuition actively sabotages C++ performance or safety. Focus on:
>    * **Memory Model:** GC vs. RAII, Heap vs. Stack, Reference Types vs. Value Types.
>    * **Data Structures:** `List<T>` vs. `std::vector` (growth policies), `Class` vs. `Struct` defaults, VTables/Virtual overhead.
>    * **Idioms:** LINQ vs. Ranges, Exceptions vs. Error Codes/`std::expected`, Properties vs. Accessors.
> 2. **Categorize & Rate:** Organize these findings into categories (e.g., "Memory," "Type System," "Performance"). For each item, assign a **Criticality Rating**:
>    * **5 - Critical:** (e.g., Stack semantics, RAII). The developer is dangerous without this.
>    * **4 - Required:** (e.g., Move semantics, `std::unique_ptr`). Essential for modern C++.
>    * **3 - Important:** (e.g., Templates, `std::string_view`).
>    * **2 - Good-to-Know:** (e.g., `constexpr`, SFINAE).
>    * **1 - Seasoning:** (e.g., Attributes, specialized casts).
> 3. **Define the Axiom:** For each "C# Danger," explicitly state the "C++ Axiom" that corrects it.
>
> **Output Format:**
> A structured markdown document titled **"The C# to C++ Transition Guide"**.
> * **Category**
>   * **Concept Name** (Criticality Rating)
>     * **The C# Instinct:** Describe the managed habit.
>     * **The C++ Reality:** Explain the mechanical difference (stack, heap, cpu).
>     * **The C++ Axiom:** The rule to follow.

---

### Prompt 2: The Pytude Deconstructor

**Goal:** Select a Pytude and map it to the Transition Guide to create a Lesson Spec.

> **Role:** You are a Curriculum Designer for Senior Software Engineers. You are using the **"C# to C++ Transition Guide"** (from Prompt 1) to design a coding lesson based on Peter Norvig's "Pytudes."
>
> **Input:**
> 1. **Pytude URL/Topic:** [Insert Pytude Name, e.g., "Sudoku" or "Spell Corrector"]
> 2. **Transition Guide:** [Paste or Reference the Output of Prompt 1]
>
> **Task:** Deconstruct the chosen Pytude and create a **"Cpptude Lesson Specification"** that maximizes the transfer of C++ knowledge.
>
> **Instructions:**
> 1. **Deconstruct the Spec:** Extract the core problem "Spec" from the Python code. (What is the input? What is the output? What is the constraint?). Ignore Python implementation details.
> 2. **Map to Friction Points:** Look at the "Transition Guide" and identify which **Critical (5)** or **Required (4)** concepts this specific problem can teach.
>    * *Example:* Does this problem involve heavy string manipulation? -> Opportunity to teach `std::string_view` (Rating 3) vs C# String interning.
>    * *Example:* Does this problem require a grid? -> Opportunity to teach Flat Memory/Cache Locality (Rating 5) vs C# Multi-dimensional Arrays.
> 3. **Determine Primary KAP (Knowledge Acquisition Point):** Select the *single* most important C++ concept this lesson will enforce.
> 4. **Override/Augment:** Decide if the C++ lesson should diverge from the Python logic to enforce a C++ Axiom (e.g., "The Python version uses a recursive search. The C++ version will use an iterative bitmask approach to teach Stack safety and Register pressure").
>
>    **⚠️ IMPORTANT:** This prompt cannot autonomously determine when to diverge from the original Pytude. An expert must provide explicit decision heuristics for each lesson, such as:
>    * When does pedagogical value outweigh fidelity to the original algorithm?
>    * Which C++ idioms are worth forcing even if they change the solution structure?
>    * What performance/safety tradeoffs justify a complete reimplementation?
>
>    If no expert guidance is provided, flag this gap clearly in the Lesson Specification under a **"Requires Expert Input"** section. The resulting Cpptude must also highlight any assumptions made in the absence of this guidance.
>
> **Output Format:**
> A "Lesson Specification" containing:
> * **Lesson Title:** (e.g., "Sudoku: The Stack & Bitwise Operations")
> * **The Spec:** (Language-agnostic problem definition)
> * **The C# Trap:** The specific habit we are baiting the user to use (and how we will block it).
> * **Targeted C++ Axioms:** The specific Critical/Required concepts from the Guide.
> * **Implementation Strategy:** A high-level directive on how the C++ code should be structured (e.g., "Use `std::array`, avoid all heap allocations").
> * **Requires Expert Input:** (If applicable) List any Override/Augment decisions that need human review before proceeding to Prompt 3.

---

### Prompt 3: The Cpptude Generator

**Goal:** Write the actual Code and Analysis based on the Spec.

> **Role:** You are an Expert C++ Systems Programmer (C++20/23) and Author.
> **Input:**
> 1. **Lesson Specification:** (Output from Prompt 2)
> 2. **Transition Guide:** (Output from Prompt 1)
> 3. **Pytude Reference:** (Optional context)
>
> **Task:** Write the complete **"Cpptude"** lesson. This includes the educational preamble, the high-performance C++ code, and the post-mortem analysis for a C# developer.
>
> **Instructions:**
> 1. **Grounding Search:** Verify any specific algorithms or mathematical constants required by the Spec via authoritative sources (e.g., "Sudoku backtracking optimization," "Norvig spell correct probability logic").
> 2. **Outline the Cpptude:** Briefly sketch the types and function signatures to ensure they align with the "Targeted C++ Axioms" (e.g., ensure `const` correctness, `noexcept`, and proper types).
> 3. **Write the Code:** Generate the full, compilable C++ solution.
>    * **Strict Adherence:** If the Lesson Spec says "No Heap," use `std::array` or stack variables. If it says "Modern," use `std::ranges`.
>    * **Optimization:** The code should behave like "Systems Code" (performant, safe), not "Application Code."
> 4. **The "For the C# Developer" Commentary:** Write a detailed breakdown explaining the code *specifically* to a C# dev.
>    * *Contrast:* "In C#, you would do X. Here, we did Y because [Deep Reason]."
>    * *Reference:* Link back to the Criticality Ratings from the Transition Guide.
> 5. **Flag Unresolved Decisions:** If the Lesson Specification contained a "Requires Expert Input" section, explicitly note in the Cpptude which design choices were made without expert guidance and why. Present these as **"Design Decision (Assumed)"** callouts so a reviewer can validate or override them.
>
> **Output Format:**
> * **Title & Difficulty**
> * **The C# Instinct vs. C++ Reality:** (Brief Hook)
> * **The Code:** (C++20/23, heavily commented with "Why", not "What")
> * **Deep Dive Analysis:** (Memory, Cache, Safety)
> * **Design Decisions (Assumed):** (If applicable) List any Override/Augment choices made without explicit expert input.
> * **Next Step:** A challenge to extend the code.
