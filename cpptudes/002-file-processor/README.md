# Cpptude #002: File Processor — RAII & Deterministic Cleanup

| Field | Value |
|-------|-------|
| **Primary KAP** | RAII / Resource Acquisition Is Initialization (Rating 5 - Critical) |
| **Secondary Concepts** | Exception safety, destructor guarantees, multi-file structure, `const` member functions |
| **Difficulty** | Intermediate |
| **Prerequisite** | Cpptude #001 (Sudoku — Stack Allocation) |

---

## Overview

This cpptude teaches **RAII (Resource Acquisition Is Initialization)** — the C++ principle that every resource acquisition is tied to an object's constructor, and every resource release is tied to its destructor. Destructors run unconditionally at scope exit: normal return, early return, or exception propagation. There is no code path where cleanup is skipped.

The log file processor is the vehicle: read a server log, count occurrences of each severity level, collect error and warning messages, and write a summary report. The problem naturally requires opening and closing files — and the critical question is: what happens when something goes wrong between open and close?

> **C# Bridge:** If you're coming from C#, this cpptude targets the instinct that resource cleanup is a "nice to have" backed by the GC. In C#, if you forget to call `Dispose()` on a `FileStream`, the finalizer eventually handles it. In C++, there is no GC, no finalizer, no safety net. If you forget to close a file and an exception propagates, the handle leaks — permanently. RAII eliminates this entire class of bugs by making cleanup automatic and unconditional.

### The C# Instinct

When a C# developer needs to safely process a file, they reach for `try`/`finally` or `using`:

```csharp
// What a C# developer naturally writes
var file = File.OpenRead(path);
try {
    // process...
} finally {
    file?.Dispose();
}

// Or more idiomatically with 'using' (C# 8+ declaration form):
using var file = File.OpenRead(path);
// Dispose() called at end of scope
```

This works well in C# — the `using` statement provides deterministic cleanup. But notice: you must *opt in* to deterministic cleanup by writing `using`. If you forget, the GC and finalizer provide a safety net (eventually). In C++, there is no safety net. RAII makes deterministic cleanup the default, not an opt-in pattern.

> **Modern C# Note:** C# 8 introduced `using` declarations (without braces), which bring C# closer to RAII — the disposal happens at the end of the enclosing scope. This mirrors C++ destructor behavior. The existence of this feature shows that C++ patterns are valuable enough that C# is adopting them.

---

## Building & Running

This is the first cpptude with **multiple source files**. You must compile both `.cpp` files and link them together.

### Basic Build

```bash
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic file_processor.cpp main.cpp -o file_processor
./file_processor
```

### Build with Sanitizers (Recommended for Learning)

```bash
g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -Wall -Wextra file_processor.cpp main.cpp -o file_processor
./file_processor
```

### Build with CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/file_processor
```

### Process a Specific Log File

```bash
./file_processor input.log report.txt
```

The sanitizers will catch:
- Resource leaks (ASan leak detection)
- Undefined behavior (UBSan)

Watch `stderr` for `[RAII]` messages that show exactly when each file is opened and closed.

---

## Build Model Sidebar: How C++ Compilation Actually Works

This is the first cpptude where you have multiple `.cpp` files. The C++ build model is fundamentally different from C#'s project model, and understanding it prevents a category of errors that have no C# equivalent.

### C# Model: The Compiler Sees Everything

In C#, the compiler processes your entire project at once. Every `.cs` file is visible to every other `.cs` file. If `ClassA` in `FileA.cs` references `ClassB` in `FileB.cs`, the compiler resolves it automatically. You never think about file boundaries.

```
C# Build:
  [FileA.cs] ──┐
  [FileB.cs] ──┼──> C# Compiler ──> Assembly.dll
  [FileC.cs] ──┘
  (all files visible to each other)
```

### C++ Model: Each File Is Compiled Alone

In C++, each `.cpp` file is compiled **independently** into an object file. A `.cpp` file can only see what has been `#include`d into it. The **linker** then connects the object files into the final executable.

```
C++ Build:
  [file_processor.hpp]
         |                  |
         v                  v
  [file_processor.cpp] [main.cpp]
         |                  |
         v                  v
  file_processor.o      main.o       (each compiled ALONE)
         |                  |
         +--------+---------+
                  |
                  v
           file_processor            (linker connects them)
```

