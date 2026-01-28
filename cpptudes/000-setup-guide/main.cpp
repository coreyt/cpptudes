// main.cpp -- Cpptude #000: Setup Guide
//
// This program deliberately contains three categories of C++ bugs.
// The goal is to see how each bug behaves WITHOUT sanitizers (silently wrong)
// vs. WITH sanitizers (caught and reported).
//
// BUILD WITHOUT SANITIZERS (the "silent failure" experience):
//   g++ -std=c++20 -O1 main.cpp -o setup
//   ./setup
//
// BUILD WITH SANITIZERS (the "caught" experience):
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -fno-omit-frame-pointer -Wall -Wextra main.cpp -o setup
//   (join the above two lines with a backslash when typing in your terminal)
//   ./setup
//
// BUILD WITH -Werror (the "compiler refuses to build" experience):
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -fno-omit-frame-pointer -Wall -Wextra -Werror main.cpp -o setup
//   (join the above two lines with a backslash when typing in your terminal)
//
// Each bug is in its own function so you can comment them out individually
// and see each sanitizer report in isolation.

#include <cstdlib>
#include <iostream>
#include <limits>

// =============================================================================
// BUG 1: Use-After-Free
// =============================================================================
//
// C# INSTINCT: The garbage collector ensures that any object you hold a
//              reference to stays alive. You cannot access freed memory
//              because the GC only frees memory when no references remain.
//
// C++ REALITY: delete frees memory immediately, regardless of whether
//              pointers to that memory still exist. Reading freed memory
//              is undefined behavior -- anything can happen, including
//              "appearing to work" on some runs and crashing on others.
//
// DETECTED BY: AddressSanitizer (ASan)

int use_after_free() {
    int* p = new int{42};   // Allocate an int on the heap
    int value = *p;         // Read while still valid: OK
    delete p;               // Free the memory

    // BUG: p still holds the old address, but the memory is freed.
    // In C#, this situation cannot arise -- the GC prevents it.
    // In C++, this compiles, and may even "work" without sanitizers.
    return *p + value;      // UNDEFINED BEHAVIOR: heap-use-after-free
}

// =============================================================================
// BUG 2: Signed Integer Overflow
// =============================================================================
//
// C# INSTINCT: Integer overflow either throws OverflowException (in a
//              checked context) or wraps predictably via two's complement
//              (in unchecked context, which is the default).
//
// C++ REALITY: Signed integer overflow is undefined behavior. The compiler
//              assumes it NEVER happens and may optimize accordingly --
//              removing branches, reordering code, or producing results
//              that no two's-complement interpretation can explain.
//
// DETECTED BY: UndefinedBehaviorSanitizer (UBSan)

int signed_overflow() {
    // volatile prevents the compiler from constant-folding this value
    // at compile time, ensuring the overflow happens at runtime where
    // UBSan can detect and report it.
    volatile int max = std::numeric_limits<int>::max();  // 2,147,483,647

    // BUG: Adding 1 to INT_MAX is undefined behavior for signed int.
    // In C# unchecked: this wraps to -2,147,483,648 (predictable).
    // In C# checked: this throws System.OverflowException.
    // In C++: this is UB. The compiler may assume it cannot happen.
    return max + 1;         // UNDEFINED BEHAVIOR: signed integer overflow
}

// =============================================================================
// BUG 3: Uninitialized Variable Read
// =============================================================================
//
// C# INSTINCT: The compiler enforces "definite assignment" -- you cannot
//              read a local variable that has not been assigned. This is
//              compile error CS0165. The C# compiler simply refuses to
//              build your code.
//
// C++ REALITY: Without -Wall -Wextra, many compilers will not warn about
//              this. The code compiles and reads whatever garbage was on
//              the stack at that memory location. With -Wall -Wextra, the
//              compiler warns. With -Werror, the warning becomes an error,
//              giving you C#-equivalent protection.
//
// DETECTED BY: -Wall -Wextra (compile-time warning), -Werror (compile error)
//
// NOTE: To see this bug in action, build WITHOUT -Werror first.
//       Then rebuild WITH -Werror to see the compiler refuse.

int uninitialized_read() {
    int x;                  // Not initialized -- holds whatever was on the stack

    // BUG: Reading an uninitialized variable is undefined behavior.
    // In C#, this line would not compile (CS0165).
    // In C++, this compiles (without -Werror) and reads garbage.
    return x + 1;           // UNDEFINED BEHAVIOR: uninitialized read
}

// =============================================================================
// MAIN
// =============================================================================

int main() {
    std::cout << "=== Cpptude #000: Setup Guide ===\n";
    std::cout << "Primary KAP: Toolchain Verification & Sanitizer Fundamentals\n\n";

    std::cout << "This program contains three deliberate bugs.\n";
    std::cout << "Without sanitizers, it may appear to work.\n";
    std::cout << "With sanitizers, each bug is caught and reported.\n\n";

    // --- Bug 1: Use-After-Free ---
    std::cout << "--- Bug 1: Use-After-Free ---\n";
    const int result1 = use_after_free();
    std::cout << "  use_after_free() returned: " << result1 << "\n";
    std::cout << "  (If ASan is enabled, you will not reach this line.)\n\n";

    // --- Bug 2: Signed Integer Overflow ---
    std::cout << "--- Bug 2: Signed Integer Overflow ---\n";
    const int result2 = signed_overflow();
    std::cout << "  signed_overflow() returned: " << result2 << "\n";
    std::cout << "  C# unchecked would give: "
              << static_cast<long long>(std::numeric_limits<int>::max()) + 1
              << " (as long), or wrap to "
              << std::numeric_limits<int>::min() << " (as int)\n";
    std::cout << "  C++ result is UNDEFINED -- any value is 'correct'.\n\n";

    // --- Bug 3: Uninitialized Variable Read ---
    std::cout << "--- Bug 3: Uninitialized Variable Read ---\n";
    const int result3 = uninitialized_read();
    std::cout << "  uninitialized_read() returned: " << result3 << "\n";
    std::cout << "  (This value is garbage -- different every run.)\n\n";

    // --- Summary ---
    std::cout << "=== Summary ===\n";
    std::cout << "If you see this message, all three bugs executed without crashing.\n";
    std::cout << "That is the problem. In C#, at least two of these would have\n";
    std::cout << "been caught (OverflowException in checked mode, CS0165 for\n";
    std::cout << "uninitialized read). In C++, silence is not safety.\n\n";
    std::cout << "Now rebuild with sanitizers and -Werror to see the difference.\n";

    return EXIT_SUCCESS;
}
