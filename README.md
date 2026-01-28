# cpptudes

"An ***etude*** (a French word meaning *study*) is an instrumental musical composition, usually short, of considerable difficulty,
and designed to provide practice material for perfecting a particular musical skill." — [*Wikipedia*](https://en.wikipedia.org/wiki/%C3%89tude)

This project contains ***cpptudes*** — C++ etudes for developers moving from C#.

# Who is this for?

If you're a C# developer who wants to learn C++, there are plenty of tutorials that will walk you through syntax. But syntax is the easy part. The hard part is **unlearning managed-runtime instincts** — reaching for `new` when the stack will do, trusting the GC to clean up objects whose lifetime you should control, treating all copies as aliases.

Cpptudes are deliberate practice for that transition. Each one teaches a specific C++ concept through a real problem, with guardrails that make managed-world habits fail visibly. You'll write C++ code, see why the C# approach doesn't translate, and build mechanical sympathy for what the machine is actually doing.

# Index of Cpptudes

Each cpptude has a lesson README with a concept walkthrough, the C++ source files, and a challenge exercise. Cpptudes are numbered in recommended order — each builds on concepts from earlier exercises.

| # | Cpptude | Primary C++ Concept | C# Trap | Rating |
|---|---------|---------------------|---------|--------|
| [001](cpptudes/001-sudoku/README.md) | [Sudoku](cpptudes/001-sudoku/) | Stack allocation, value semantics | Heap-allocating everything with `new` | 5 - Critical |

See [dev/concept-coverage.md](dev/concept-coverage.md) for the full coverage plan and upcoming cpptudes.

# Index of C++ Files

| File | Description |
|------|-------------|
| [sudoku.hpp](cpptudes/001-sudoku/sudoku.hpp) | *Sudoku solver header — Board type, constexpr lookup tables, solver interface* |
| [sudoku.cpp](cpptudes/001-sudoku/sudoku.cpp) | *Sudoku solver implementation — constraint propagation and backtracking search* |
| [main.cpp](cpptudes/001-sudoku/main.cpp) | *Entry point — parses and solves puzzles from stdin* |

# Etudes for Programmers

The name *cpptudes* follows Peter Norvig's [*pytudes*](https://github.com/norvig/pytudes). Norvig got the idea for the "etudes" part of the name from
a [1978 book](https://books.google.com/books/about/Etudes_for_programmers.html?id=u89WAAAAMAAJ)
by [Charles Wetherell](http://demin.ws/blog/english/2012/08/25/interview-with-charles-wetherell/).