### What `#include` Actually Does

`#include "file_processor.hpp"` is a **textual copy-paste**. The preprocessor literally replaces the `#include` directive with the entire contents of the header file. Both `file_processor.cpp` and `main.cpp` include the header, so both see the type definitions and function declarations. But neither `.cpp` file can see the other `.cpp` file's function bodies.

This is why you get "undefined reference" linker errors — a concept with no C# equivalent. If you declare a function in the header but forget to define it in the `.cpp`, the compiler is happy (it trusts the declaration), but the linker cannot find the function body.

### The Header Guard

```cpp
#ifndef CPPTUDE_FILE_PROCESSOR_HPP
#define CPPTUDE_FILE_PROCESSOR_HPP
// ... declarations ...
#endif
```

Because `#include` is textual copy-paste, if a header is included twice in the same translation unit, you get duplicate definitions. The `#ifndef`/`#define`/`#endif` pattern (called an **include guard**) prevents this. It is boilerplate, but it is necessary.

> **C++ Future:** C++20 modules replace this model with a proper module system similar to C#'s assemblies. Compiler support is still maturing, so this curriculum uses the traditional header/source model.

---

## The Code, Explained

### Part 1: The RAII Wrapper Classes

#### The C++ Concept

RAII ties resource lifetime to object lifetime. The constructor acquires the resource; the destructor releases it. Because C++ destructors run deterministically at scope exit, the resource is always released — no matter how the scope is exited.

```cpp
class LogFileReader {
public:
    // CONSTRUCTOR: Opens the file (acquires the resource).
    // If this fails, we throw — the object never exists, so the
    // destructor never runs. You only clean up what you acquired.
    explicit LogFileReader(const std::filesystem::path& file_path);

    // DESTRUCTOR: Closes the file (releases the resource).
    // Runs unconditionally at scope exit. This is the RAII guarantee.
    ~LogFileReader();

    // No copying: a file handle is a unique resource.
    LogFileReader(const LogFileReader&) = delete;
    LogFileReader& operator=(const LogFileReader&) = delete;
    LogFileReader(LogFileReader&&) = delete;
    LogFileReader& operator=(LogFileReader&&) = delete;

    // const: reading does not modify the reader's logical state.
    [[nodiscard]] bool read_line(std::string& out_line) const;
    [[nodiscard]] bool is_open() const noexcept;
};
```

Three properties make this work:

1. **Constructor = Acquire.** The file opens when the object is created. If the open fails, the constructor throws, and the destructor never runs. There is no "half-constructed" object with a leaked resource.

2. **Destructor = Release.** The file closes when the object is destroyed. The destructor runs at scope exit — always. Normal return, early return, `break`, `continue`, or exception — it does not matter.

3. **Scope = Lifetime.** The object lives exactly as long as the enclosing scope. When the `}` is reached, destruction happens. This is deterministic and predictable.

Here is how RAII looks in practice:

```cpp
LogSummary analyze_log(const std::filesystem::path& log_path) {
    // File opens HERE (constructor)
    const LogFileReader reader{log_path};

    LogSummary summary{};
    std::string line;

    while (reader.read_line(line)) {
        // ... process each line ...
    }

    // File closes HERE (destructor runs at scope exit)
    // You did not write reader.close(). You cannot forget it.
    return summary;
}
```

If any code between the constructor and the return were to throw an exception, the destructor would still run. The file would still close. This is not a convention or a best practice — it is a language guarantee.

#### The C# Bridge

In C#, the equivalent pattern is `IDisposable` with a `using` statement:

```csharp
// C# RAII-like pattern: using statement
using var reader = new StreamReader(path);
// reader.Dispose() is called when the scope exits.
```

The `using` statement works well, but there are critical differences:

