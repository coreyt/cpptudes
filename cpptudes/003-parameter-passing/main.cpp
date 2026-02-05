// main.cpp — Cpptude #003: Parameter Passing Test Harness
//
// This file demonstrates the cost difference between:
//   - The C# instinct: passing everything by value
//   - The C++ way: using const T&, T&, and T&& appropriately
//
// Run this program and watch the copy counters. The numbers tell the story.
//
// BUILD:
//   cmake -B build -DCMAKE_BUILD_TYPE=Debug
//   cmake --build build
//   ./build/parameter_passing

#include "telemetry.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

using namespace cpptude;

// =============================================================================
// COUNTER DISPLAY UTILITIES
// =============================================================================

// Print a separator line for visual clarity
void print_separator(char ch = '=', std::size_t width = 70) {
    std::cout << std::string(width, ch) << '\n';
}

// Print current counter values with clear formatting
void print_counters(std::string_view label) {
    // Center the label within 40 characters
    const std::size_t label_width = 40;
    const std::size_t padding_total = (label_width > label.size()) ? (label_width - label.size()) : 0;
    const std::size_t pad_left = padding_total / 2;
    const std::size_t pad_right = padding_total - pad_left;

    std::cout << "\n  [" << std::string(pad_left, ' ') << label
              << std::string(pad_right, ' ') << "]\n";
    std::cout << "  TelemetryEvent  copies: " << std::setw(6) << TelemetryEvent::copy_count
              << "  moves: " << std::setw(6) << TelemetryEvent::move_count << '\n';
    std::cout << "  TelemetryBatch  copies: " << std::setw(6) << TelemetryBatch::copy_count
              << "  moves: " << std::setw(6) << TelemetryBatch::move_count << '\n';
}

// Print counter difference from a baseline
void print_counter_delta(std::string_view operation,
                         std::size_t event_copies_before, std::size_t batch_copies_before) {
    const auto event_copies = TelemetryEvent::copy_count - event_copies_before;
    const auto batch_copies = TelemetryBatch::copy_count - batch_copies_before;

    // Pad operation to 40 characters
    std::cout << "  " << std::left << std::setw(40) << operation << std::right
              << " -> Event copies: " << std::setw(5) << event_copies
              << ", Batch copies: " << std::setw(2) << batch_copies << '\n';
}

// =============================================================================
// MEMORY LAYOUT VISUALIZATION
// =============================================================================

void demo_memory_layout() {
    print_separator();
    std::cout << "DEMO: Memory Layout and Copy Costs\n";
    print_separator();

    std::cout << R"(
  TelemetryEvent memory layout:
  +------------------+--------+----------------------------------------+
  | Member           | Size   | Notes                                  |
  +------------------+--------+----------------------------------------+
  | std::string      | ~32 B  | + heap allocation for content          |
  | std::array<16d>  | 128 B  | Fixed on stack, always copied          |
  | std::vector      | ~24 B  | + heap allocation for payload          |
  | time_point       |   8 B  | Trivial copy                           |
  +------------------+--------+----------------------------------------+
  | TOTAL (approx)   | ~192 B | Plus 2 heap allocations per event!     |
  +------------------+--------+----------------------------------------+

  When you COPY a TelemetryEvent:
    - 192+ bytes copied from stack portion
    - String content: malloc + memcpy (message can be any length)
    - Payload: malloc + memcpy (64-192 bytes per event in our demo)

  When you MOVE a TelemetryEvent:
    - 192 bytes copied (the std::array part is unavoidable)
    - String: just swap 3 pointers (no malloc!)
    - Payload: just swap 3 pointers (no malloc!)

  TelemetryBatch with 1000 events:
    - COPY cost: ~1000 * (192B stack + 2 heap allocs) = ~200KB + 2000 mallocs!
    - MOVE cost: ~24 bytes (just swap the vector's internal pointers)

)";

    // Show actual sizes
    std::cout << "  Actual sizes on this platform:\n";
    std::cout << "    sizeof(TelemetryEvent)  = " << std::setw(4) << sizeof(TelemetryEvent) << " bytes\n";
    std::cout << "    sizeof(TelemetryBatch)  = " << std::setw(4) << sizeof(TelemetryBatch) << " bytes\n";
    std::cout << "    sizeof(std::string)     = " << std::setw(4) << sizeof(std::string) << " bytes\n";
    std::cout << "    sizeof(std::vector<T>)  = " << std::setw(4) << sizeof(std::vector<int>) << " bytes\n";
    std::cout << '\n';
}

