// telemetry.hpp — Cpptude #003: Parameter Passing
//
// PRIMARY KAP: Parameter Passing Modes (Value, const&, &, &&)
//
// This header demonstrates:
//   - The four parameter-passing modes in C++
//   - Instrumented copy/move counters to make costs VISIBLE
//   - How signature design communicates ownership and intent
//   - const correctness in function parameters
//
// C# BRIDGE:
//   In C#, reference types are passed by reference-handle by default.
//   Passing a List<T> to a method passes the reference, not a copy.
//   In C++, `T` means COPY the object. If you pass a TelemetryBatch by value,
//   you are copying every event, every string, every byte of payload.
//   This cpptude makes that cost impossible to ignore.
//
// BUILD:
//   cmake -B build -DCMAKE_BUILD_TYPE=Debug
//   cmake --build build
//   ./build/parameter_passing
//
// BUILD WITH SANITIZERS (Debug mode enables this automatically):
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -Wall -Wextra telemetry.cpp main.cpp -o parameter_passing

#ifndef CPPTUDE_TELEMETRY_HPP
#define CPPTUDE_TELEMETRY_HPP

#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace cpptude {

// =============================================================================
// TELEMETRY EVENT — The core data type
// =============================================================================
//
// This struct is intentionally large to make copy costs VISIBLE.
// Each TelemetryEvent contains:
//   - A string (heap allocation)
//   - 16 doubles (128 bytes of metrics)
//   - A vector of bytes (heap allocation)
//   - A timestamp
//
// When you copy a TelemetryEvent, you copy ALL of that data.
// When you move a TelemetryEvent, you transfer ownership of the heap
// allocations (the string and vector) with just a few pointer assignments.

struct TelemetryEvent {
    // =========================================================================
    // INSTRUMENTATION: Make copy/move costs visible
    // =========================================================================
    //
    // These static counters track every copy and move operation.
    // After running your code, check these counters to see the true cost
    // of your parameter-passing choices.
    //
    // `inline` allows definition in the header (C++17+).
    // Without inline, these would need to be defined in a .cpp file.

    static inline std::size_t copy_count = 0;
    static inline std::size_t move_count = 0;

    // Reset counters between tests
    static void reset_counters() noexcept {
        copy_count = 0;
        move_count = 0;
    }

    // =========================================================================
    // DATA MEMBERS
    // =========================================================================

    std::string message;                          // Diagnostic message (heap-allocated)
    std::array<double, 16> metrics{};             // Fixed-size metric buffer (128 bytes on stack)
    std::vector<std::byte> payload;               // Variable payload (heap-allocated)
    std::chrono::system_clock::time_point timestamp{};  // Event timestamp

    // =========================================================================
    // SPECIAL MEMBER FUNCTIONS (Rule of Five)
    // =========================================================================
    //
    // We define all five special members to instrument copy/move operations.
    // In production code, you would usually follow the Rule of Zero (let the
    // compiler generate these). Here, we need to count operations.

    // Default constructor
    TelemetryEvent() = default;

    // Copy constructor: EXPENSIVE operation
    // Copies the string (heap allocation + memcpy)
    // Copies the array (128 bytes memcpy)
    // Copies the vector (heap allocation + memcpy of all payload bytes)
    TelemetryEvent(const TelemetryEvent& other)
        : message{other.message}
        , metrics{other.metrics}
        , payload{other.payload}
        , timestamp{other.timestamp}
    {
        ++copy_count;
    }

    // Move constructor: CHEAP operation
    // Transfers ownership of string's heap buffer (just pointer assignment)
    // Copies the array (128 bytes — std::array is not a heap type)
    // Transfers ownership of vector's heap buffer (just pointer assignment)
    //
    // noexcept is critical: std::vector uses move only if it's noexcept,
    // otherwise it falls back to copy for exception safety.
    TelemetryEvent(TelemetryEvent&& other) noexcept
        : message{std::move(other.message)}
        , metrics{std::move(other.metrics)}     // std::array move is actually a copy
        , payload{std::move(other.payload)}
        , timestamp{other.timestamp}            // Trivially copyable
    {
        ++move_count;
    }

    // Copy assignment: EXPENSIVE
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

    // Move assignment: CHEAP
    // noexcept for the same reason as move constructor.
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

    // Destructor: default is fine (string and vector clean up themselves)
    ~TelemetryEvent() = default;
};

