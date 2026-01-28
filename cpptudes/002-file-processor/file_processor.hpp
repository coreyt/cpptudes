// file_processor.hpp — Cpptude #002: RAII & File Processing
//
// PRIMARY KAP: RAII (Resource Acquisition Is Initialization)
//
// This header demonstrates:
//   - RAII wrapper class that owns a file resource
//   - Destructor guarantees: cleanup happens unconditionally at scope exit
//   - const member functions for read-only operations
//   - explicit single-argument constructor (prevents implicit conversion)
//   - Multi-file structure: this .hpp is included by both .cpp files
//
// BUILD:
//   g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic file_processor.cpp main.cpp -o file_processor
//
// BUILD WITH SANITIZERS:
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -Wall -Wextra file_processor.cpp main.cpp -o file_processor

#ifndef CPPTUDE_FILE_PROCESSOR_HPP
#define CPPTUDE_FILE_PROCESSOR_HPP

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace cpptude {

// =============================================================================
// LOG ENTRY TYPES
// =============================================================================

// Severity levels found in log files. These match a simple log format:
//   [SEVERITY] message text
enum class Severity {
    info,
    warning,
    error,
    unknown
};

// Parse a severity string ("INFO", "WARNING", "ERROR") into the enum.
[[nodiscard]] Severity parse_severity(std::string_view token) noexcept;

// Convert severity back to a display string.
[[nodiscard]] std::string_view severity_to_string(Severity sev) noexcept;

// Summary statistics produced by analyzing a log file.
struct LogSummary {
    std::size_t total_lines{0};
    std::size_t info_count{0};
    std::size_t warning_count{0};
    std::size_t error_count{0};
    std::size_t unknown_count{0};

    // Tracks how many times each unique error message appeared.
    std::unordered_map<std::string, std::size_t> error_messages;

    // Tracks how many times each unique warning message appeared.
    std::unordered_map<std::string, std::size_t> warning_messages;
};

// =============================================================================
// RAII FILE READER
// =============================================================================

// C# INSTINCT: var stream = File.OpenRead(path); ... stream.Close();
//              Or: using var stream = File.OpenRead(path);
//              In C#, if you forget Close(), the GC/finalizer handles it eventually.
//
// C++ REALITY: There is no GC. If you open a file and an exception propagates
//              before you call close(), the file handle leaks. RAII solves this:
//              the destructor runs unconditionally when the object leaves scope,
//              whether by normal flow or by exception.
//
// This class wraps std::ifstream to demonstrate the RAII principle explicitly.
// In production code, std::ifstream already IS an RAII type — its destructor
// closes the file. We wrap it here to make the destructor behavior visible
// through logging, so you can SEE when cleanup happens.

class LogFileReader {
public:
    // explicit prevents accidental conversion: LogFileReader r = "path";
    // would silently construct a reader — we want that to be a compile error.
    explicit LogFileReader(const std::filesystem::path& file_path);

    // The destructor is the RAII guarantee. It runs unconditionally when this
    // object leaves scope — whether the scope exits normally, via return,
    // via break, or via exception propagation. This is not optional cleanup
    // like C#'s IDisposable — it is guaranteed by the language.
    ~LogFileReader();

    // Disallow copying: a file handle is a unique resource. Two objects
    // closing the same handle would be a bug. (Rule of Five consideration:
    // since we define a destructor, we must think about copy/move. For now,
    // we simply delete copy and move — Rule of Zero comes in Cpptude #10.)
    LogFileReader(const LogFileReader&) = delete;
    LogFileReader& operator=(const LogFileReader&) = delete;
    LogFileReader(LogFileReader&&) = delete;
    LogFileReader& operator=(LogFileReader&&) = delete;

    // const member function: reading does not modify the LogFileReader's
    // logical state. Marking it const tells the compiler and the reader
    // that this operation is safe to call on a const reference.
    // (We use mutable on the stream because getline modifies the stream
    // position, which is an implementation detail, not a logical state change.)
    //
    // NOTE: read_line returns false when the stream is exhausted.
    [[nodiscard]] bool read_line(std::string& out_line) const;

    // Check whether the file was opened successfully.
    // const because checking status does not modify the object.
    [[nodiscard]] bool is_open() const noexcept;

    // Return the file path for diagnostics.
    // const because it only reads internal state.
    [[nodiscard]] const std::filesystem::path& path() const noexcept;

private:
    std::filesystem::path file_path_;

    // mutable because getline() modifies stream state (read position),
    // but the logical state of our reader does not change — we are still
    // "a reader of this file." This lets read_line() be const.
    mutable std::ifstream stream_;
};

// =============================================================================
// RAII REPORT WRITER
// =============================================================================

// Same RAII principle for output: the destructor flushes and closes the file.
// If the program throws while writing, the destructor still runs, ensuring
// partial output is flushed and the file handle is released.

class ReportWriter {
public:
    explicit ReportWriter(const std::filesystem::path& file_path);
    ~ReportWriter();

    // Disallow copying and moving (same rationale as LogFileReader)
    ReportWriter(const ReportWriter&) = delete;
    ReportWriter& operator=(const ReportWriter&) = delete;
    ReportWriter(ReportWriter&&) = delete;
    ReportWriter& operator=(ReportWriter&&) = delete;

    // Write a line to the report file.
    void write_line(std::string_view line);

    // Write the formatted summary to the report file.
    void write_summary(const LogSummary& summary);

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] const std::filesystem::path& path() const noexcept;

private:
    std::filesystem::path file_path_;
    std::ofstream stream_;
};

// =============================================================================
// LOG ANALYSIS (free functions)
// =============================================================================

// Analyze a log file and produce a summary.
// This function demonstrates RAII in action: the LogFileReader is created
// at the top of the function and automatically cleaned up when the function
// returns — even if an exception is thrown during processing.
[[nodiscard]] LogSummary analyze_log(const std::filesystem::path& log_path);

// Write a summary report to a file.
// Same RAII principle: ReportWriter cleans up automatically.
void write_report(const std::filesystem::path& report_path,
                  const LogSummary& summary);

// Combined operation: read log, analyze, write report.
// Demonstrates multiple RAII objects in the same scope — each one cleans up
// in reverse order of construction (stack unwinding order).
void process_log_file(const std::filesystem::path& log_path,
                      const std::filesystem::path& report_path);

} // namespace cpptude

#endif // CPPTUDE_FILE_PROCESSOR_HPP