// =============================================================================
// DEMO: The C# Instinct (Everything by Value)
// =============================================================================

void demo_bad_api() {
    print_separator();
    std::cout << "DEMO: The C# Instinct (Pass by Value Everywhere)\n";
    print_separator();

    std::cout << R"(
  C# developers are used to passing reference types by handle.
  In C#, passing a List<T> to a method does NOT copy the list.

  In C++, passing std::vector<T> by value COPIES THE ENTIRE VECTOR.

  Watch the copy counters explode when using the bad_api functions:

)";

    reset_all_counters();
    auto batch = load_batch("/mock/telemetry.bin");
    print_counters("After load_batch (RVO applies)");

    const auto events_before_count = TelemetryEvent::copy_count;
    const auto batches_before_count = TelemetryBatch::copy_count;

    std::cout << "\n  Operations using BAD API (pass-by-value):\n";
    std::cout << "  " << std::string(65, '-') << '\n';

    // BAD: count_events copies the entire batch just to return a number
    [[maybe_unused]] auto count = bad_api::count_events_bad(batch);
    print_counter_delta("count_events_bad(batch)", events_before_count, batches_before_count);

    const auto events_after_count = TelemetryEvent::copy_count;
    const auto batches_after_count = TelemetryBatch::copy_count;

    // BAD: normalize_timestamps copies the batch, modifies the copy, original unchanged
    bad_api::normalize_timestamps_bad(batch, std::chrono::seconds{3600});
    print_counter_delta("normalize_timestamps_bad(batch, offset)",
                        events_after_count, batches_after_count);

    // BAD: append_batch copies BOTH batches
    auto batch2 = load_batch("/mock/telemetry2.bin");
    const auto events_before_append = TelemetryEvent::copy_count;
    const auto batches_before_append = TelemetryBatch::copy_count;

    [[maybe_unused]] auto merged_bad = bad_api::append_batch_bad(batch, batch2);
    print_counter_delta("append_batch_bad(batch, batch2)",
                        events_before_append, batches_before_append);

    print_counters("TOTAL after bad_api usage");

    std::cout << R"(
  PROBLEM: Each "by value" parameter triggered a FULL COPY of the batch.
  With 1000 events per batch, that's thousands of unnecessary copies.

  In a real system with 100,000 events, this would be:
    - Millions of bytes copied
    - Thousands of heap allocations
    - Measurable latency (milliseconds to seconds)

)";
}

// =============================================================================
// DEMO: Correct Parameter Passing (const T&, T&, T&&)
// =============================================================================

void demo_good_api() {
    print_separator();
    std::cout << "DEMO: Correct Parameter Passing (const T&, T&, T&&)\n";
    print_separator();

    std::cout << R"(
  Now let's use the CORRECT API with proper parameter modes:
    - const T&  for read-only access (no copy)
    - T&        for in-place mutation (no copy)
    - T by value for sink parameters, with std::move at call site

)";

    reset_all_counters();
    auto batch = load_batch("/mock/telemetry.bin");
    print_counters("After load_batch");

    std::cout << "\n  Operations using GOOD API (proper parameter modes):\n";
    std::cout << "  " << std::string(65, '-') << '\n';

    auto events_before = TelemetryEvent::copy_count;
    auto batches_before = TelemetryBatch::copy_count;

    // GOOD: filter_errors takes const TelemetryBatch& — no copy!
    auto errors = filter_errors(batch);
    print_counter_delta("filter_errors(batch) [const T&]", events_before, batches_before);
    std::cout << "    -> Found " << errors.size() << " error events (returned as pointers, not copies)\n";

    events_before = TelemetryEvent::copy_count;
    batches_before = TelemetryBatch::copy_count;

    // GOOD: normalize_timestamps takes TelemetryBatch& — modifies in place, no copy!
    normalize_timestamps(batch, std::chrono::seconds{3600});
    print_counter_delta("normalize_timestamps(batch, offset) [T&]", events_before, batches_before);

    events_before = TelemetryEvent::copy_count;
    batches_before = TelemetryBatch::copy_count;

    // GOOD: append_batch with const T& for source
    auto batch2 = load_batch("/mock/telemetry2.bin");
    events_before = TelemetryEvent::copy_count;
    batches_before = TelemetryBatch::copy_count;

    append_batch(batch, batch2);  // destination by &, source by const&
    print_counter_delta("append_batch(batch, batch2) [T&, const T&]", events_before, batches_before);
    std::cout << "    -> Note: Events ARE copied from source (unavoidable when source is const)\n";

    print_counters("TOTAL after good_api usage");

    std::cout << R"(
  RESULT: Dramatic reduction in copies!
    - filter_errors: ZERO copies (const T& parameter, pointer return)
    - normalize_timestamps: ZERO copies (T& parameter, in-place mutation)
    - append_batch: Events copied from source (necessary), but NO batch copies

)";
}

