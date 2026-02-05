// main.cpp — Cpptude #004: Image Pipeline Test Harness
//
// This file demonstrates move semantics through three scenarios:
//   1. Pipeline WITHOUT std::move (expensive copies)
//   2. Pipeline WITH std::move (zero copies)
//   3. Moved-from state examination
//
// Watch the copy_count and move_count counters to see the difference!
//
// BUILD:
//   cmake -B build -DCMAKE_BUILD_TYPE=Debug
//   cmake --build build
//   ./build/image_pipeline
//
// Or manually:
//   g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic image_buffer.cpp main.cpp -o image_pipeline

#include "image_buffer.hpp"

#include <cstdint>
#include <iostream>
#include <iomanip>

using namespace cpptude;

// =============================================================================
// HELPER: Print current counter state
// =============================================================================

void print_counters(const char* label) {
    std::cout << "\n  " << label << '\n';
    std::cout << "    Copies: " << ImageBuffer::copy_count << '\n';
    std::cout << "    Moves:  " << ImageBuffer::move_count << '\n';
}

// =============================================================================
// HELPER: Print memory layout of an ImageBuffer
// =============================================================================

void print_memory_layout(const char* name, const ImageBuffer& buffer) {
    std::cout << "  " << name << ":\n";
    std::cout << "    Address of ImageBuffer object: " << &buffer << '\n';
    std::cout << "    Address of pixels_ pointer:    " << static_cast<const void*>(buffer.data()) << '\n';
    std::cout << "    Dimensions: " << buffer.width() << "x" << buffer.height() << '\n';
    std::cout << "    Size: " << buffer.size() << " bytes\n";
    std::cout << "    Empty (moved-from): " << (buffer.empty() ? "yes" : "no") << '\n';
}

// =============================================================================
// DEMO 1: Pipeline WITHOUT std::move — Lots of Copies
// =============================================================================

void demo_without_move() {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "DEMO 1: Pipeline WITHOUT std::move (The Expensive Way)\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "This demonstrates what happens when you pass ImageBuffer by value\n";
    std::cout << "without using std::move. Each pipeline stage copies the entire\n";
    std::cout << "pixel buffer — megabytes of data!\n";
    std::cout << "\n";
    std::cout << "C# INSTINCT: In C#, passing an object just copies a reference.\n";
    std::cout << "             Both variables point to the same data. No cost.\n";
    std::cout << "\n";
    std::cout << "C++ REALITY: Passing by value COPIES the entire object.\n";
    std::cout << "             For ImageBuffer, that means allocating a new buffer\n";
    std::cout << "             and memcpy'ing all the pixels.\n";
    std::cout << "\n";

    ImageBuffer::reset_counters();
    print_counters("Initial state:");

    std::cout << "\n--- Starting pipeline (no std::move) ---\n";

    // Load the image
    ImageBuffer img = load_image("input.png");
    print_counters("After load_image:");

    // Convert to grayscale — WITHOUT std::move, this COPIES img
    // The copy constructor runs because 'img' is an lvalue.
    ImageBuffer gray = to_grayscale(img);  // COPY HERE!
    print_counters("After to_grayscale (img copied!):");

    // Apply blur — WITHOUT std::move, this COPIES gray
    ImageBuffer blurred = blur(gray, 5);   // COPY HERE!
    print_counters("After blur (gray copied!):");

    // Save — const reference, no copy needed
    save_image(blurred, "output.png");
    print_counters("After save_image (const ref, no copy):");

    std::cout << "\n";
    std::cout << "--- Pipeline complete ---\n";
    std::cout << "\n";
    std::cout << "RESULT: " << ImageBuffer::copy_count << " copies, "
              << ImageBuffer::move_count << " moves\n";
    std::cout << "\n";
    std::cout << "PROBLEM: Each copy duplicated the entire pixel buffer!\n";
    std::cout << "         For a 640x480 image, each copy is ~1.2MB of memcpy.\n";
    std::cout << "         For a 4K image (3840x2160), each copy is ~33MB.\n";
    std::cout << "\n";

    // All three buffers are still valid — we made independent copies
    std::cout << "All buffers are still valid (independent copies):\n";
    std::cout << "  img.empty():     " << (img.empty() ? "yes" : "no") << '\n';
    std::cout << "  gray.empty():    " << (gray.empty() ? "yes" : "no") << '\n';
    std::cout << "  blurred.empty(): " << (blurred.empty() ? "yes" : "no") << '\n';
}

// =============================================================================
// DEMO 2: Pipeline WITH std::move — Zero Copies
// =============================================================================

