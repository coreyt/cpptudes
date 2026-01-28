# Cpptude Authoring Workflow

This document defines the agent-based workflow for producing cpptude lessons. It replaces the earlier monolithic prompt chain with a pipeline of four specialized agents, each responsible for a distinct aspect of lesson quality.

## Agents

| Agent | Responsibility | Requirement |
|-------|---------------|-------------|
| **curriculum-designer** | Lesson spec, sequencing, pedagogical structure | Both A and B |
| **cpp-expert** | C++ correctness, idiomaticity, performance, safety | A |
| **csharp-expert** | C# trap identification, bridge accuracy, learner prediction | B |
| **lesson-writer** | Prose, structure, synthesis of all inputs into final lesson | Both A and B |

## Reference Documents

All agents should read these before starting work on a cpptude:

- `dev/best-practices.md` — Design principles and quality checklist
- `dev/concept-coverage.md` — Concept coverage plan with ratings and trap mechanics
- `dev/curriculum-map.md` — Scope and sequence, session structure, prerequisites

---

## Pipeline

Each cpptude is produced through a six-stage pipeline. Stages 1-3 can be run in parallel. Stage 4 depends on 1-3. Stages 5-6 depend on 4.

### Stage 1: Lesson Specification (curriculum-designer)

**Input:** The cpptude's entry in the curriculum map (week number, primary KAP, secondary concepts, prerequisites, C# trap).

**Task:** Produce a lesson specification containing:

- Lesson title and problem description (language-agnostic)
- Primary KAP with rating and justification
- Secondary concepts that naturally arise
- The C++ concept (Requirement A): what the lesson teaches and why it matters
- The C# bridge (Requirement B): which habit is targeted and how the lesson makes it fail visibly
- Bait: what the C# developer will naturally try
- Failure signal: what makes the wrong approach fail (compile error, ASan, benchmark)
- Verification tool: what tooling surfaces the failure
- Implementation constraints (e.g., "zero heap allocations")
- Prerequisites: which earlier cpptudes the learner must have completed
- Session structure: how the 2-3 hour block is divided (warm-up, trap encounter, concept intro, guided practice, independent practice)
- Homework: lightweight reinforcement task

**Output:** A lesson spec document (markdown) saved to the cpptude's directory.

### Stage 2: C++ Technical Design (cpp-expert)

**Input:** The lesson specification from Stage 1 (or the curriculum map entry if running in parallel).

**Task:** Produce a technical design containing:

- Data structures and type choices with justification
- Function signatures with parameter passing modes explained
- Compiler flags and build configuration
- Expected code structure (modules, functions, types)
- Performance considerations and what the compiler generates
- Sanitizer expectations: what ASan/UBSan should and should not flag
- Review of the implementation constraints: are they achievable and pedagogically sound?
- Any "Requires Expert Input" flags for design decisions that need human review

**Output:** A technical design document (markdown) or annotations on the lesson spec.

### Stage 3: C# Trap Design (csharp-expert)

**Input:** The lesson specification from Stage 1 (or the curriculum map entry if running in parallel).

**Task:** Produce a trap design containing:

- The specific C# code a developer would naturally write for this problem
- Why that approach is natural and reasonable in C# (present it fairly, not as a mistake)
- Where the C++ behavior will surprise the learner
- Predicted confusion points (ranked by likelihood)
- C# bridge talking points: what the lesson text should say to connect C++ concepts to C# experience
- Verification: is the bait realistic? Would an experienced C# developer actually fall for it?

**Output:** A trap design document (markdown) or annotations on the lesson spec.

### Stage 4: Lesson Writing (lesson-writer)

**Input:** All three outputs from Stages 1-3.

**Task:** Synthesize all inputs into the complete cpptude lesson:

- README.md following the standard lesson structure
- C++ source code (`.cpp`/`.hpp` files) with "why" comments
- Build instructions (CMakeLists.txt or compiler commands)
- All sections: Overview, Building & Running, The Code Explained (with C++ Concept and C# Bridge subsections), Deep Dive, Key Takeaways, Next Step Challenge
- Flag any "Design Decisions (Assumed)" where expert input was not available

The lesson must read as a single cohesive document, not as three separate voices stitched together.

**Output:** The complete cpptude directory (`cpptudes/NNN-name/`).

### Stage 5: Technical Review (cpp-expert)

**Input:** The complete cpptude from Stage 4.

**Task:** Review for Requirement A compliance:

- Code compiles with specified flags on target compilers (GCC 12+, Clang 15+)
- No undefined behavior (verify with ASan/UBSan mental model)
- `const` correctness throughout
- `explicit` on single-argument constructors
- Brace initialization used consistently
- `noexcept` where appropriate
- `[[nodiscard]]` where ignoring return values is a bug
- No unnecessary heap allocation
- Performance claims are accurate and grounded in reasoning
- Code is idiomatic modern C++20/23

**Output:** Review with specific line-level feedback. Pass/fail verdict.

### Stage 6: Bridge Review (csharp-expert)

**Input:** The complete cpptude from Stage 4.

**Task:** Review for Requirement B compliance:

- C# Bridge sections use correct C# terminology
- C# patterns are presented as reasonable, not as mistakes
- The bait targets a real C# instinct (not a strawman)
- The trap would actually catch an experienced C# developer
- C# code examples (if included) are idiomatic C#
- The bridge connects to the reader's existing knowledge without condescending

**Output:** Review with specific section-level feedback. Pass/fail verdict.

---

## Parallel Execution

Stages 1-3 can run in parallel when the curriculum map provides sufficient detail. The curriculum-designer produces the lesson spec, while the cpp-expert and csharp-expert work from the curriculum map entry directly. If running in parallel, all three outputs are reconciled before Stage 4.

Stages 5-6 can also run in parallel (both review the same artifact independently).

```
Stage 1 (curriculum-designer) --\
Stage 2 (cpp-expert)          ----> Stage 4 (lesson-writer) ---> Stage 5 (cpp-expert)
Stage 3 (csharp-expert)       --/                            \-> Stage 6 (csharp-expert)
```

---

## Iteration

If Stage 5 or 6 returns a fail verdict:

1. The reviewing agent provides specific, actionable feedback
2. The lesson-writer revises the affected sections
3. Only the failing review stage re-runs (not the full pipeline)

If both reviews pass, the cpptude is complete and ready for the quality checklist in `dev/best-practices.md`.

---

## Running the Workflow

To produce a cpptude, invoke the pipeline with the cpptude's week number or name:

1. Launch curriculum-designer, cpp-expert, and csharp-expert agents in parallel with the cpptude's curriculum map entry
2. Collect all three outputs
3. Launch lesson-writer with the combined inputs
4. Launch cpp-expert and csharp-expert review agents in parallel on the draft
5. If reviews pass, the cpptude is done
6. If reviews fail, iterate with lesson-writer on the feedback

Each agent reads the project context files (`best-practices.md`, `concept-coverage.md`, `curriculum-map.md`) at invocation time, so they always work from the current state of the curriculum.
