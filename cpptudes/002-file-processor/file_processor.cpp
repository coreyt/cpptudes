// file_processor.cpp — Cpptude #002: RAII & File Processing
//
// Implementation of the RAII file wrappers and log analysis logic.
// See file_processor.hpp for type definitions and educational commentary.
//
// KEY LESSON: Every resource acquisition (file open) is paired with
// automatic release (destructor close). There is no code path —
// normal return, early return, or exception — where cleanup is skipped.
//
// BUILD:
//   g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic file_processor.cpp main.cpp -o file_processor
//
// BUILD WITH SANITIZERS:
//   g++ -std=c++20 -O1 -g -fsanitize=address,undefined
//       -Wall -Wextra file_processor.cpp main.cpp -o file_processor

#include "file_processor.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace cpptude {

// =============================================================================
// SEVERITY HELPERS
// =============================================================================

[[nodiscard]] Severity parse_severity(std::string_view token) noexcept {
    if (token == "INFO") return Severity::info;
    if (token == "WARNING") return Severity::warning;
    if (token == "ERROR") return Severity::error;
    return Severity::unknown;
}

[[nodiscard]] std::string_view severity_to_string(Severity sev) noexcept {
    switch (sev) {
        case Severity::info:    return "INFO";
        case Severity::warning: return "WARNING";
        case Severity::error:   return "ERROR";
        case Severity::unknown: return "UNKNOWN";
    }
    return "UNKNOWN";
}

// =============================================================================
// LogFileReader — RAII wrapper for input files
// =============================================================================

// CONSTRUCTOR: Acquires the resource (opens the file).
// The "RI" in RAII: Resource acquisition IS initialization.
// If the file cannot be opened, we throw — the object is never constructed,
// so the destructor will never run. This is correct: you only clean up
// resources you successfully acquired.
LogFileReader::LogFileReader(const std::filesystem::path& file_path)
    : file_path_{file_path}
    , stream_{file_path}
{
    if (!stream_.is_open()) {
        throw std::runtime_error{
            "Failed to open log file: " + file_path.string()
        };
    }

    // Destructor logging: visible proof that RAII is working.
    // In production code you would not log from a constructor, but for
    // learning purposes this makes the resource lifecycle observable.
    std::cerr << "[RAII] LogFileReader opened: " << file_path_ << '\n';
}

// DESTRUCTOR: Releases the resource (closes the file).
// This runs unconditionally when the LogFileReader leaves scope.
//
// C# INSTINCT: "I need to call Dispose() or wrap in using."
// C++ REALITY: The destructor IS the using block. You cannot forget it.
//              It runs at scope exit. Period. No matter how you exit the scope.
//
// The destructor must not throw — if it runs during stack unwinding (because
// an exception is propagating), a throwing destructor would call std::terminate.
LogFileReader::~LogFileReader() {
    // stream_ closes automatically (it is itself an RAII type), but we log
    // to make the destruction visible for learning.
    std::cerr << "[RAII] LogFileReader closing: " << file_path_ << '\n';

    // stream_.close() is called by ~ifstream() — we do not need to call it
    // explicitly. This is RAII all the way down: our RAII class contains an
    // RAII member, and both destructors run automatically.
}

[[nodiscard]] bool LogFileReader::read_line(std::string& out_line) const {
    // std::getline modifies stream_ (advances read position), which is why
    // stream_ is marked mutable. The logical state of the reader ("I am
    // reading this file") does not change, but the physical stream state does.
    return static_cast<bool>(std::getline(stream_, out_line));
}

[[nodiscard]] bool LogFileReader::is_open() const noexcept {
    return stream_.is_open();
}

[[nodiscard]] const std::filesystem::path& LogFileReader::path() const noexcept {
    return file_path_;
}

// =============================================================================
// ReportWriter — RAII wrapper for output files
// =============================================================================

ReportWriter::ReportWriter(const std::filesystem::path& file_path)
    : file_path_{file_path}
    , stream_{file_path}
{
    if (!stream_.is_open()) {
        throw std::runtime_error{
            "Failed to open report file: " + file_path.string()
        };
    }

    std::cerr << "[RAII] ReportWriter opened: " << file_path_ << '\n';
}

// Same guarantee as LogFileReader: unconditional cleanup at scope exit.
// The ofstream destructor flushes any buffered data, then closes the handle.
ReportWriter::~ReportWriter() {
    std::cerr << "[RAII] ReportWriter closing: " << file_path_ << '\n';
}

void ReportWriter::write_line(std::string_view line) {
    stream_ << line << '\n';
}

