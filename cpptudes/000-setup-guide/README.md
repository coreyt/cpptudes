# Cpptude #000: Setup Guide -- Toolchain Verification and Sanitizer Fundamentals

| Field | Value |
|-------|-------|
| **Primary KAP** | Tooling: ASan, UBSan, `-Wall -Wextra -Werror` |
| **Secondary Concepts** | CMake build configuration, reading sanitizer reports, brace initialization |
| **Difficulty** | Introductory |
| **Constraint** | None (this is a setup exercise, not a problem-domain cpptude) |

---

## Overview

This cpptude establishes the **tooling infrastructure** that every subsequent cpptude depends on. It is not a full problem-solving exercise. It is a guided walkthrough that answers one question: *How does C++ tell you when something is wrong?*

The answer, for a C# developer, is surprising: **by default, it does not.** C++ compiles and runs programs that contain use-after-free, signed integer overflow, and uninitialized variable reads -- silently producing wrong results or corrupting memory. There is no runtime to throw `NullReferenceException` or `OverflowException`. There is no compiler enforcing definite assignment by default.

The fix is tooling: AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and `-Wall -Wextra -Werror`. Once configured, these tools catch bugs with detailed diagnostic reports that are as informative as any C# exception stack trace. But you must opt into them.

> **C# Bridge:** If you are coming from C#, this cpptude targets the instinct that "the runtime catches my errors." In C#, the CLR is your safety net: it throws exceptions for null dereferences, enforces definite assignment at compile time, and offers `checked` arithmetic. In C++, there is no CLR. Sanitizers and compiler warnings are the safety net you build for yourself.

---

## Building & Running

This cpptude is designed to be built twice: once without sanitizers (to see silent failure) and once with sanitizers (to see the bugs caught).

### Step 1: Build Without Sanitizers (The "Silent Failure" Experience)

```bash
g++ -std=c++20 -O1 main.cpp -o setup
./setup
```

The program compiles without errors. It runs. It prints results. Those results are **wrong** -- or at least undefined -- but nothing tells you that. This is what C++ development looks like without tooling.

### Step 2: Build With Sanitizers (The "Caught" Experience)

```bash
g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer -Wall -Wextra main.cpp -o setup
./setup
```

Now ASan catches the use-after-free. UBSan catches the signed overflow. And `-Wall -Wextra` warns about the uninitialized read. The program halts with a detailed diagnostic report instead of silently producing garbage.

### Step 3: Build With -Werror (The "Compiler Refuses" Experience)

```bash
g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer -Wall -Wextra -Werror main.cpp -o setup
```

This time, the code does not compile at all. The uninitialized variable read is caught as a compile error -- the same category of protection that C# provides with definite assignment (CS0165). The `-Werror` flag turns the compiler warning into an error.

### Using CMake

```bash
# "Naive" build (Release) -- no sanitizers, bugs are silent
cmake -B build-naive -DCMAKE_BUILD_TYPE=Release
cmake --build build-naive
./build-naive/setup-guide

# "Safe" build (Debug) -- sanitizers + -Werror, bugs are caught
cmake -B build-safe -DCMAKE_BUILD_TYPE=Debug
cmake --build build-safe
./build-safe/setup-guide
```

**Note:** The Debug build with `-Werror` will refuse to compile because of the uninitialized variable bug. This is the intended outcome -- see Bug 3 below.

---

## The Code, Explained

### Part 1: Use-After-Free

#### The C++ Concept

In C++, `delete` frees memory immediately. The pointer still exists, still holds the old address, but the memory it points to is no longer yours. Accessing it is undefined behavior -- the program might return the old value (because the memory has not been overwritten yet), return garbage (because something else reused the memory), or crash (because the OS reclaimed the page).

```cpp
int use_after_free() {
    int* p = new int{42};   // Allocate an int on the heap
    int value = *p;         // Read while still valid: OK
    delete p;               // Free the memory

    return *p + value;      // UNDEFINED BEHAVIOR: heap-use-after-free
}
```

The bug is on the last line: `*p` reads memory that has already been freed. Without sanitizers, this often "works" -- the value 42 may still be sitting in that memory location because nothing has overwritten it yet. The program returns 84, and you have no idea anything is wrong. This is the silent corruption that makes C++ bugs so dangerous.

With ASan enabled, the program halts immediately with a report like this:

