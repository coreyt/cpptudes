// image_buffer.hpp — Cpptude #004: Image Pipeline — Move Semantics & Ownership Transfer
//
// PRIMARY KAP: Move Semantics (Rating 4 - Required)
//
// This header demonstrates:
//   - Resource-owning class with heap-allocated pixel buffer
//   - Rule of Five: destructor, copy ctor, copy assign, move ctor, move assign
//   - noexcept move operations (required for STL container optimizations)
//   - Moved-from state: valid but empty (safe to destroy or reassign)
//   - Static counters to measure copy vs. move overhead
//
// BUILD:
//   g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic image_buffer.cpp main.cpp -o image_pipeline
//
// BUILD WITH SANITIZERS:
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -Wall -Wextra image_buffer.cpp main.cpp -o image_pipeline

#ifndef CPPTUDE_IMAGE_BUFFER_HPP
#define CPPTUDE_IMAGE_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace cpptude {

// =============================================================================
// ImageBuffer — A resource-owning class for teaching move semantics
// =============================================================================
//
// C# INSTINCT: In C#, you would write:
//     byte[] pixels = new byte[width * height * 4];
//     var copy = pixels;  // Both references point to the SAME array.
//
// C++ REALITY: In C++, value semantics mean:
//     ImageBuffer copy = original;  // Deep copy: allocates new buffer, copies all pixels.
//
// Move semantics let you transfer ownership efficiently:
//     ImageBuffer moved = std::move(original);  // Cheap: just swaps pointers.
//     // original is now empty (valid but moved-from state).
//
// This class uses raw new[]/delete[] deliberately. In production code, you would
// use std::unique_ptr<std::byte[]>, but raw pointers make the ownership transfer
// visible and explicit for learning purposes.

class ImageBuffer {
public:
    // -------------------------------------------------------------------------
    // INSTRUMENTATION: Copy/Move counters
    // -------------------------------------------------------------------------
    // These counters let you SEE the cost difference between copying and moving.
    // A 4K image (3840x2160) with 4 bytes per pixel = 33MB per copy.
    // A move is 3 pointer/int swaps regardless of image size.

    static inline std::size_t copy_count{0};
    static inline std::size_t move_count{0};

    // Reset counters between demos for clarity.
    static void reset_counters() noexcept {
        copy_count = 0;
        move_count = 0;
    }

    // -------------------------------------------------------------------------
    // CONSTRUCTORS
    // -------------------------------------------------------------------------

    // Default constructor: creates an empty buffer.
    // Useful for declaring variables before assignment.
    ImageBuffer() noexcept = default;

    // Primary constructor: allocates pixel buffer.
    // 4 bytes per pixel: RGBA (red, green, blue, alpha).
    //
    // Note: We use explicit to prevent accidental implicit conversions.
    // Without explicit, code like: ImageBuffer buf = 1920; would compile.
    ImageBuffer(std::int32_t width, std::int32_t height);

    // -------------------------------------------------------------------------
    // DESTRUCTOR
    // -------------------------------------------------------------------------
    // The "RI" in RAII: Resource Initialization Is Acquisition (and cleanup).
    // The destructor frees the pixel buffer unconditionally at scope exit.

    ~ImageBuffer();

    // -------------------------------------------------------------------------
    // COPY OPERATIONS — EXPENSIVE
    // -------------------------------------------------------------------------
    // Copying duplicates the entire pixel buffer. For a 4K image, this is 33MB
    // of memcpy. The compiler cannot optimize this away.

    // Copy constructor: deep copy of pixel data.
    ImageBuffer(const ImageBuffer& other);

    // Copy assignment: replace contents with a deep copy.
    // Uses copy-and-swap idiom for exception safety.
    ImageBuffer& operator=(const ImageBuffer& other);

    // -------------------------------------------------------------------------
    // MOVE OPERATIONS — CHEAP (and noexcept!)
    // -------------------------------------------------------------------------
    // Moving transfers ownership by swapping pointers. This is O(1) regardless
    // of image size. The source object is left in a valid-but-empty state.
    //
    // noexcept is critical: if a move constructor might throw, the standard
    // library containers (std::vector::push_back, etc.) will fall back to
    // copying to preserve strong exception safety. Mark move ops noexcept
    // to enable the optimization.

    // Move constructor: steal the source's pixel buffer.
    ImageBuffer(ImageBuffer&& other) noexcept;

    // Move assignment: replace contents by stealing from source.
    ImageBuffer& operator=(ImageBuffer&& other) noexcept;

    // -------------------------------------------------------------------------
    // SWAP — Foundation for copy-and-swap idiom
    // -------------------------------------------------------------------------
    // noexcept because it only swaps primitive types.
    void swap(ImageBuffer& other) noexcept;

    // -------------------------------------------------------------------------
    // ACCESSORS
    // -------------------------------------------------------------------------
    // [[nodiscard]] warns if the caller ignores the return value.
    // For getters, ignoring the result is always a bug.

    [[nodiscard]] std::byte* data() noexcept { return pixels_; }
    [[nodiscard]] const std::byte* data() const noexcept { return pixels_; }

    [[nodiscard]] std::int32_t width() const noexcept { return width_; }
    [[nodiscard]] std::int32_t height() const noexcept { return height_; }

    // Total size in bytes: width * height * 4 (RGBA)
    [[nodiscard]] std::size_t size() const noexcept;

    // Check if this is a moved-from (empty) buffer.
    // A moved-from ImageBuffer has pixels_ == nullptr and dimensions == 0.
    [[nodiscard]] bool empty() const noexcept { return pixels_ == nullptr; }

private:
    std::int32_t width_{0};
    std::int32_t height_{0};
    std::byte* pixels_{nullptr};  // Heap-allocated: width * height * 4 bytes
};

// Non-member swap for ADL (Argument-Dependent Lookup)
inline void swap(ImageBuffer& a, ImageBuffer& b) noexcept {
    a.swap(b);
}

// =============================================================================
// PIPELINE FUNCTIONS
// =============================================================================
//
// These functions take ImageBuffer BY VALUE. This is intentional:
//   - If you pass an lvalue, it copies (expensive).
//   - If you pass std::move(lvalue), it moves (cheap).
//   - If you pass a temporary, it moves automatically (NRVO/copy elision).
//
// The return value is also by value, enabling NRVO (Named Return Value
// Optimization) or move semantics on the way out.

// Simulate loading an image from disk.
// Returns a new ImageBuffer with "loaded" pixel data.
[[nodiscard]] ImageBuffer load_image(const std::filesystem::path& path);

// Convert image to grayscale.
// Takes ownership of the input buffer, returns a new (or modified) buffer.
[[nodiscard]] ImageBuffer to_grayscale(ImageBuffer image);

// Apply a blur filter.
// Takes ownership of the input buffer, returns a new (or modified) buffer.
[[nodiscard]] ImageBuffer blur(ImageBuffer image, int radius);

// Save image to disk.
// Takes a const reference: reading the buffer does not require ownership.
void save_image(const ImageBuffer& image, const std::filesystem::path& path);

} // namespace cpptude

#endif // CPPTUDE_IMAGE_BUFFER_HPP