// =============================================================================
// DEMO: Move Semantics with merge_batches
// =============================================================================

void demo_move_semantics() {
    print_separator();
    std::cout << "DEMO: Move Semantics with Sink Parameters\n";
    print_separator();

    std::cout << R"(
  merge_batches(TelemetryBatch left, TelemetryBatch right)

  Takes parameters BY VALUE — but the caller controls the cost:
    - Pass lvalue: parameter is COPY-constructed (expensive)
    - Pass rvalue via std::move: parameter is MOVE-constructed (cheap!)

)";

    // Scenario 1: Passing lvalues (copies occur)
    {
        std::cout << "  Scenario 1: Passing lvalues (caller wants to keep originals)\n";
        std::cout << "  " << std::string(60, '-') << '\n';

        reset_all_counters();
        auto a = load_batch("/mock/batch_a.bin");
        auto b = load_batch("/mock/batch_b.bin");

        const auto events_before = TelemetryEvent::copy_count;
        const auto batches_before = TelemetryBatch::copy_count;

        // Pass by lvalue — both batches are COPIED into the function
        auto merged = merge_batches(a, b);

        print_counter_delta("merge_batches(a, b) [lvalues -> copies]",
                            events_before, batches_before);

        std::cout << "    -> Merged batch has " << merged.events.size() << " events\n";
        std::cout << "    -> Original 'a' still has " << a.events.size() << " events (preserved)\n";
        std::cout << '\n';
    }

    // Scenario 2: Passing rvalues via std::move (no copies!)
    {
        std::cout << "  Scenario 2: Passing rvalues via std::move (caller transfers ownership)\n";
        std::cout << "  " << std::string(60, '-') << '\n';

        reset_all_counters();
        auto a = load_batch("/mock/batch_a.bin");
        auto b = load_batch("/mock/batch_b.bin");

        const auto events_before = TelemetryEvent::copy_count;
        const auto batches_before = TelemetryBatch::copy_count;

        // Pass by rvalue — both batches are MOVED into the function (cheap!)
        auto merged = merge_batches(std::move(a), std::move(b));

        print_counter_delta("merge_batches(std::move(a), std::move(b)) [rvalues -> moves]",
                            events_before, batches_before);

        std::cout << "    -> Merged batch has " << merged.events.size() << " events\n";
        std::cout << "    -> Original 'a' now has " << a.events.size() << " events (moved-from)\n";
        std::cout << '\n';
    }

    std::cout << R"(
  KEY INSIGHT: By-value sink parameters give the CALLER the choice:
    - Keep the original? Pass normally. Pay the copy cost.
    - Done with the original? Use std::move(). Pay only the move cost.

  This is more flexible than:
    - const T&: Forces internal copy if you need to own the data
    - T&&: Only accepts rvalues, awkward for lvalues

)";
}

// =============================================================================
// DEMO: The Silent Bug (Mutation of a Copy)
// =============================================================================