void demo_with_move() {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "DEMO 2: Pipeline WITH std::move (The Efficient Way)\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "This demonstrates the power of move semantics. By using std::move,\n";
    std::cout << "we transfer ownership instead of copying. The pixel buffer pointer\n";
    std::cout << "is simply handed from one ImageBuffer to another.\n";
    std::cout << "\n";
    std::cout << "KEY INSIGHT: std::move() is NOT a function that moves anything!\n";
    std::cout << "             It is a CAST to an rvalue reference (T&&).\n";
    std::cout << "             The actual move happens in the move constructor/assignment.\n";
    std::cout << "\n";

    ImageBuffer::reset_counters();
    print_counters("Initial state:");

    std::cout << "\n--- Starting pipeline (with std::move) ---\n";

    // Load the image
    ImageBuffer img = load_image("input.png");
    print_counters("After load_image:");

    // Convert to grayscale — WITH std::move, this MOVES img
    // std::move(img) casts 'img' to ImageBuffer&&, which binds to the
    // move constructor. The pixels pointer is transferred, not copied.
    ImageBuffer gray = to_grayscale(std::move(img));  // MOVE, not copy!
    print_counters("After to_grayscale (img moved!):");

    // img is now in a moved-from state — valid but empty
    std::cout << "\n  [Check] img.empty() after move: "
              << (img.empty() ? "yes (correct!)" : "no (BUG!)") << '\n';

    // Apply blur — WITH std::move, this MOVES gray
    ImageBuffer blurred = blur(std::move(gray), 5);   // MOVE, not copy!
    print_counters("After blur (gray moved!):");

    std::cout << "\n  [Check] gray.empty() after move: "
              << (gray.empty() ? "yes (correct!)" : "no (BUG!)") << '\n';

    // Save — const reference, no copy or move needed
    save_image(blurred, "output.png");
    print_counters("After save_image (const ref):");

    std::cout << "\n";
    std::cout << "--- Pipeline complete ---\n";
    std::cout << "\n";
    std::cout << "RESULT: " << ImageBuffer::copy_count << " copies, "
              << ImageBuffer::move_count << " moves\n";
    std::cout << "\n";
    std::cout << "SUCCESS: Zero copies! Each move just transferred a pointer.\n";
    std::cout << "         The same pixel data traveled through the whole pipeline\n";
    std::cout << "         without ever being duplicated.\n";
    std::cout << "\n";

    // Moved-from buffers are empty; only blurred has the data
    std::cout << "Buffer states after pipeline:\n";
    std::cout << "  img.empty():     " << (img.empty() ? "yes (moved-from)" : "no") << '\n';
    std::cout << "  gray.empty():    " << (gray.empty() ? "yes (moved-from)" : "no") << '\n';
    std::cout << "  blurred.empty(): " << (blurred.empty() ? "yes (BUG!)" : "no (has data)") << '\n';
}

// =============================================================================
// DEMO 3: Moved-From State Deep Dive
// =============================================================================

void demo_moved_from_state() {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "DEMO 3: Understanding Moved-From State\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "After an object is moved from, it is in a 'valid but unspecified' state.\n";
    std::cout << "For ImageBuffer, we define this precisely: moved-from objects are EMPTY.\n";
    std::cout << "\n";
    std::cout << "C# COMPARISON:\n";
    std::cout << "  In C#, there is no concept of 'moved-from'. After:\n";
    std::cout << "    var b = a;  // Both a and b reference the same object.\n";
    std::cout << "  Both variables remain valid and point to the same data.\n";
    std::cout << "\n";
    std::cout << "C++ REALITY:\n";
    std::cout << "  After: ImageBuffer b = std::move(a);\n";
    std::cout << "  'a' is empty. Its resources were stolen by 'b'.\n";
    std::cout << "  You can still destroy 'a' or assign a new value to it,\n";
    std::cout << "  but you cannot assume it contains the old data.\n";
    std::cout << "\n";

    ImageBuffer::reset_counters();

    // Create an image
    std::cout << "--- Creating original buffer ---\n";
    ImageBuffer original{100, 100};

    std::cout << "\n--- Memory layout BEFORE move ---\n";
    print_memory_layout("original", original);

    // Move to a new buffer
    std::cout << "\n--- Moving: ImageBuffer moved = std::move(original); ---\n";
    ImageBuffer moved = std::move(original);

    std::cout << "\n--- Memory layout AFTER move ---\n";
    print_memory_layout("original (moved-from)", original);
    print_memory_layout("moved (now owns data)", moved);

    std::cout << "\n";
    std::cout << "KEY OBSERVATION:\n";
    std::cout << "  - 'original' now has pixels_ = nullptr (moved-from state)\n";
    std::cout << "  - 'moved' has the same pixels_ address that 'original' had\n";
    std::cout << "  - The pixel data was NOT copied — just the pointer was transferred\n";
    std::cout << "\n";

    // Demonstrate that moved-from object can be safely reused
    std::cout << "--- Reusing moved-from object: original = ImageBuffer{50, 50}; ---\n";
    original = ImageBuffer{50, 50};

    std::cout << "\n--- Memory layout after reassignment ---\n";
    print_memory_layout("original (new value)", original);
    print_memory_layout("moved (unchanged)", moved);

    std::cout << "\n";
    std::cout << "A moved-from object can be:\n";
    std::cout << "  - Destroyed (destructor handles empty state correctly)\n";
    std::cout << "  - Assigned a new value (like we just did)\n";
    std::cout << "  - Queried for emptiness (empty() returns true)\n";
    std::cout << "\n";
    std::cout << "A moved-from object should NOT be:\n";
    std::cout << "  - Assumed to contain the old data\n";
    std::cout << "  - Dereferenced without checking (data() returns nullptr)\n";
    std::cout << "\n";

    print_counters("Final counter state:");
}