| Aspect | C# `using` / `IDisposable` | C++ RAII |
|--------|---------------------------|----------|
| Opt-in or default? | Opt-in. You must remember to write `using`. | Default. Destructors always run. |
| What if you forget? | The GC/finalizer eventually cleans up (maybe). | The resource leaks. Permanently. |
| Who calls cleanup? | The developer (via `using`) or the GC (eventually). | The language (unconditionally at scope exit). |
| Deterministic? | Only with `using`. GC timing is nondeterministic. | Always deterministic. |
| Thread safety of cleanup? | Finalizer runs on a separate thread. | Destructor runs on the same thread, same scope. |

The C# developer's instinct is: "Resource cleanup is important but the runtime has my back if I forget." In C++, there is no runtime. RAII is how C++ provides the same safety guarantee — but it is built into every object's lifetime, not bolted on as an interface.

The key mental shift: **In C++, the destructor IS the `using` block, and you get it for free on every object that has one.** You do not opt in. You cannot opt out.

> **C# Note:** If you've worked with `SafeHandle` in C# (the base class for OS handle wrappers like `SafeFileHandle`), you've seen C#'s closest approximation to RAII. `SafeHandle` ensures that even if you forget `Dispose()`, the handle is released when the GC finalizes the object. But in C++, there is no finalizer fallback — RAII is the only mechanism, which makes it both more critical to understand and more reliable when used correctly.

### Part 2: `const` Member Functions

#### The C++ Concept

Marking a member function `const` tells the compiler that calling this function will not modify the object's state. This is a compile-time contract:

```cpp
// const: this function does not modify the LogFileReader
[[nodiscard]] bool is_open() const noexcept;

// const: reading does not change the reader's logical state
[[nodiscard]] bool read_line(std::string& out_line) const;
```

Why `read_line` is `const` even though it advances the stream position: the stream's read position is an implementation detail. From the caller's perspective, a reader that has read 5 lines and a reader that has read 10 lines are both "a reader of this file." The logical state has not changed. We express this by marking the `std::ifstream` member as `mutable`:

```cpp
private:
    std::filesystem::path file_path_;
    mutable std::ifstream stream_;  // mutable: physical state changes, logical state does not
```

This is the first time you see `const` on a member function. The rule: **if a function does not modify the object, mark it `const`.** The compiler enforces this. If you try to modify a non-mutable member inside a `const` function, the code will not compile.

#### The C# Bridge

C# has `readonly` fields and `init`-only properties, but no equivalent of `const` member functions. In C#, there is no way to declare that a method does not modify the object — you rely on convention and documentation. The closest C# gets is the `[Pure]` attribute from Code Contracts, which indicates a method has no side effects, but this is not compiler-enforced — it is merely a hint for static analysis tools.

In C++, the compiler enforces `const`. If you try to modify a non-`mutable` member inside a `const` function, the code does not compile. This is another instance of the broader C++ principle: make invariants checkable at compile time, not at code review time.

### Part 3: Exception Safety Through RAII

#### The C++ Concept

Consider what happens when processing fails mid-file:

```cpp
void process_log_file(const std::filesystem::path& log_path,
                      const std::filesystem::path& report_path) {
    const LogFileReader reader{log_path};    // File 1 opens
    ReportWriter writer{report_path};        // File 2 opens

    // ... processing happens here ...
    // What if this throws?

}   // File 2 closes (writer destroyed first — reverse construction order)
    // File 1 closes (reader destroyed second)
```

If an exception propagates through this function:
1. The stack unwinds.
2. `writer`'s destructor runs (constructed second, destroyed first).
3. `reader`'s destructor runs (constructed first, destroyed second).
4. Both files are closed. No leaks.

The destruction order is **reverse construction order** — like a stack. This is deterministic and guaranteed by the language. It matches the logical dependency: the writer might depend on the reader's data, so the reader should outlive the writer.

Contrast with the manual approach:

```cpp
// THE WRONG WAY: Manual open/close
auto* in_file = new std::ifstream{log_path};   // Opened
auto* out_file = new std::ofstream{report_path}; // Opened

// ... if this throws, both file handles leak! ...
// There is no GC to eventually close them.

delete out_file;  // Never reached if exception propagates
delete in_file;   // Never reached if exception propagates
```

In C#, the GC and finalizer would eventually close these handles. In C++, they are gone. The handles leak for the lifetime of the process. RAII prevents this entire class of bugs.

#### The C# Bridge

