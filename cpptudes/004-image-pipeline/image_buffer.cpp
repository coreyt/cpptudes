// image_buffer.cpp — Cpptude #004: Image Pipeline — Move Semantics Implementation
//
// This file implements the ImageBuffer class and pipeline functions.
// Key learning points are marked with educational comments.
//
// BUILD:
//   g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic image_buffer.cpp main.cpp -o image_pipeline
//
// BUILD WITH SANITIZERS:
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -Wall -Wextra image_buffer.cpp main.cpp -o image_pipeline

#include "image_buffer.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>

namespace cpptude {

// =============================================================================
// ImageBuffer Implementation
// =============================================================================

// Primary constructor: allocates pixel buffer.
ImageBuffer::ImageBuffer(std::int32_t width, std::int32_t height)
    : width_{width}
    , height_{height}
    , pixels_{nullptr}
{
    if (width_ > 0 && height_ > 0) {
        const auto byte_count = static_cast<std::size_t>(width_) *
                                static_cast<std::size_t>(height_) * 4;
        pixels_ = new std::byte[byte_count];

        // Initialize to zero (transparent black) for deterministic behavior.
        std::memset(pixels_, 0, byte_count);
    }

    std::cerr << "[ImageBuffer] Constructed: " << width_ << "x" << height_
              << " (" << size() << " bytes)\n";
}

// Destructor: frees the pixel buffer.
// This is the core RAII guarantee: cleanup happens at scope exit.
ImageBuffer::~ImageBuffer() {
    std::cerr << "[ImageBuffer] Destroyed: " << width_ << "x" << height_;
    if (pixels_ != nullptr) {
        std::cerr << " (freeing " << size() << " bytes)";
        delete[] pixels_;
        pixels_ = nullptr;  // Not strictly necessary, but good hygiene.
    } else {
        std::cerr << " (empty/moved-from)";
    }
    std::cerr << '\n';
}

// -----------------------------------------------------------------------------
// COPY OPERATIONS — EXPENSIVE
// -----------------------------------------------------------------------------

// Copy constructor: deep copy of all pixel data.
//
// This is EXPENSIVE. For a 1920x1080 image with 4 bytes per pixel:
//   1920 * 1080 * 4 = 8,294,400 bytes = 8MB of memcpy
//
// The copy_count counter lets you see how many times this happens.
ImageBuffer::ImageBuffer(const ImageBuffer& other)
    : width_{other.width_}
    , height_{other.height_}
    , pixels_{nullptr}
{
    if (other.pixels_ != nullptr) {
        const auto byte_count = static_cast<std::size_t>(width_) *
                                static_cast<std::size_t>(height_) * 4;
        pixels_ = new std::byte[byte_count];

        // THE EXPENSIVE PART: copying all pixel data byte-by-byte.
        std::memcpy(pixels_, other.pixels_, byte_count);
    }

    ++copy_count;

    std::cerr << "[ImageBuffer] COPY constructor: " << width_ << "x" << height_
              << " (" << size() << " bytes copied) — total copies: "
              << copy_count << '\n';
}

// Copy assignment: replace contents with a deep copy.
//
// Uses copy-and-swap idiom for strong exception safety:
//   1. Create a temporary copy of 'other' (might throw if allocation fails)
//   2. Swap our internals with the temporary (noexcept)
//   3. Temporary destructor cleans up our old data
//
// If step 1 throws, 'this' is unchanged — strong exception guarantee.
ImageBuffer& ImageBuffer::operator=(const ImageBuffer& other) {
    if (this != &other) {
        // Step 1: Create temporary copy (might throw)
        ImageBuffer temp{other};

        // Step 2: Swap with temporary (noexcept)
        swap(temp);

        // Step 3: temp destructor runs here, freeing our old pixels
    }
    // Note: copy_count was already incremented in temp's copy constructor.
    return *this;
}

// -----------------------------------------------------------------------------
// MOVE OPERATIONS — CHEAP (and noexcept!)
// -----------------------------------------------------------------------------

// Move constructor: steal the source's resources.
//
// This is CHEAP: just copy 3 integers and null out the source.
// No allocation. No memcpy. O(1) regardless of image size.
//
// The source is left in a "valid but empty" state:
//   - pixels_ == nullptr (safe to delete[] in destructor)
//   - width_ == 0, height_ == 0 (consistent dimensions)
//
// noexcept is CRITICAL: std::vector::push_back and other STL operations
// will only use move semantics if the move constructor is noexcept.
// If it might throw, they fall back to copying for exception safety.
ImageBuffer::ImageBuffer(ImageBuffer&& other) noexcept
    : width_{other.width_}
    , height_{other.height_}
    , pixels_{other.pixels_}
{
    // Leave source in valid-but-empty state.
    other.width_ = 0;
    other.height_ = 0;
    other.pixels_ = nullptr;

    ++move_count;

    std::cerr << "[ImageBuffer] MOVE constructor: " << width_ << "x" << height_
              << " (pointer stolen, zero bytes copied) — total moves: "
              << move_count << '\n';
}

// Move assignment: replace contents by stealing from source.
//
// Steps:
//   1. Free our current resources (delete[] old pixels)
//   2. Steal source's resources
//   3. Leave source in valid-but-empty state
//
// noexcept because we only:
//   - delete[] (which is noexcept)
//   - copy integers (noexcept)
//   - set pointers to nullptr (noexcept)
ImageBuffer& ImageBuffer::operator=(ImageBuffer&& other) noexcept {
    if (this != &other) {
        // Step 1: Free our current resources.
        delete[] pixels_;

        // Step 2: Steal source's resources.
        width_ = other.width_;
        height_ = other.height_;
        pixels_ = other.pixels_;

        // Step 3: Leave source in valid-but-empty state.
        other.width_ = 0;
        other.height_ = 0;
        other.pixels_ = nullptr;

        ++move_count;

        std::cerr << "[ImageBuffer] MOVE assignment: " << width_ << "x" << height_
                  << " (pointer stolen) — total moves: " << move_count << '\n';
    }
    return *this;
}

// -----------------------------------------------------------------------------
// SWAP
// -----------------------------------------------------------------------------

// Swap internals with another ImageBuffer.
// This is the building block for copy-and-swap and for std::swap.
// noexcept because we only swap primitive types.
void ImageBuffer::swap(ImageBuffer& other) noexcept {
    std::swap(width_, other.width_);
    std::swap(height_, other.height_);
    std::swap(pixels_, other.pixels_);
}

// -----------------------------------------------------------------------------
// ACCESSORS
// -----------------------------------------------------------------------------

std::size_t ImageBuffer::size() const noexcept {
    return static_cast<std::size_t>(width_) *
           static_cast<std::size_t>(height_) * 4;
}

// =============================================================================
// PIPELINE FUNCTIONS
// =============================================================================
//
// These functions demonstrate how move semantics work in a processing pipeline.
// Each function takes ImageBuffer by value, which allows:
//   - Copying if caller passes an lvalue (expensive, but preserves original)
//   - Moving if caller passes std::move(lvalue) (cheap, consumes original)
//   - Zero-copy if caller passes a temporary (NRVO/copy elision)

// Load an image from disk (simulated).
// In a real application, this would read PNG/JPEG data and decode it.
// We simulate this by filling the buffer with pseudo-random pixel data.
[[nodiscard]] ImageBuffer load_image(const std::filesystem::path& path) {
    std::cerr << "\n[Pipeline] load_image(\"" << path.string() << "\")\n";

    // Simulate loading a 640x480 image (1.2MB) for reasonable demo output.
    // In production, dimensions would come from the file header.
    constexpr std::int32_t width{640};
    constexpr std::int32_t height{480};

    ImageBuffer buffer{width, height};

    // Fill with pseudo-random pixel data (simulating decoded image).
    // Using a fixed seed for reproducible output.
    auto* pixels = buffer.data();
    std::mt19937 rng{42};  // Fixed seed for reproducibility
    std::uniform_int_distribution<int> dist{0, 255};

    for (std::size_t i = 0; i < buffer.size(); i += 4) {
        pixels[i + 0] = static_cast<std::byte>(dist(rng));  // R
        pixels[i + 1] = static_cast<std::byte>(dist(rng));  // G
        pixels[i + 2] = static_cast<std::byte>(dist(rng));  // B
        pixels[i + 3] = static_cast<std::byte>(255);        // A (opaque)
    }

    std::cerr << "[Pipeline] Loaded " << width << "x" << height
              << " image (" << buffer.size() << " bytes)\n";

    // NRVO (Named Return Value Optimization): The compiler may construct
    // 'buffer' directly in the caller's return slot, avoiding any copy or move.
    // If NRVO doesn't apply, the return triggers a move (not a copy) because
    // 'buffer' is a local variable about to go out of scope.
    return buffer;
}

// Convert image to grayscale (simulated).
// Takes ownership of the input buffer by accepting it by value.
[[nodiscard]] ImageBuffer to_grayscale(ImageBuffer image) {
    std::cerr << "\n[Pipeline] to_grayscale()\n";

    if (image.empty()) {
        std::cerr << "[Pipeline] Warning: empty input to to_grayscale()\n";
        return image;
    }

    // Convert RGBA to grayscale using luminance formula:
    //   gray = 0.299*R + 0.587*G + 0.114*B
    // This weights green heavily because human eyes are most sensitive to it.
    auto* pixels = image.data();
    const auto pixel_count = image.size() / 4;

    for (std::size_t i = 0; i < pixel_count; ++i) {
        const auto r = static_cast<int>(pixels[i * 4 + 0]);
        const auto g = static_cast<int>(pixels[i * 4 + 1]);
        const auto b = static_cast<int>(pixels[i * 4 + 2]);

        // Integer approximation of luminance formula to avoid floating point.
        const auto gray = static_cast<std::byte>((r * 77 + g * 150 + b * 29) >> 8);

        pixels[i * 4 + 0] = gray;  // R
        pixels[i * 4 + 1] = gray;  // G
        pixels[i * 4 + 2] = gray;  // B
        // Alpha unchanged
    }

    std::cerr << "[Pipeline] Converted " << image.width() << "x" << image.height()
              << " to grayscale\n";

    // We modified 'image' in place and return it.
    // Since 'image' is a function parameter about to go out of scope,
    // this triggers a move (not copy) on return.
    return image;
}

// Apply a blur filter (simulated).
// Takes ownership of the input buffer by accepting it by value.
[[nodiscard]] ImageBuffer blur(ImageBuffer image, int radius) {
    std::cerr << "\n[Pipeline] blur(radius=" << radius << ")\n";

    if (image.empty()) {
        std::cerr << "[Pipeline] Warning: empty input to blur()\n";
        return image;
    }

    // A real blur would use a convolution kernel. For this educational demo,
    // we simulate the blur by slightly reducing contrast (averaging with gray).
    // This avoids the complexity of boundary handling while still "doing work."
    auto* pixels = image.data();
    const auto pixel_count = image.size() / 4;

    // Simulate blur intensity based on radius.
    const auto blend = std::min(radius, 10) * 10;  // 0-100 range

    for (std::size_t i = 0; i < pixel_count; ++i) {
        auto r = static_cast<int>(pixels[i * 4 + 0]);
        auto g = static_cast<int>(pixels[i * 4 + 1]);
        auto b = static_cast<int>(pixels[i * 4 + 2]);

        // Blend toward middle gray (128) based on blur strength.
        r = r + ((128 - r) * blend) / 100;
        g = g + ((128 - g) * blend) / 100;
        b = b + ((128 - b) * blend) / 100;

        pixels[i * 4 + 0] = static_cast<std::byte>(r);
        pixels[i * 4 + 1] = static_cast<std::byte>(g);
        pixels[i * 4 + 2] = static_cast<std::byte>(b);
    }

    std::cerr << "[Pipeline] Applied blur (radius " << radius << ") to "
              << image.width() << "x" << image.height() << " image\n";

    return image;
}

// Save image to disk (simulated).
// Takes a const reference: we only need to read the pixel data.
void save_image(const ImageBuffer& image, const std::filesystem::path& path) {
    std::cerr << "\n[Pipeline] save_image(\"" << path.string() << "\")\n";

    if (image.empty()) {
        std::cerr << "[Pipeline] Warning: saving empty image\n";
        return;
    }

    // In a real application, this would encode to PNG/JPEG and write to disk.
    // We simulate by just reporting what we would save.
    std::cerr << "[Pipeline] Saved " << image.width() << "x" << image.height()
              << " image (" << image.size() << " bytes) to " << path << '\n';
}

} // namespace cpptude
