# Cpptude #003: Parameter Passing — Value, `const&`, `&`, and `&&`

| Field | Value |
|-------|-------|
| **Primary KAP** | Parameter Passing Modes (Rating 5 - Critical) |
| **Secondary Concepts** | `const` references, output parameters, rvalue references (preview), copy vs move cost |
| **Difficulty** | Intermediate |
| **Prerequisite** | Cpptude #002 (File Processor — RAII) |
| **Bait** | Pass large structs by value everywhere (C# reference-type instinct) |
| **Failure Signal** | Copy counters spike; performance cliff; caller state unchanged after mutation attempt |
| **Verification Tool** | Instrumented copy/move counters + unit tests on caller state |

---

## Overview

This cpptude teaches the **four parameter-passing modes** in C++:

- `T` (pass by value)
- `const T&` (pass by const reference)
- `T&` (pass by mutable reference)
- `T&&` (pass by rvalue reference)

The exercise makes the cost of copies visible and forces the learner to decide which functions *should* mutate the caller and which should not.

> **C# Bridge:** In C#, reference types are passed by reference-handle by default. `=` and method calls pass the handle, not the object. In C++, `T` means *copy the object*. If you pass a large struct by value, you are copying it. This cpptude makes that cost impossible to ignore.

---

## Session Structure

| Phase | Duration | Activity |
|-------|----------|----------|
| **1. Warm-Up and Review** | 15-20 min | Review RAII and destructor guarantees from #002. Preview that function signatures control copying. |
| **2. Trap Encounter** | 30-45 min | Present the problem with a large `TelemetryBatch` struct. Let the learner pass it by value everywhere. Use counters to show massive copies. |
| **3. Concept Introduction** | 20-30 min | Explain the four modes, `const` reference as the default, and when to use `T&` for mutation. Preview `T&&` for move-only types. |
| **4. Guided Practice** | 30-45 min | Refactor signatures to `const T&` or `T&`. Add an overload that takes `T&&` for move-only input. |
| **5. Independent Practice** | 20-30 min | Challenge: design a small API that minimizes copies while keeping call sites readable. |

---

## The Problem: Telemetry Batch Processor

You are given a **batch of telemetry events** arriving from a game server. Each event includes payload data, timestamps, and a diagnostic string. The batch is large (tens of thousands of events), and you must:

1. Validate every event.
2. Normalize timestamps.
3. Partition events by severity.
4. Produce a summary report.

The core type is intentionally large to make copies obvious:

```cpp
struct TelemetryEvent {
    std::string message;
    std::array<double, 16> metrics;
    std::vector<std::byte> payload;
    std::chrono::system_clock::time_point timestamp;
};

struct TelemetryBatch {
    std::vector<TelemetryEvent> events;
    std::string source;
};
```

### Required Functions

You will implement and choose correct signatures for the following functions:

```cpp
TelemetryBatch load_batch(const std::filesystem::path& file_path);

void normalize_timestamps(TelemetryBatch& batch, std::chrono::seconds offset);

std::vector<const TelemetryEvent*> filter_errors(const TelemetryBatch& batch);

void append_batch(TelemetryBatch& destination, const TelemetryBatch& source);

TelemetryBatch merge_batches(TelemetryBatch left, TelemetryBatch right);
```

The key learning goal: **every signature should express ownership and intent**. Which functions must mutate the caller? Which ones simply inspect? Which ones can take ownership and move?

---

## Signature Rationale (What Each One Teaches)

```cpp
TelemetryBatch load_batch(const std::filesystem::path& file_path);
```

- Returns by value because the caller receives a *new* owned batch.

```cpp
void normalize_timestamps(TelemetryBatch& batch, std::chrono::seconds offset);
```

- Mutable reference because the function must update the caller's batch in place.

```cpp
std::vector<const TelemetryEvent*> filter_errors(const TelemetryBatch& batch);
```

- `const T&` because filtering should not mutate the batch.
- Returns non-owning pointers to avoid copying events.

```cpp
void append_batch(TelemetryBatch& destination, const TelemetryBatch& source);
```

- Destination mutates; source is read-only.

```cpp
TelemetryBatch merge_batches(TelemetryBatch left, TelemetryBatch right);
```

- By-value parameters enable **move semantics** at the call site.
- Callers can write `merge_batches(std::move(a), std::move(b))` to transfer ownership.

---

## Instrumentation: Make Copies Visible

Add counters to make copy cost impossible to ignore:

```cpp
struct TelemetryEvent {
    static inline std::size_t copy_count = 0;
    static inline std::size_t move_count = 0;

    TelemetryEvent() = default;
    TelemetryEvent(const TelemetryEvent& other)
        : message(other.message),
          metrics(other.metrics),
          payload(other.payload),
          timestamp(other.timestamp) {
        ++copy_count;
    }

    TelemetryEvent(TelemetryEvent&& other) noexcept
        : message(std::move(other.message)),
          metrics(std::move(other.metrics)),
          payload(std::move(other.payload)),
          timestamp(other.timestamp) {
        ++move_count;
    }

    TelemetryEvent& operator=(const TelemetryEvent& other) {
        if (this != &other) {
            message = other.message;
            metrics = other.metrics;
            payload = other.payload;
            timestamp = other.timestamp;
            ++copy_count;
        }
        return *this;
    }

    TelemetryEvent& operator=(TelemetryEvent&& other) noexcept {
        if (this != &other) {
            message = std::move(other.message);
            metrics = std::move(other.metrics);
            payload = std::move(other.payload);
            timestamp = other.timestamp;
            ++move_count;
        }
        return *this;
    }
};

struct TelemetryBatch {
    static inline std::size_t copy_count = 0;
    static inline std::size_t move_count = 0;

    TelemetryBatch() = default;
    TelemetryBatch(const TelemetryBatch& other)
        : events(other.events),
          source(other.source) {
        ++copy_count;
    }

    TelemetryBatch(TelemetryBatch&& other) noexcept
        : events(std::move(other.events)),
          source(std::move(other.source)) {
        ++move_count;
    }

    TelemetryBatch& operator=(const TelemetryBatch& other) {
        if (this != &other) {
            events = other.events;
            source = other.source;
            ++copy_count;
        }
        return *this;
    }

    TelemetryBatch& operator=(TelemetryBatch&& other) noexcept {
        if (this != &other) {
            events = std::move(other.events);
            source = std::move(other.source);
            ++move_count;
        }
        return *this;
    }
};
```

Run the program with your original signatures, then again after refactoring. The copy counts should collapse.

---

## The Four Parameter Modes (Cheat Sheet)

| Mode | Meaning | Typical Use |
|------|---------|-------------|
| `T` | Copy the object | Small, cheap-to-copy types or when you need an owned copy |
| `const T&` | Borrow immutably | Default for large objects that you only read |
| `T&` | Borrow mutably | When you need to modify the caller's object |
| `T&&` | Take ownership / move | When the caller is willing to give up the object |

### Recommended Defaults

- **Default to `const T&`** for large user-defined types.
- Use `T` for small trivially copyable types (`int`, `double`, small structs).
- Use `T&` when the function *must* mutate the caller.
- Use `T&&` only when you truly intend to consume the argument.

> **Rule of Thumb:** If the function does not need to modify the caller and you want to avoid copies, use `const T&`.

---

## The Trap: “Passing by Value Is Free”

C# developers are used to passing reference types by handle. This feels free. In C++:

```cpp
void process_batch(TelemetryBatch batch); // copies the entire batch
```

That function copies every event, every payload, and every string. The cost is enormous. The cpptude makes this visible by instrumenting the copy constructor of `TelemetryBatch` and `TelemetryEvent` to count copies.

You will see output like:

```
TelemetryEvent copies: 50,000
TelemetryBatch copies: 12
```

Your task: refactor signatures so those numbers drop to near zero.

---

## C# Bridge: `ref`, `in`, and `out`

C# has `ref`, `in`, and `out` parameters, but they are not the default. The default is still passing a reference handle for reference types. In C++, the default is **value** — a copy. That makes C++ *strict* but also precise: if you see `const T&`, you know no copy happens. If you see `T`, you know a copy happens.

This clarity is a feature, not a burden.

### Parameter Mode Comparison Table

| C++ Mode | C# Equivalent | Notes |
|----------|---------------|-------|
| `T` (pass by value) | Value type by value, or copying a `struct` | Rare in C# for large types; in C++, this is the default for all types |
| `const T&` | `in T` (C# 7.2+) for value types | Both prevent copies AND mutation; `in` is C#'s closest match |
| `T&` | `ref T` | Explicit at both declaration and call site in C# |
| `T&&` | No equivalent | C# has no concept of "consuming" or "moving from" an argument |

### Why C++ Has Four Modes When C# Has Two

In C#, reference types are always passed by handle — the GC manages the underlying object's lifetime, so there is no need to distinguish between "borrow" and "consume." The GC tracks all references and cleans up when no references remain.

In C++, there is no GC. The caller and callee must agree on who owns the object and for how long. The four parameter modes express this ownership contract:

- **`T`**: "I want my own copy. Your object is unaffected."
- **`const T&`**: "I will borrow this to read from. I promise not to modify it."
- **`T&`**: "I will borrow this and may modify it. You keep ownership."
- **`T&&`**: "Give me ownership. You should not use this object afterward."

### C# `struct` Types and Value Semantics

One alignment between C# and C++: C# `struct` types (value types) ARE passed by value by default, just like C++ types. In C#, if you pass a large `struct` without `ref` or `in`, you copy it:

```csharp
// C# — struct is copied by value
void Process(LargeStruct data) { /* data is a copy */ }

// C# 7.2+ — 'in' prevents copy AND prevents mutation (like const T&)
void Process(in LargeStruct data) { /* data is a readonly reference */ }
```

The C# `in` parameter modifier (introduced in C# 7.2) is semantically similar to C++ `const T&`: it passes by reference to avoid copying, and it prevents modification. If you understand `in`, you understand `const T&`.

The key difference: in C# you use `in` only for large `struct` types as an optimization. In C++, `const T&` is the idiomatic default for *any* non-trivial type.

### The "Reference Type by Reference" Confusion

A common source of confusion: in C#, "passing by reference" for a reference type (`class`) is different from passing a value type by `ref`. When you pass a `List<int>` to a method, you pass the *handle* (reference) by value. The method receives a copy of the handle pointing to the same underlying object.

```csharp
// C# — handle is copied, but both handles point to the same List
void AddItem(List<int> list) {
    list.Add(42);  // Modifies the original list
    list = new List<int>();  // Does NOT affect caller's variable
}
```

In C++, there is no handle/object distinction for value types. When you pass `T&`, you pass a reference to the actual object. When you pass `T`, you copy the entire object — not a handle to it.

---

## Next Step Challenge

Design a small API for a `TelemetryIndex` type with these operations:

- `add_batch` (takes ownership)
- `preview_batch` (read-only view)
- `merge_from` (mutates the index)
- `snapshot` (returns a copy)

Write the signatures only. Then justify each parameter mode in one sentence.

---

## Key Takeaways

1. **`T` means copy.** Use it when you really want a copy.
2. **`const T&` is the default for large, read-only types.**
3. **`T&` makes mutation explicit and visible.**
4. **`T&&` expresses ownership transfer.** It is not a “faster reference.”
5. **Correct signatures communicate intent and prevent accidental copies.**

> **C# Bridge:** C# hides copying and ownership behind a reference-handle model. C++ makes it explicit. Learning parameter passing modes is the first step toward writing efficient, correct, modern C++.