C# developers handle this scenario with `try`/`finally`:

```csharp
FileStream? file = null;
try {
    file = File.OpenRead(path);
    // ... process ...
} finally {
    file?.Dispose();  // Cleanup even on exception
}
```

Or more idiomatically with `using`:

```csharp
using var file = File.OpenRead(path);
// ... process ...
// Dispose() called at end of scope — similar to RAII
```

The C# `using` statement is the closest equivalent to RAII, and it works well. The difference is that in C#, `using` is something you choose to write. In C++, the destructor runs whether you asked for it or not. Every C++ object with a destructor is automatically exception-safe with respect to resource cleanup. You get this for free by following RAII.

### Part 4: Multi-File Structure

#### The C++ Concept

This cpptude separates declarations (header) from definitions (implementation):

| File | Purpose |
|------|---------|
| `file_processor.hpp` | Class declarations, function signatures, type definitions |
| `file_processor.cpp` | Function bodies, constructor/destructor implementations |
| `main.cpp` | Test harness, demonstrations, `main()` entry point |

Both `.cpp` files `#include "file_processor.hpp"`. The compiler processes each `.cpp` independently, producing an object file. The linker combines them into the final executable.

Why separate header and implementation?
- **Compile time.** Changing a function body in `file_processor.cpp` only recompiles that file. Changing the header recompiles everything that includes it.
- **Encapsulation.** Users of the class see only the header. Implementation details stay in the `.cpp`.
- **Dependency management.** Headers should include as little as possible to minimize compile-time dependencies.

#### The C# Bridge

In C#, there is no header/source split. A class definition in a `.cs` file contains both declarations and implementations. The C# compiler processes all files together, so it never needs forward declarations or include guards. The C++ compilation model requires more ceremony, but it gives you fine-grained control over compilation boundaries and build times.

---

## Deep Dive: What RAII Really Means

### The Name

RAII stands for "Resource Acquisition Is Initialization." The name is misleading — it sounds like it is about initialization, but it is really about **guaranteed cleanup**. A better name would be "Scope-Bound Resource Management" (SBRM), but RAII is the established term.

The principle: **bind the lifetime of a resource to the lifetime of an object.** When the object is constructed, the resource is acquired. When the object is destroyed, the resource is released. Because C++ guarantees that destructors run at scope exit, the resource is guaranteed to be released.

### What Counts as a "Resource"?

RAII applies to any resource that must be acquired and released:

| Resource | Acquire | Release |
|----------|---------|---------|
| File handle | `open()` | `close()` |
| Memory | `new` | `delete` |
| Mutex lock | `lock()` | `unlock()` |
| Network socket | `connect()` | `disconnect()` |
| Database connection | `connect()` | `disconnect()` |
| Temporary file | `create()` | `remove()` |

Every one of these follows the same RAII pattern: wrap the resource in a class, acquire in the constructor, release in the destructor. The standard library already provides RAII wrappers for the most common cases:

| Resource | RAII Wrapper |
|----------|-------------|
| File | `std::ifstream`, `std::ofstream` |
| Memory | `std::unique_ptr`, `std::shared_ptr` |
| Mutex | `std::lock_guard`, `std::unique_lock` |

### Destruction Order Is Deterministic

Local variables are destroyed in **reverse construction order**:

```cpp
{
    LogFileReader reader{path};   // Constructed FIRST
    ReportWriter writer{output};  // Constructed SECOND
    // ...
}   // writer destroyed FIRST, then reader — reverse order, always
```

This is deterministic, predictable, and guaranteed. In C#, the GC collects objects in an unspecified order. In C++, you know exactly when each destructor runs and in what order.

### The Destructor Must Not Throw

If a destructor throws while the stack is unwinding due to another exception, `std::terminate` is called and the program dies immediately. For this reason, destructors should be `noexcept` (which is the implicit default for destructors in C++11 and later). Our destructor performs only logging and stream closure — neither of which should throw.

> **C# Bridge:** In C#, the GC finalization thread catches and suppresses exceptions from finalizers. You might never notice a failing finalizer. In C++, a throwing destructor is a program-terminating bug. This strictness is intentional: it forces you to design cleanup that cannot fail.

---