```
==12345==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000000010
READ of size 4 at 0x602000000010 thread T0
    #0 0x... in use_after_free() main.cpp:52
    #1 0x... in main main.cpp:126

freed by thread T0 here:
    #0 0x... in operator delete(void*)
    #1 0x... in use_after_free() main.cpp:47

previously allocated by thread T0 here:
    #0 0x... in operator new(unsigned long)
    #1 0x... in use_after_free() main.cpp:44
```

ASan tells you exactly what happened: the memory was allocated (line 44), freed (line 46), and then read (line 48). It names the file, the line, and the function. This is the level of detail you need to find and fix the bug.

#### The C# Bridge

In C#, this situation cannot arise. The garbage collector ensures that memory is only freed when no references point to it. If you hold a reference to an object, that object stays alive. The closest C# analogy is accessing a disposed object:

```csharp
var stream = new FileStream("data.txt", FileMode.Open);
stream.Dispose();
stream.Read(buffer, 0, 100);  // ObjectDisposedException
```

But even this is not the same. `Dispose()` releases the *resource* (file handle), not the *memory*. The `stream` object itself is still in memory, still has valid fields, and the `Read()` method runs real code that checks a flag and throws a managed exception. In C++, `delete` frees the memory itself. The pointer becomes a dangling pointer -- it points to memory that may now belong to something else entirely.

**Key bridge point:** ASan is the C++ equivalent of the CLR's memory safety guarantee, but it is a *detection* tool, not a *prevention* tool. The CLR prevents use-after-free by design (garbage collection). ASan detects use-after-free at runtime and halts the program. You must opt into it.

### Part 2: Signed Integer Overflow

#### The C++ Concept

In C++, signed integer overflow is undefined behavior. This does not mean "it wraps around" or "it produces a wrong value." It means the language standard places no constraints on what happens. The compiler is allowed to assume signed overflow never occurs, and it uses that assumption to optimize your code.

```cpp
int signed_overflow() {
    const int max = std::numeric_limits<int>::max();  // 2,147,483,647

    return max + 1;         // UNDEFINED BEHAVIOR: signed integer overflow
}
```