// =============================================================================
// DEMO 4: Memory Layout Visualization
// =============================================================================

void demo_memory_layout() {
    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "DEMO 4: Memory Layout — What the CPU Sees\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "ImageBuffer is a small object (typically 16 bytes on 64-bit systems):\n";
    std::cout << "  - int32_t width_   (4 bytes)\n";
    std::cout << "  - int32_t height_  (4 bytes)\n";
    std::cout << "  - byte* pixels_    (8 bytes on 64-bit)\n";
    std::cout << "\n";
    std::cout << "The pixel data itself is on the heap, potentially megabytes away.\n";
    std::cout << "\n";

    std::cout << "sizeof(ImageBuffer) = " << sizeof(ImageBuffer) << " bytes\n";
    std::cout << "sizeof(std::int32_t) = " << sizeof(std::int32_t) << " bytes\n";
    std::cout << "sizeof(std::byte*) = " << sizeof(std::byte*) << " bytes\n";
    std::cout << "\n";

    ImageBuffer::reset_counters();

    ImageBuffer small{10, 10};    // 400 bytes of pixels
    ImageBuffer large{1000, 1000}; // 4,000,000 bytes of pixels

    std::cout << "\n--- Two buffers, very different pixel sizes ---\n";
    print_memory_layout("small (10x10)", small);
    print_memory_layout("large (1000x1000)", large);

    std::cout << "\n";
    std::cout << "KEY INSIGHT:\n";
    std::cout << "  Copying 'small' memcpy's 400 bytes.\n";
    std::cout << "  Copying 'large' memcpy's 4,000,000 bytes.\n";
    std::cout << "  Moving either one transfers 3 integers/pointers.\n";
    std::cout << "\n";
    std::cout << "  Move cost is O(1). Copy cost is O(width * height).\n";
    std::cout << "\n";
}

// =============================================================================
// MAIN
// =============================================================================

int main() {
    std::cout << "================================================================\n";
    std::cout << "Cpptude #004: Image Pipeline — Move Semantics & Ownership\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "This cpptude teaches MOVE SEMANTICS through an image processing\n";
    std::cout << "pipeline. You will see exactly when copies and moves happen,\n";
    std::cout << "and learn why std::move enables zero-copy pipelines.\n";
    std::cout << "\n";
    std::cout << "Watch the [ImageBuffer] messages on stderr for construction,\n";
    std::cout << "destruction, copy, and move events.\n";

    // Run all demonstrations
    demo_without_move();
    demo_with_move();
    demo_moved_from_state();
    demo_memory_layout();

    std::cout << "\n";
    std::cout << "================================================================\n";
    std::cout << "KEY TAKEAWAYS\n";
    std::cout << "================================================================\n";
    std::cout << "\n";
    std::cout << "1. COPIES DUPLICATE RESOURCES.\n";
    std::cout << "   Copying an ImageBuffer allocates a new pixel buffer and\n";
    std::cout << "   memcpy's all the data. This is expensive.\n";
    std::cout << "\n";
    std::cout << "2. MOVES TRANSFER OWNERSHIP.\n";
    std::cout << "   Moving an ImageBuffer transfers the pixel pointer.\n";
    std::cout << "   No allocation. No memcpy. O(1) regardless of size.\n";
    std::cout << "\n";
    std::cout << "3. std::move IS A CAST, NOT A FUNCTION.\n";
    std::cout << "   std::move(x) does not move anything. It casts 'x' to an\n";
    std::cout << "   rvalue reference (T&&), which enables the move constructor\n";
    std::cout << "   or move assignment operator to be selected.\n";
    std::cout << "\n";
    std::cout << "4. MOVED-FROM OBJECTS ARE VALID BUT EMPTY.\n";
    std::cout << "   After std::move(x), 'x' is in a valid-but-unspecified state.\n";
    std::cout << "   For ImageBuffer, this means empty()==true, data()==nullptr.\n";
    std::cout << "   You can destroy it or assign a new value, but not use the old data.\n";
    std::cout << "\n";
    std::cout << "5. noexcept ENABLES OPTIMIZATION.\n";
    std::cout << "   STL containers only use move semantics if the move constructor\n";
    std::cout << "   is marked noexcept. Otherwise they fall back to copying.\n";
    std::cout << "\n";
    std::cout << "================================================================\n";

    return 0;
}