void ReportWriter::write_summary(const LogSummary& summary) {
    stream_ << "=== Log Analysis Report ===\n\n";
    stream_ << "Total lines analyzed: " << summary.total_lines << '\n';
    stream_ << "  INFO:    " << summary.info_count << '\n';
    stream_ << "  WARNING: " << summary.warning_count << '\n';
    stream_ << "  ERROR:   " << summary.error_count << '\n';
    stream_ << "  UNKNOWN: " << summary.unknown_count << '\n';

    if (!summary.error_messages.empty()) {
        stream_ << "\n--- Error Details ---\n";

        // Collect and sort error messages by count (descending) for a
        // deterministic, readable report.
        std::vector<std::pair<std::string, std::size_t>> sorted_errors{
            summary.error_messages.begin(), summary.error_messages.end()
        };
        std::sort(sorted_errors.begin(), sorted_errors.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        for (const auto& [message, count] : sorted_errors) {
            stream_ << "  [" << count << "x] " << message << '\n';
        }
    }

    if (!summary.warning_messages.empty()) {
        stream_ << "\n--- Warning Details ---\n";

        std::vector<std::pair<std::string, std::size_t>> sorted_warnings{
            summary.warning_messages.begin(), summary.warning_messages.end()
        };
        std::sort(sorted_warnings.begin(), sorted_warnings.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        for (const auto& [message, count] : sorted_warnings) {
            stream_ << "  [" << count << "x] " << message << '\n';
        }
    }
}

[[nodiscard]] bool ReportWriter::is_open() const noexcept {
    return stream_.is_open();
}

[[nodiscard]] const std::filesystem::path& ReportWriter::path() const noexcept {
    return file_path_;
}

// =============================================================================
// PARSING HELPERS
// =============================================================================

namespace {

// Parse a log line in the format: [SEVERITY] message text
// Returns the severity and extracts the message portion.
struct ParsedLine {
    Severity severity;
    std::string message;
};

[[nodiscard]] ParsedLine parse_log_line(std::string_view line) {
    // Find the severity tag: [INFO], [WARNING], [ERROR]
    if (line.size() < 3 || line[0] != '[') {
        return {Severity::unknown, std::string{line}};
    }

    const auto close_bracket = line.find(']');
    if (close_bracket == std::string_view::npos) {
        return {Severity::unknown, std::string{line}};
    }

    const auto severity_str = line.substr(1, close_bracket - 1);
    const auto severity = parse_severity(severity_str);

    // Extract message: everything after "] "
    auto message_start = close_bracket + 1;
    if (message_start < line.size() && line[message_start] == ' ') {
        ++message_start;
    }

    const auto message = line.substr(message_start);
    return {severity, std::string{message}};
}

} // anonymous namespace

// =============================================================================
// LOG ANALYSIS
// =============================================================================

// This function demonstrates RAII in its most natural form.
// The LogFileReader is created at function entry and destroyed at function exit.
// If parse_log_line or any other operation were to throw, the reader's
// destructor would still run, closing the file. There is no try/finally,
// no using block, no manual cleanup — the language guarantees it.
//
// DESTRUCTION ORDER: C++ destroys local variables in reverse order of
// construction. If this function had multiple RAII objects, the last one
// constructed would be the first one destroyed. This is deterministic
// and predictable — unlike C#'s GC, which collects in arbitrary order.

[[nodiscard]] LogSummary analyze_log(const std::filesystem::path& log_path) {
    // Resource acquired here — file opens in constructor
    const LogFileReader reader{log_path};

    LogSummary summary{};
    std::string line;

    // read_line is const: it does not modify the reader's logical state
    while (reader.read_line(line)) {
        ++summary.total_lines;

        const auto parsed = parse_log_line(line);

        switch (parsed.severity) {
            case Severity::info:
                ++summary.info_count;
                break;
            case Severity::warning:
                ++summary.warning_count;
                ++summary.warning_messages[parsed.message];
                break;
            case Severity::error:
                ++summary.error_count;
                ++summary.error_messages[parsed.message];
                break;
            case Severity::unknown:
                ++summary.unknown_count;
                break;
        }
    }

    // reader's destructor runs here — file closes automatically.
    // You did not write "reader.close()". You cannot forget it.
    return summary;
}

void write_report(const std::filesystem::path& report_path,
                  const LogSummary& summary) {
    // Resource acquired here — file opens in constructor
    ReportWriter writer{report_path};

    writer.write_summary(summary);

    // writer's destructor runs here — file flushes and closes automatically.
}

void process_log_file(const std::filesystem::path& log_path,
                      const std::filesystem::path& report_path) {
    // Step 1: Analyze the log file.
    // The LogFileReader inside analyze_log opens and closes automatically.
    const LogSummary summary = analyze_log(log_path);

    // Step 2: Write the report.
    // The ReportWriter inside write_report opens and closes automatically.
    write_report(report_path, summary);

    std::cerr << "[RAII] Processing complete. All resources cleaned up.\n";
}

} // namespace cpptude