// =============================================================================
// TELEMETRY BATCH — A collection of events
// =============================================================================
//
// A batch contains thousands of events. Copying a batch means copying
// every single event, which means copying every string, every vector,
// every 128-byte metrics array. The cost is enormous.
//
// SIGNATURE DESIGN: When you see a function that takes TelemetryBatch,
// the parameter mode tells you:
//   - TelemetryBatch         → "I will copy this batch"
//   - const TelemetryBatch&  → "I will read this batch without copying"
//   - TelemetryBatch&        → "I will modify your batch in place"
//   - TelemetryBatch&&       → "I will take ownership of this batch"

struct TelemetryBatch {
    // Instrumentation counters (same pattern as TelemetryEvent)
    static inline std::size_t copy_count = 0;
    static inline std::size_t move_count = 0;

    static void reset_counters() noexcept {
        copy_count = 0;
        move_count = 0;
    }

    // Data members
    std::vector<TelemetryEvent> events;  // The actual telemetry data
    std::string source;                   // Origin identifier (e.g., server name)

    // Default constructor
    TelemetryBatch() = default;

    // Copy constructor: VERY EXPENSIVE
    // Copies the source string (heap allocation)
    // Copies the events vector (heap allocation)
    // Each event copy triggers TelemetryEvent's copy constructor
    //
    // For a batch with 10,000 events, this is 10,000+ heap allocations!
    TelemetryBatch(const TelemetryBatch& other)
        : events{other.events}
        , source{other.source}
    {
        ++copy_count;
    }

    // Move constructor: CHEAP
    // Transfers ownership of the entire events vector (just pointer swap)
    // Transfers ownership of the source string (just pointer swap)
    //
    // For a batch with 10,000 events, this is still just ~3 pointer operations!
    TelemetryBatch(TelemetryBatch&& other) noexcept
        : events{std::move(other.events)}
        , source{std::move(other.source)}
    {
        ++move_count;
    }

    // Copy assignment: VERY EXPENSIVE
    TelemetryBatch& operator=(const TelemetryBatch& other) {
        if (this != &other) {
            events = other.events;
            source = other.source;
            ++copy_count;
        }
        return *this;
    }

    // Move assignment: CHEAP
    TelemetryBatch& operator=(TelemetryBatch&& other) noexcept {
        if (this != &other) {
            events = std::move(other.events);
            source = std::move(other.source);
            ++move_count;
        }
        return *this;
    }

    ~TelemetryBatch() = default;
};

// =============================================================================
// RESET ALL COUNTERS (convenience function)
// =============================================================================

inline void reset_all_counters() noexcept {
    TelemetryEvent::reset_counters();
    TelemetryBatch::reset_counters();
}

// =============================================================================
// API FUNCTIONS — Each signature teaches a different parameter mode
// =============================================================================
//
// THE FOUR MODES:
//
// | Mode       | Meaning              | Typical Use                              |
// |------------|----------------------|------------------------------------------|
// | T          | Copy the object      | Small types, or when you need a copy     |
// | const T&   | Borrow immutably     | Default for large read-only parameters   |
// | T&         | Borrow mutably       | When you must modify the caller's object |
// | T&&        | Take ownership/move  | When caller is giving up the object      |
//
// DEFAULT RULE: For large user-defined types, default to `const T&`.
//               Use `T&` only when the function MUST mutate the caller.
//               Use `T` when you need an owned copy (often for sink parameters).
//               Use `T&&` when you explicitly want to consume the argument.

// -----------------------------------------------------------------------------
// load_batch: Returns a new batch by value
// -----------------------------------------------------------------------------
//
// SIGNATURE: TelemetryBatch load_batch(const std::filesystem::path& file_path)
//
// WHY RETURN BY VALUE?
//   The caller receives a NEW owned batch. This is the natural way to express
//   "this function creates and returns an object."
//
// WHY const path&?
//   std::filesystem::path can be large (contains a string). We only need to
//   read it, so const reference avoids the copy.
//
// RETURN VALUE OPTIMIZATION (RVO):
//   Modern compilers eliminate the "copy" when returning by value. The batch
//   is constructed directly in the caller's memory. You get value semantics
//   with reference performance.
//
// NOTE: This is a mock implementation for demonstration. In a real system,
//       you would actually parse a file.

[[nodiscard]] TelemetryBatch load_batch(const std::filesystem::path& file_path);