In practice, the consequences can range from the "expected" wrapping to -2,147,483,648 (on many platforms, because the hardware uses two's complement) to entirely unexpected behavior where the compiler removes code paths it "proves" are unreachable by assuming overflow cannot happen.

With UBSan enabled, the program reports:

```
main.cpp:80:16: runtime error: signed integer overflow:
2147483647 + 1 cannot be represented in type 'int'
```

UBSan catches the overflow at the exact line where it occurs and tells you the values involved. Unlike ASan (which halts the program by default), UBSan prints the report and continues execution, so you may see reports for multiple bugs in a single run.

#### The C# Bridge

In C#, signed integer overflow has two defined behaviors, depending on context:

```csharp
// Checked context: throws System.OverflowException
checked {
    int max = int.MaxValue;
    int result = max + 1;  // throws OverflowException
}

// Unchecked context (default): wraps via two's complement
unchecked {
    int max = int.MaxValue;
    int result = max + 1;  // result is -2147483648 (predictable)
}
```

Both of these are *defined behavior*. C# specifies exactly what happens in each case. Even the wrapping in `unchecked` mode is predictable -- it always produces the two's complement result.

C++ has no such contract. Signed overflow is undefined behavior, which means:

1. The compiler does not guarantee wrapping. On most hardware it happens to wrap, but the compiler is not required to preserve that behavior.
2. The compiler may *assume* overflow does not happen and use that assumption to optimize. For example, it may determine that a loop "must terminate" because the loop counter "cannot overflow," and remove the termination check.
3. Different optimization levels can produce different results for the same code.

**Key bridge point:** UBSan gives you something stronger than C#'s `checked` keyword. C# `checked` only catches overflow at the point where it happens. UBSan catches all categories of undefined behavior -- not just overflow -- and it does so at runtime with zero source code changes. You enable it through a compiler flag, not through code annotations.

### Part 3: Uninitialized Variable Read

#### The C++ Concept

In C++, local variables are not automatically initialized. When you declare `int x;` without an initializer, `x` holds whatever bytes happened to be at that stack location from a previous function call. Reading that value is undefined behavior.

```cpp
int uninitialized_read() {
    int x;                  // Not initialized -- holds stack garbage

    return x + 1;           // UNDEFINED BEHAVIOR: uninitialized read
}
```

Without `-Wall -Wextra`, some compilers will not warn about this. The code compiles, runs, and produces a different value on every execution (or even on different calls within the same execution). This is a class of bug that is notoriously difficult to reproduce because the "garbage" value depends on what was on the stack before.

With `-Wall -Wextra`, the compiler warns:

```
main.cpp:109:16: warning: 'x' is used uninitialized [-Wuninitialized]
  109 |     return x + 1;
      |                ^
```

With `-Werror` added, this warning becomes a compile error. The compiler refuses to build the code -- the same protection that C# provides by default.

#### The C# Bridge

In C#, the compiler enforces definite assignment analysis. You cannot read a local variable that the compiler cannot prove has been assigned a value:

```csharp
int x;
return x + 1;  // Compile error CS0165: Use of unassigned local variable 'x'
```

This is not a warning. It is a hard error. The C# compiler will not produce an executable.

In C++, this protection is available but not enabled by default. The `-Wall` flag enables the warning; `-Werror` makes it a compile error. This is why `-Wall -Wextra -Werror` is non-negotiable infrastructure for every cpptude: it gives you back a safety guarantee that C# provides out of the box.

**Key bridge point:** `-Werror` is the C++ equivalent of C#'s "treat warnings as errors" setting (which many C# projects also enable). The difference is that in C#, definite assignment is always a hard error. In C++, you must opt into making uninitialized reads a hard error. This cpptude configures that opt-in for every future build.

> **Brace initialization sidebar:** This is why cpptudes use brace initialization (`int x{0};` or `int x{};`) as the default. Brace initialization always initializes the variable. `int x{};` gives you zero. `int x;` gives you garbage. The braces are not decoration -- they are a safety mechanism. Every cpptude from this point forward uses brace initialization by default.

---

## Deep Dive: What Are Sanitizers?

### How Sanitizers Work

Sanitizers are compiler features, not separate tools. When you pass `-fsanitize=address` to the compiler, it instruments your code -- it inserts additional checks around every memory access, allocation, and deallocation. The resulting binary is larger and slower (typically 2-3x for ASan), but it detects bugs that would otherwise be silent.

| Sanitizer | Flag | What It Catches | Runtime Overhead |
|-----------|------|-----------------|-----------------|
| AddressSanitizer (ASan) | `-fsanitize=address` | Use-after-free, heap/stack buffer overflow, memory leaks | ~2x slowdown, ~3x memory |
| UndefinedBehaviorSanitizer (UBSan) | `-fsanitize=undefined` | Signed overflow, null dereference, misaligned access, shift UB | Minimal (< 10%) |

You can combine them: `-fsanitize=address,undefined` enables both.

### Sanitizers vs. the CLR Safety Net

| Bug Category | C# (CLR) | C++ (No Tooling) | C++ (With Tooling) |
|-------------|----------|-------------------|---------------------|
| Null dereference | `NullReferenceException` | Segfault or silent corruption | ASan report |
| Use-after-free | Impossible (GC prevents it) | Silent corruption | ASan report |
| Buffer overflow | `IndexOutOfRangeException` | Silent corruption | ASan report |
| Integer overflow | `OverflowException` (checked) or defined wrap (unchecked) | Undefined behavior | UBSan report |
| Uninitialized read | Compile error (CS0165) | Silent garbage | `-Wall -Werror` compile error |

The pattern is clear: C# provides safety by default through the CLR. C++ provides safety through tooling that you must explicitly enable. Once enabled, the C++ tooling is at least as informative as C# exceptions -- sanitizer reports include full stack traces, allocation history, and the exact bytes involved.

### When to Use Sanitizers

- **During development:** Always. Every build you run locally should have sanitizers enabled. This is the Debug configuration in the CMakeLists.txt.
- **In CI/CD:** Always. Your continuous integration pipeline should build and test with sanitizers.
- **In production:** Usually not. Sanitizers have runtime overhead (ASan uses 2-3x memory). Production builds use optimized Release configurations without sanitizers.

This is different from C#, where the CLR safety net is always present -- even in production. In C++, the development-time safety net (sanitizers) is removed in production. The safety guarantee comes from having *tested thoroughly* with sanitizers, not from having them active at runtime.

---

## Key Takeaways

After completing this cpptude, you should understand:

### 1. C++ Does Not Have a Runtime Safety Net

```cpp
int* p = new int{42};
delete p;
*p;  // In C#: impossible. In C++: compiles, runs, silently corrupts.
```

The CLR's garbage collector, null-reference checks, and bounds checking do not exist in C++. If your code has a memory bug, the program will not tell you -- unless you enable tooling.

### 2. Sanitizers Are Infrastructure, Not Debugging Tools

You do not reach for ASan when something goes wrong. You enable ASan *before anything goes wrong* and keep it on for every debug build. Think of sanitizers as the C++ equivalent of the CLR: always watching, always checking.

```bash
# This is your default build command from now on:
g++ -std=c++20 -O1 -g -fsanitize=address,undefined \
    -fno-omit-frame-pointer -Wall -Wextra -Werror main.cpp -o program
```

### 3. -Werror Restores Compile-Time Safety

```cpp
int x;       // C#: compile error CS0165
return x;    // C++ without -Werror: compiles with warning (or silently!)
             // C++ with -Werror: compile error
```

`-Werror` turns compiler warnings into errors. For categories of bugs that the compiler can detect (uninitialized variables, narrowing conversions, unused results), this gives you the same compile-time safety that C# provides by default.

### 4. Undefined Behavior Means "Anything Can Happen"

```cpp
int max = std::numeric_limits<int>::max();
int result = max + 1;  // NOT "wraps to INT_MIN"
                        // NOT "implementation-defined"
                        // UNDEFINED: the compiler assumes it cannot happen
```

This is the most foreign concept for a C# developer. C# defines behavior for every situation. C++ intentionally leaves some situations undefined, allowing the compiler to optimize aggressively. UBSan catches these situations at runtime.

### 5. Brace Initialization Prevents Uninitialized Variables

```cpp
int x;      // DANGEROUS: uninitialized, holds stack garbage
int y{};    // SAFE: zero-initialized
int z{42};  // SAFE: initialized to 42
```

From this point forward, every cpptude uses brace initialization as the default. The braces are not style -- they are safety.

> **C# Bridge:** In C#, `int x;` in a method body is unassigned but reading it is a compile error. In C++, `int x;` is uninitialized *and* reading it is undefined behavior that compiles without error (by default). Brace initialization (`int x{};`) closes this gap by ensuring every variable starts with a known value.

---

## Design Decisions (Assumed)

The following design choices were made without explicit expert guidance:

1. **Single file structure:** This cpptude uses a single `main.cpp` with no headers. Multi-file structure is introduced in cpptude #2 (File Processor) with a Build Model sidebar. Keeping Week 0 to a single file reduces cognitive load.

2. **Three bugs, not more:** The program demonstrates exactly three bug categories (one per sanitizer/tool). Adding more would dilute the primary lesson: that these tools exist and how to read their output.

3. **The uninitialized read requires two builds:** Because `-Werror` prevents the uninitialized read from compiling, the learner must first build without `-Werror` to see the runtime behavior, then build with `-Werror` to see the compile-time catch. This two-step demonstration is intentional -- it shows the progression from "C++ lets you do this" to "C++ with proper flags stops you."

4. **UBSan continues after reporting:** UBSan's default behavior is to print the report and continue execution (unlike ASan, which halts). This means the learner may see the UBSan report for signed overflow followed by ASan halting on use-after-free, depending on function call order. The code calls `use_after_free()` first so ASan fires before UBSan, ensuring the learner sees each report in isolation by commenting out earlier functions.

---

## Next Step Challenge

**Challenge: Read and Interpret Sanitizer Reports**

Comment out all three bug functions except one. Rebuild with sanitizers and run the program. For each bug:

1. Read the sanitizer report carefully.
2. Identify the exact line where the bug occurs.
3. Explain what happened using the sanitizer's output (allocation site, free site, access site for ASan; operation and values for UBSan).
4. Write a one-sentence fix for each bug.

Then, trigger one additional bug that we did not demonstrate: a **stack buffer overflow**. Add a function like this:

```cpp
int stack_buffer_overflow() {
    int arr[5]{};
    return arr[10];   // Out-of-bounds read
}
```

Build with ASan and observe the report. Compare it to the use-after-free report. What is different? What is the same?

Requirements:
- Each bug should be tested in isolation (comment out the others).
- You should see three distinct sanitizer/compiler reports.
- The stack buffer overflow is a new bug type, but ASan catches it the same way.

---

## Homework

Read the [AddressSanitizer documentation](https://clang.llvm.org/docs/AddressSanitizer.html) page for one error type you triggered today (e.g., "heap-use-after-free" or "stack-buffer-overflow"). Write a one-sentence summary of what ASan checks for that error type.

Time estimate: 15-20 minutes.