## Key Takeaways

After completing this cpptude, you should understand:

### 1. RAII = Automatic, Exception-Safe Cleanup

```cpp
{
    LogFileReader reader{path};  // Resource acquired
    // ... use the resource ...
}   // Resource released — automatically, unconditionally
```

No `close()`. No `try`/`finally`. No `using`. The destructor runs at scope exit. Period.

### 2. Constructor = Acquire, Destructor = Release

Every RAII class follows this pattern:
- Constructor opens/acquires/locks the resource. If it fails, throw.
- Destructor closes/releases/unlocks the resource. It must not throw.

### 3. Exception Safety Is Free with RAII

If you acquire all resources via RAII objects, your code is exception-safe by construction. There is no code path where a resource leaks, because there is no code path where the destructor does not run.

### 4. Destruction Order Is Reverse Construction Order

```cpp
{
    A a{};  // constructed first
    B b{};  // constructed second
}   // b destroyed first, then a — always, deterministically
```

### 5. `const` Member Functions Are Compiler-Enforced Contracts

```cpp
[[nodiscard]] bool is_open() const noexcept;  // "I promise not to modify this object"
```

Mark member functions `const` when they do not modify the object. The compiler enforces the promise.

### 6. Multi-File Structure Requires Headers

Declarations go in `.hpp`. Implementations go in `.cpp`. Both `.cpp` files include the `.hpp`. The linker connects them. This is fundamentally different from C#'s single-file-per-class model.

> **C# Bridge:** In C#, the runtime manages resource cleanup through the GC and finalizers, with `IDisposable`/`using` as an opt-in deterministic alternative. In C++, RAII makes deterministic cleanup the default for every object. You do not opt in — you get it automatically. The destructor is not `IDisposable`. It is the object's guarantee to the world: "I will clean up after myself, no matter what."

---

## Design Decisions (Assumed)

The following design choices were made without explicit expert guidance:

1. **Explicit RAII wrapper vs. using `std::ifstream` directly.** We wrap `std::ifstream` in `LogFileReader` to make destructor behavior visible through logging. In production code, `std::ifstream` is already an RAII type — its destructor closes the file. The wrapper exists purely for pedagogical observability.

2. **`mutable` for the stream member.** `read_line` is marked `const` because it does not change the logical state of the reader. The physical stream position is an implementation detail. This introduces `mutable`, which is a secondary teaching point — if it causes confusion, the `const` on `read_line` could be removed to simplify.

3. **Copy and move deleted (not just copy).** We delete all four copy/move operations. A more complete design would implement move operations to allow transferring ownership of the file handle. We defer this to Cpptude #004 (Move Semantics) to keep the focus on RAII.

4. **Logging to stderr.** Destructor logging goes to `stderr` so it does not interfere with `stdout` output. This is a convention, not a requirement.

5. **`std::filesystem::path` parameters.** We use `std::filesystem::path` instead of `std::string` for file paths. This is idiomatic modern C++ and avoids platform-specific path separator issues.

---

## Next Step Challenge

**Challenge: RAII Mutex Logger**

Write an RAII wrapper for a simulated mutex that logs its lock/unlock behavior:

```cpp
class ScopedLock {
public:
    explicit ScopedLock(std::string_view resource_name);
    ~ScopedLock();

    // No copying, no moving
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;
    ScopedLock(ScopedLock&&) = delete;
    ScopedLock& operator=(ScopedLock&&) = delete;
};
```

Requirements:
- The constructor prints `[LOCK] Acquired: <resource_name>`
- The destructor prints `[LOCK] Released: <resource_name>`
- Write a function that acquires two locks and verify (via the log output) that they are released in reverse order
- Write a function that acquires a lock and then throws — verify the lock is still released
- Mark `resource_name()` as a `const` member function

This exercises:
- Writing your own RAII class from scratch
- Verifying destruction order empirically
- Verifying exception-path cleanup empirically
- `const` member functions

---

## Homework

Re-read your File Processor solution. For every RAII object, draw a bracket showing where the constructor runs and where the destructor runs. Then answer: what would leak if you removed the destructor from `LogFileReader`? What would happen if `analyze_log` threw an exception after opening the file but before returning?