// -----------------------------------------------------------------------------
// normalize_timestamps: Modifies batch IN PLACE
// -----------------------------------------------------------------------------
//
// SIGNATURE: void normalize_timestamps(TelemetryBatch& batch, std::chrono::seconds offset)
//
// WHY TelemetryBatch& (mutable reference)?
//   The function MUST modify the caller's batch. A mutable reference makes
//   this explicit: when you see `&` without `const`, you know the function
//   will change your object.
//
// WHY std::chrono::seconds by value?
//   std::chrono::seconds is a small, trivially copyable type (just an int64_t).
//   Passing small types by value is actually faster than by reference because:
//   1. No indirection (no pointer to dereference)
//   2. Better register usage
//   3. No aliasing concerns for the optimizer
//
// RULE OF THUMB: Pass by value for types <= 2 * sizeof(void*) that are
//                trivially copyable. Pass by const& for everything else.

void normalize_timestamps(TelemetryBatch& batch, std::chrono::seconds offset);

// -----------------------------------------------------------------------------
// filter_errors: Read-only access, returns non-owning pointers
// -----------------------------------------------------------------------------
//
// SIGNATURE: std::vector<const TelemetryEvent*> filter_errors(const TelemetryBatch& batch)
//
// WHY const TelemetryBatch&?
//   Filtering is a read-only operation. The batch should not be modified.
//   const reference says: "I will read this, not copy it, not modify it."
//
// WHY return std::vector<const TelemetryEvent*>?
//   We want to return references to events WITHOUT copying them.
//   Returning `std::vector<TelemetryEvent>` would copy every error event.
//   Returning `std::vector<const TelemetryEvent*>` returns pointers that
//   point into the original batch — no copies, just lightweight pointers.
//
// DANGER: The returned pointers are only valid as long as the batch exists
//         and is not modified. This is a borrowing relationship, not ownership.
//         In production, you might use std::span or document the lifetime clearly.
//
// [[nodiscard]] because ignoring the return value is almost certainly a bug.

[[nodiscard]] std::vector<const TelemetryEvent*> filter_errors(const TelemetryBatch& batch);

// -----------------------------------------------------------------------------
// append_batch: Mutates destination, reads from source
// -----------------------------------------------------------------------------
//
// SIGNATURE: void append_batch(TelemetryBatch& destination, const TelemetryBatch& source)
//
// This signature demonstrates MIXED modes:
//   - destination is T&  (mutable) — we will add events to it
//   - source is const T& (read-only) — we will copy events from it
//
// The signature tells the whole story:
//   "Append events FROM source TO destination. Destination changes, source doesn't."

void append_batch(TelemetryBatch& destination, const TelemetryBatch& source);

// -----------------------------------------------------------------------------
// merge_batches: Takes ownership via by-value parameters
// -----------------------------------------------------------------------------
//
// SIGNATURE: TelemetryBatch merge_batches(TelemetryBatch left, TelemetryBatch right)
//
// WHY PASS BY VALUE?
//   This is the "sink parameter" idiom. The function needs to own the data.
//   By taking parameters by value, the caller decides whether to copy or move:
//
//   TelemetryBatch a = ...;
//   TelemetryBatch b = ...;
//
//   // Caller wants to keep a and b: they get copied into merge_batches
//   auto merged1 = merge_batches(a, b);
//
//   // Caller is done with a and b: they get moved (cheap!)
//   auto merged2 = merge_batches(std::move(a), std::move(b));
//
// This pattern is more flexible than:
//   - Taking const T& and copying internally (always copies)
//   - Taking T&& (only accepts rvalues, awkward for lvalues)
//
// The by-value approach lets the caller choose copy vs move at the call site.

[[nodiscard]] TelemetryBatch merge_batches(TelemetryBatch left, TelemetryBatch right);

// =============================================================================
// BAD API — Demonstrates the C# instinct (pass everything by value)
// =============================================================================
//
// These functions have INCORRECT signatures that cause unnecessary copies.
// They exist to demonstrate the cost of the C# instinct in C++.
// Compare counter values before and after refactoring.

namespace bad_api {

// BAD: Copies the entire batch just to read it
[[nodiscard]] std::size_t count_events_bad(TelemetryBatch batch);

// BAD: Copies the batch, modifies the copy, caller sees no change
void normalize_timestamps_bad(TelemetryBatch batch, std::chrono::seconds offset);

// BAD: Copies both batches unnecessarily
[[nodiscard]] TelemetryBatch append_batch_bad(TelemetryBatch destination, TelemetryBatch source);

} // namespace bad_api

} // namespace cpptude

#endif // CPPTUDE_TELEMETRY_HPP
