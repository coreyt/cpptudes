// telemetry.cpp — Cpptude #003: Parameter Passing
//
// Implementation of the telemetry batch processing API.
// See telemetry.hpp for type definitions and detailed signature rationale.
//
// KEY LESSON: The parameter mode in each function signature is a DESIGN DECISION
// that communicates intent and controls copying. Compare the counters when using
// the correct API vs the bad_api to see the cost difference.
//
// BUILD:
//   cmake -B build -DCMAKE_BUILD_TYPE=Debug
//   cmake --build build
//   ./build/parameter_passing

#include "telemetry.hpp"

#include <algorithm>
#include <iterator>

namespace cpptude {

// =============================================================================
// load_batch — Factory function returning by value
// =============================================================================
//
// This is a mock implementation that creates sample data.
// In production, you would parse an actual file.
//
// RETURN BY VALUE: Modern C++ encourages returning by value because:
//   1. It's the clearest expression of "this function creates a new object"
//   2. RVO (Return Value Optimization) eliminates the copy
//   3. The caller gets clear ownership semantics
//
// Note: We could use std::expected<TelemetryBatch, Error> in C++23 for
// better error handling. For now, we keep it simple.

[[nodiscard]] TelemetryBatch load_batch(const std::filesystem::path& file_path) {
    TelemetryBatch batch;
    batch.source = file_path.string();

    // Create sample telemetry events
    // In production, you would read these from the actual file

    constexpr std::size_t sample_event_count = 1000;
    batch.events.reserve(sample_event_count);

    const auto base_time = std::chrono::system_clock::now();

    for (std::size_t i = 0; i < sample_event_count; ++i) {
        TelemetryEvent event;

        // Alternate between normal events and error events
        if (i % 10 == 0) {
            event.message = "ERROR: Connection timeout";
        } else if (i % 5 == 0) {
            event.message = "WARNING: High latency detected";
        } else {
            event.message = "INFO: Request processed successfully";
        }

        // Fill metrics with sample data
        for (std::size_t j = 0; j < event.metrics.size(); ++j) {
            event.metrics[j] = static_cast<double>(i * 16 + j) * 0.1;
        }

        // Create variable-size payload (simulating real telemetry)
        const std::size_t payload_size = 64 + (i % 128);
        event.payload.resize(payload_size);
        for (std::size_t j = 0; j < payload_size; ++j) {
            event.payload[j] = static_cast<std::byte>((i + j) % 256);
        }

        event.timestamp = base_time + std::chrono::milliseconds{i * 10};

        // emplace_back constructs in place, avoiding a copy
        // But we have a local 'event', so this is a move
        batch.events.push_back(std::move(event));
    }

    // RVO: The compiler constructs 'batch' directly in the caller's memory.
    // No copy of the batch happens here despite the "return by value."
    return batch;
}

// =============================================================================
// normalize_timestamps — Mutates batch in place
// =============================================================================
//
// PARAMETER MODES:
//   - TelemetryBatch& batch       → Mutable reference, we WILL modify caller's batch
//   - std::chrono::seconds offset → By value, it's a small trivially-copyable type
//
// The & without const is the signal: "This function modifies your object."
// When you call normalize_timestamps(myBatch, offset), you KNOW myBatch will change.

void normalize_timestamps(TelemetryBatch& batch, std::chrono::seconds offset) {
    // Iterate by reference to modify in place.
    // Using `auto&` not `auto` — the latter would copy each event!
    for (auto& event : batch.events) {
        event.timestamp += offset;
    }

    // No copies occurred. We modified the caller's batch directly.
    // If we had taken TelemetryBatch by value, we would have:
    //   1. Copied the entire batch on function entry
    //   2. Modified the copy
    //   3. Returned, leaving the caller's original unchanged
    //
    // That's a silent bug — the kind C# developers hit in C++ because they
    // expect reference-type behavior by default.
}

// =============================================================================
// filter_errors — Read-only access returning non-owning pointers
// =============================================================================
//
// PARAMETER MODE: const TelemetryBatch&
//   - const says: "I will not modify this batch"
//   - & says: "I will not copy this batch"
//
// RETURN TYPE: std::vector<const TelemetryEvent*>
//   - We return POINTERS to events, not copies of events
//   - const TelemetryEvent* means: "You can look but not touch"
//
// LIFETIME WARNING: The returned pointers are only valid while the batch
// exists and is not modified. This is a borrowing relationship.

[[nodiscard]] std::vector<const TelemetryEvent*> filter_errors(const TelemetryBatch& batch) {
    std::vector<const TelemetryEvent*> errors;

    // Reserve space based on heuristic (assume ~10% are errors)
    errors.reserve(batch.events.size() / 10);

    for (const auto& event : batch.events) {
        // Check if the message starts with "ERROR:"
        // Using starts_with (C++20) for clarity
        if (event.message.starts_with("ERROR:")) {
            // Store a pointer to the event, NOT a copy
            errors.push_back(&event);
        }
    }

    // No TelemetryEvent copies occurred!
    // We're returning lightweight pointers that reference the original batch.
    //
    // If we had returned std::vector<TelemetryEvent>, we would have copied
    // every error event — expensive for large events with payloads.
    return errors;
}

// =============================================================================
// append_batch — Mixed parameter modes
// =============================================================================
//
// PARAMETER MODES:
//   - TelemetryBatch& destination       → Mutable, we add events to it
//   - const TelemetryBatch& source      → Read-only, we copy events from it
//
// This function demonstrates that you can mix modes in one function.
// The signature clearly communicates: "destination changes, source doesn't."
//
// NOTE: We copy events from source. If you wanted to MOVE events, you would
// need a different signature like: append_batch(TelemetryBatch& dest, TelemetryBatch&& source)

void append_batch(TelemetryBatch& destination, const TelemetryBatch& source) {
    // Reserve space to avoid repeated reallocations
    destination.events.reserve(destination.events.size() + source.events.size());

    // Copy events from source to destination
    // This DOES copy each event (unavoidable when source is const)
    for (const auto& event : source.events) {
        destination.events.push_back(event);  // Copies the event
    }

    // Alternative using std::copy (same behavior, different style):
    // std::copy(source.events.begin(), source.events.end(),
    //           std::back_inserter(destination.events));

    // Update the source identifier
    if (!destination.source.empty()) {
        destination.source += " + ";
    }
    destination.source += source.source;
}

// =============================================================================
// merge_batches — By-value parameters (sink pattern)
// =============================================================================
//
// PARAMETER MODES: TelemetryBatch left, TelemetryBatch right (both by value)
//
// WHY BY VALUE?
//   This is the "sink parameter" pattern. The function conceptually consumes
//   its inputs to produce an output. By taking parameters by value:
//
//   1. If the caller passes an lvalue: the parameter is COPY-constructed
//   2. If the caller passes an rvalue (std::move): the parameter is MOVE-constructed
//
//   The caller controls the cost at the call site:
//     merge_batches(a, b)              // Copies a and b
//     merge_batches(std::move(a), b)   // Moves a, copies b
//     merge_batches(std::move(a), std::move(b))  // Moves both (cheapest)
//
// INSIDE THE FUNCTION: We can freely move from left and right because we own them.

[[nodiscard]] TelemetryBatch merge_batches(TelemetryBatch left, TelemetryBatch right) {
    TelemetryBatch result;

    // Reserve space for all events
    result.events.reserve(left.events.size() + right.events.size());

    // Move events from left (we own it, so this is safe)
    // std::move_iterator makes the range adapter that moves instead of copies
    std::move(left.events.begin(), left.events.end(),
              std::back_inserter(result.events));

    // Move events from right
    std::move(right.events.begin(), right.events.end(),
              std::back_inserter(result.events));

    // Create merged source identifier
    result.source = std::move(left.source) + " + " + std::move(right.source);

    // RVO applies here too
    return result;
}

// =============================================================================
// BAD API — The C# instinct (pass everything by value)
// =============================================================================
//
// These functions demonstrate what happens when you bring C# habits to C++.
// In C#, passing a List<T> passes the reference, not the list.
// In C++, passing std::vector<T> copies the ENTIRE vector.

namespace bad_api {

// BAD: Takes batch by value, triggering a full copy just to count events
[[nodiscard]] std::size_t count_events_bad(TelemetryBatch batch) {
    // The batch was ALREADY copied when we entered this function.
    // We paid the cost of copying thousands of events just to count them.
    return batch.events.size();
}

// GOOD alternative (not implemented here, just for reference):
// [[nodiscard]] std::size_t count_events_good(const TelemetryBatch& batch) {
//     return batch.events.size();  // Zero copies!
// }

// BAD: Takes batch by value, modifies the COPY, caller sees no change
void normalize_timestamps_bad(TelemetryBatch batch, std::chrono::seconds offset) {
    // This modifies our LOCAL COPY of the batch.
    // The caller's batch is unchanged!
    for (auto& event : batch.events) {
        event.timestamp += offset;
    }

    // When this function returns, our modified copy is destroyed.
    // The caller's batch still has the original timestamps.
    // This is a silent bug — the C# instinct leads you astray.
}

// GOOD alternative (not implemented here, just for reference):
// void normalize_timestamps_good(TelemetryBatch& batch, std::chrono::seconds offset) {
//     // Modifies the caller's batch directly. No copies.
// }

// BAD: Takes both batches by value, copying both unnecessarily
[[nodiscard]] TelemetryBatch append_batch_bad(TelemetryBatch destination, TelemetryBatch source) {
    // Both destination and source were copied on function entry.
    // We've already paid the cost of two full batch copies.

    destination.events.reserve(destination.events.size() + source.events.size());

    for (const auto& event : source.events) {
        destination.events.push_back(event);  // More copies!
    }

    destination.source += " + " + source.source;

    return destination;  // And another copy on return (maybe, if no RVO)
}

// GOOD alternative (the one in the main API):
// void append_batch(TelemetryBatch& destination, const TelemetryBatch& source);
//   - destination by &: modifies in place, no copy
//   - source by const&: read-only, no copy

} // namespace bad_api

} // namespace cpptude
