# Cpptude #004: Image Pipeline — Move Semantics & Ownership Transfer

| Field | Value |
|-------|-------|
| **Primary KAP** | Move Semantics (Rating 4 - Required) |
| **Secondary Concepts** | `std::move`, moved-from state, `noexcept` move, copy elision, ownership transfer |
| **Difficulty** | Intermediate+ |
| **Prerequisite** | Cpptude #003 (Parameter Passing) |
| **Bait** | Copy a resource-owning object and expect both copies to remain valid |
| **Failure Signal** | Moved-from object becomes empty; use-after-move bugs; copy counter spikes |
| **Verification Tool** | Copy/move counters + ASan for use-after-move |

---

## Overview

This cpptude teaches **move semantics** — the C++ feature that allows safe, explicit transfer of resources from one object to another without copying. You will build a simple image-processing pipeline where each stage consumes an image and produces a new one. The pipeline is intentionally designed so that copying is expensive, while moving is cheap and correct.

> **C# Bridge:** In C#, `=` on a reference type creates another reference to the same object. Both references remain valid, and the GC manages lifetime. In C++, `=` on a value type **copies** the object. If that object owns a resource (like a heap buffer), copying means duplicating the resource — or worse, double-free bugs if you implement it incorrectly. Move semantics is how C++ gives you *efficient* and *correct* ownership transfer.

---

## Session Structure

| Phase | Duration | Activity |
|-------|----------|----------|
| **1. Warm-Up and Review** | 15-20 min | Review parameter passing modes from #003. Identify where `T&&` appears. |
| **2. Trap Encounter** | 30-45 min | Present a resource-owning `ImageBuffer` type. Let the learner copy it and observe the cost. |
| **3. Concept Introduction** | 20-30 min | Explain move constructor/assignment, `std::move`, and moved-from state. |
| **4. Guided Practice** | 30-45 min | Implement move operations and refactor pipeline functions to accept/return by value with moves. |
| **5. Independent Practice** | 20-30 min | Challenge: add a new pipeline stage that reuses buffers without copies. |

---

## The Problem: Image Pipeline

You are given a sequence of transforms that operate on a large `ImageBuffer`:

1. Load raw pixels from disk.
2. Convert to grayscale.
3. Apply a blur filter.
4. Save the result.

The core type owns a heap buffer:

```cpp
class ImageBuffer {
public:
    ImageBuffer(int width, int height);
    ~ImageBuffer();

    ImageBuffer(const ImageBuffer& other);            // expensive copy
    ImageBuffer& operator=(const ImageBuffer& other); // expensive copy

    ImageBuffer(ImageBuffer&& other) noexcept;        // cheap move
    ImageBuffer& operator=(ImageBuffer&& other) noexcept;

    std::byte* data();
    const std::byte* data() const;

private:
    int width_{};
    int height_{};
    std::byte* pixels_{}; // heap-allocated
};
```

The pipeline operations should **consume** their inputs when possible:

```cpp
ImageBuffer load_image(const std::filesystem::path& path);
ImageBuffer to_grayscale(ImageBuffer image);
ImageBuffer blur(ImageBuffer image, int radius);
void save_image(const ImageBuffer& image, const std::filesystem::path& path);
```

Notice that `to_grayscale` and `blur` take `ImageBuffer` **by value**. This is intentional: it enables move semantics. When you call `to_grayscale(std::move(image))`, ownership transfers into the function. The return value is then moved (or elided) out.

---

## Instrumentation: Count Copies vs Moves

Add counters inside `ImageBuffer` so the cost is obvious:

```cpp
class ImageBuffer {
public:
    static inline std::size_t copy_count = 0;
    static inline std::size_t move_count = 0;

    ImageBuffer(const ImageBuffer& other)
        : width_(other.width_),
          height_(other.height_),
          pixels_(new std::byte[other.width_ * other.height_ * 4]) {
        std::memcpy(pixels_, other.pixels_, width_ * height_ * 4);
        ++copy_count;
    }

    ImageBuffer(ImageBuffer&& other) noexcept
        : width_(other.width_),
          height_(other.height_),
          pixels_(other.pixels_) {
        other.width_ = 0;
        other.height_ = 0;
        other.pixels_ = nullptr;
        ++move_count;
    }

    ImageBuffer& operator=(const ImageBuffer& other) {
        if (this != &other) {
            ImageBuffer temp(other);
            swap(temp);
        }
        return *this;
    }

    ImageBuffer& operator=(ImageBuffer&& other) noexcept {
        if (this != &other) {
            delete[] pixels_;
            width_ = other.width_;
            height_ = other.height_;
            pixels_ = other.pixels_;
            other.width_ = 0;
            other.height_ = 0;
            other.pixels_ = nullptr;
            ++move_count;
        }
        return *this;
    }

    void swap(ImageBuffer& other) noexcept {
        std::swap(width_, other.width_);
        std::swap(height_, other.height_);
        std::swap(pixels_, other.pixels_);
    }
};
```

Run the pipeline first with copies, then with `std::move`. Your target is **zero copies** in the steady-state pipeline.

---

## The Trap: “Copies Are Just Aliases”

A C# developer expects:

```cpp
ImageBuffer a = load_image(path);
ImageBuffer b = a;  // C# intuition: b is another reference to the same pixels
```

In C++, this is a deep copy of the pixel buffer. If the copy constructor is not implemented correctly, it can cause **double-free** or **use-after-free** bugs. Even when correct, it is slow.

Move semantics fix this:

```cpp
ImageBuffer a = load_image(path);
ImageBuffer b = std::move(a); // a is now in a valid-but-empty state
```

Now the pixel buffer ownership moves from `a` to `b` without copying.

---

## Moved-From State

A moved-from object must remain **valid but empty**. You can safely destroy it or assign a new value, but you cannot assume it still contains the old data.

```cpp
ImageBuffer a = load_image(path);
ImageBuffer b = std::move(a);
// a.data() is now null; width/height may be 0.
```

This is a critical difference from C# — there is no concept of a moved-from object in C# because there is no ownership transfer.

---

## `noexcept` Matters

The standard library only uses moves in many contexts if the move constructor is `noexcept`. If it might throw, the library will fall back to copying to preserve exception safety.

For `ImageBuffer`, the move operations should be `noexcept` because they only swap pointers and integers.

---

## C# Bridge: Why This Exists at All

C# developers are used to writing:

```csharp
var filtered = Blur(Grayscale(image));
```

This allocates a new image for each step, but the GC handles cleanup. In C++, you need to be explicit about ownership to avoid memory leaks and to keep performance predictable. Move semantics gives you a way to write code that looks high-level but still performs like hand-optimized C++.

---

## Next Step Challenge

Add a `crop` stage that returns a new `ImageBuffer`. Your goal: ensure **zero deep copies** in the pipeline.

1. Add copy/move counters to `ImageBuffer`.
2. Build a pipeline: `load -> grayscale -> blur -> crop -> save`.
3. Make the copy count zero while keeping the code readable.

---

## Key Takeaways

1. **Moves transfer ownership; copies duplicate resources.**
2. **`std::move` doesn’t move — it *casts* to an rvalue.** The move happens in the constructor/assignment.
3. **Moved-from objects are valid but empty.** Don’t use their contents.
4. **`noexcept` enables moves inside standard containers.**
5. **Move semantics are the bridge between performance and safety in C++.**

> **C# Bridge:** C# hides ownership. C++ makes it explicit. Move semantics are the tool that lets you write C++ that is both fast and safe without manual `new`/`delete` juggling.