void demo_silent_bug() {
    print_separator();
    std::cout << "DEMO: The Silent Bug (Mutating a Copy)\n";
    print_separator();

    std::cout << R"(
  This is the most insidious trap for C# developers.

  In C#:
    void UpdateBatch(TelemetryBatch batch) {
        batch.Events[0].Message = "Modified";  // Modifies the original!
    }

  In C++:
    void update_batch(TelemetryBatch batch) {
        batch.events[0].message = "Modified";  // Modifies a COPY!
    }

  The C# code works because batch is a reference. The C++ code silently fails
  because batch is a copy. Let's demonstrate:

)";

    reset_all_counters();
    auto batch = load_batch("/mock/telemetry.bin");

    std::cout << "  Before bad_api::normalize_timestamps_bad:\n";
    std::cout << "    batch.events[0].timestamp = "
              << batch.events[0].timestamp.time_since_epoch().count() << '\n';

    // This modifies a COPY — the caller's batch is unchanged!
    bad_api::normalize_timestamps_bad(batch, std::chrono::seconds{9999});

    std::cout << "  After bad_api::normalize_timestamps_bad:\n";
    std::cout << "    batch.events[0].timestamp = "
              << batch.events[0].timestamp.time_since_epoch().count() << '\n';
    std::cout << "    (Unchanged! The function modified a copy.)\n\n";

    // Now with the correct API
    std::cout << "  Now with the correct API (T& parameter):\n";
    std::cout << "  Before normalize_timestamps:\n";
    std::cout << "    batch.events[0].timestamp = "
              << batch.events[0].timestamp.time_since_epoch().count() << '\n';

    // This modifies the ACTUAL batch
    normalize_timestamps(batch, std::chrono::seconds{9999});

    std::cout << "  After normalize_timestamps:\n";
    std::cout << "    batch.events[0].timestamp = "
              << batch.events[0].timestamp.time_since_epoch().count() << '\n';
    std::cout << "    (Changed! The function modified the caller's batch.)\n";

    std::cout << R"(
  THE BUG: Pass-by-value creates a copy. Changes to the copy are invisible
  to the caller. The code compiles. The code runs. It just does nothing.

  THE FIX: Use T& when the function must modify the caller's object.
  The & makes the intent explicit and prevents the accidental copy.

)";
}

// =============================================================================
// SUMMARY: The Four Modes Cheat Sheet
// =============================================================================

void print_cheat_sheet() {
    print_separator();
    std::cout << "SUMMARY: The Four Parameter Modes\n";
    print_separator();

    std::cout << R"(
  +----------+------------------------+------------------------------------------+
  | Mode     | Meaning                | When to Use                              |
  +----------+------------------------+------------------------------------------+
  | T        | Copy the object        | Small types, or "sink" parameters        |
  | const T& | Borrow immutably       | DEFAULT for large read-only parameters   |
  | T&       | Borrow mutably         | When you MUST modify the caller's object |
  | T&&      | Take ownership / move  | When caller is giving up the object      |
  +----------+------------------------+------------------------------------------+

  RULES OF THUMB:

  1. DEFAULT to const T& for large user-defined types
  2. Use T for small, trivially copyable types (int, double, std::chrono::seconds)
  3. Use T& when the function MUST mutate the caller's object
  4. Use T (by value) for sink parameters — let caller choose copy vs move
  5. Use T&& rarely — mainly for perfect forwarding or explicit move-only APIs

  THE C# BRIDGE:

  In C#, reference types are passed by reference-handle by default.
  In C++, EVERYTHING is pass-by-value by default.

  When you write: void process(TelemetryBatch batch)
  You are saying: "Copy the entire batch into this function."

  When you write: void process(const TelemetryBatch& batch)
  You are saying: "Borrow the batch. No copy. No modification."

  The signature IS the contract. Make it explicit.

)";
}

// =============================================================================
// MAIN
// =============================================================================

int main() {
    std::cout << "=== Cpptude #003: Parameter Passing ===\n";
    std::cout << "Primary KAP: Value, const&, &, and && parameter modes\n\n";

    demo_memory_layout();
    demo_bad_api();
    demo_good_api();
    demo_move_semantics();
    demo_silent_bug();
    print_cheat_sheet();

    print_separator();
    std::cout << "Key Insight: Correct signatures communicate intent AND prevent copies.\n";
    std::cout << "The copy counters prove it.\n";
    print_separator();

    return 0;
}
