// main.cpp — Cpptude #002: Test harness and RAII demonstrations
//
// This file demonstrates:
//   1. Normal RAII cleanup (happy path)
//   2. RAII cleanup on exception (the critical path)
//   3. Manual open/close as a contrast (the C# instinct)
//
// BUILD:
//   g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic file_processor.cpp main.cpp -o file_processor
//
// RUN:
//   ./file_processor                        # Run all demos with sample data
//   ./file_processor input.log report.txt   # Process a specific log file

#include "file_processor.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace cpptude;

// =============================================================================
// SAMPLE DATA GENERATION
// =============================================================================

// Create a sample log file for demonstration.
// Returns the path to the created file.
std::filesystem::path create_sample_log(const std::filesystem::path& dir) {
    const auto log_path = dir / "sample.log";
    std::ofstream out{log_path};

    if (!out.is_open()) {
        throw std::runtime_error{"Failed to create sample log: " + log_path.string()};
    }

    // Realistic log entries that exercise all severity levels
    out << "[INFO] Application started\n";
    out << "[INFO] Loading configuration from /etc/app/config.yaml\n";
    out << "[INFO] Database connection established\n";
    out << "[WARNING] Configuration key 'timeout' not found, using default 30s\n";
    out << "[INFO] Listening on port 8080\n";
    out << "[ERROR] Failed to connect to cache server at 10.0.0.5:6379\n";
    out << "[WARNING] Configuration key 'timeout' not found, using default 30s\n";
    out << "[INFO] Request received: GET /api/health\n";
    out << "[INFO] Request received: POST /api/data\n";
    out << "[ERROR] Database query timeout after 30s\n";
    out << "[ERROR] Failed to connect to cache server at 10.0.0.5:6379\n";
    out << "[WARNING] Disk usage at 85%\n";
    out << "[ERROR] Database query timeout after 30s\n";
    out << "[INFO] Request received: GET /api/health\n";
    out << "[ERROR] Failed to connect to cache server at 10.0.0.5:6379\n";
    out << "[INFO] Graceful shutdown initiated\n";
    out << "[WARNING] 3 pending requests during shutdown\n";
    out << "[ERROR] Database query timeout after 30s\n";
    out << "[INFO] Application stopped\n";

    // out's destructor flushes and closes the file here — RAII at work,
    // even in this helper function.
    return log_path;
}

// =============================================================================
// DEMONSTRATION: THE C# INSTINCT (manual open/close)
// =============================================================================

// This function shows what a C# developer would naturally write.
// It works on the happy path but leaks the file handle on the error path.
//
// DO NOT WRITE CODE LIKE THIS IN C++. This exists only to demonstrate
// why manual resource management fails when exceptions enter the picture.

void demo_manual_open_close(const std::filesystem::path& log_path) {
    std::cout << "\n=== DEMO: Manual Open/Close (The C# Instinct) ===\n\n";

    // C# INSTINCT: var file = File.OpenText(path);
    auto* file = new std::ifstream{log_path};

    if (!file->is_open()) {
        std::cout << "  Failed to open file.\n";
        delete file;
        return;
    }

    std::cout << "  File opened manually.\n";

    std::string line;
    int line_count{0};

    while (std::getline(*file, line)) {
        ++line_count;

        // Imagine this function throws on a malformed line.
        // In C#, the GC would eventually clean up the stream.
        // In C++, the file handle leaks. delete file never runs.
        //
        // Uncomment the next line to simulate the error path:
        // if (line_count == 5) throw std::runtime_error{"Simulated parse error"};
    }

    std::cout << "  Processed " << line_count << " lines.\n";

    // C# INSTINCT: file.Close();  — but what if we threw above?
    delete file;
    std::cout << "  File closed manually.\n";

    std::cout << "\n  PROBLEM: If an exception occurs between open and close,\n";
    std::cout << "  the file handle leaks. There is no GC to save you.\n";
}

// =============================================================================
// DEMONSTRATION: RAII (The C++ Way)
// =============================================================================

void demo_raii_normal(const std::filesystem::path& log_path) {
    std::cout << "\n=== DEMO: RAII Cleanup (Normal Exit) ===\n\n";

    // The LogFileReader opens the file in its constructor.
    // Watch stderr for the [RAII] messages showing construction and destruction.
    {
        const LogFileReader reader{log_path};
        std::string line;
        int line_count{0};

        while (reader.read_line(line)) {
            ++line_count;
        }

        std::cout << "  Processed " << line_count << " lines.\n";
        std::cout << "  About to exit scope — watch stderr for destructor message.\n";

        // reader's destructor runs HERE, at the closing brace.
        // You did not call close(). You cannot forget it.
    }

    std::cout << "  Scope exited. File was closed automatically.\n";
}

void demo_raii_exception(const std::filesystem::path& log_path) {
    std::cout << "\n=== DEMO: RAII Cleanup (Exception Path) ===\n\n";

    try {
        const LogFileReader reader{log_path};
        std::string line;
        int line_count{0};

        while (reader.read_line(line)) {
            ++line_count;

            // Simulate an error during processing.
            // The reader's destructor STILL runs because RAII guarantees
            // cleanup on all exit paths, including exceptions.
            if (line_count == 5) {
                std::cout << "  Throwing exception after line " << line_count << "...\n";
                throw std::runtime_error{"Simulated processing error at line 5"};
            }
        }

        // This line is never reached when the exception fires.
        std::cout << "  Processed " << line_count << " lines.\n";

        // reader's destructor runs even though we are leaving via exception.
        // Watch stderr: you will see "[RAII] LogFileReader closing:" BEFORE
        // the catch block runs. The destructor fires during stack unwinding.
    } catch (const std::runtime_error& e) {
        std::cout << "  Caught exception: " << e.what() << '\n';
        std::cout << "  File was STILL closed automatically (check stderr above).\n";
    }
}

// =============================================================================
// DEMONSTRATION: Full Pipeline
// =============================================================================

void demo_full_pipeline(const std::filesystem::path& log_path,
                        const std::filesystem::path& report_path) {
    std::cout << "\n=== DEMO: Full Log Processing Pipeline ===\n\n";

    process_log_file(log_path, report_path);

    // Read and display the generated report
    std::cout << "\n  Generated report (" << report_path << "):\n";
    std::cout << "  ----------------------------------------\n";

    std::ifstream report{report_path};
    std::string line;
    while (std::getline(report, line)) {
        std::cout << "  " << line << '\n';
    }
    std::cout << "  ----------------------------------------\n";
}

// =============================================================================
// DEMONSTRATION: Destruction Order
// =============================================================================

void demo_destruction_order(const std::filesystem::path& log_path,
                            const std::filesystem::path& report_path) {
    std::cout << "\n=== DEMO: Destruction Order ===\n\n";

    std::cout << "  Creating two RAII objects in sequence.\n";
    std::cout << "  Watch stderr: they destroy in REVERSE order.\n";
    std::cout << "  (Last constructed = first destroyed, like a stack.)\n\n";

    {
        // Constructed first — will be destroyed SECOND
        const LogFileReader reader{log_path};

        // Constructed second — will be destroyed FIRST
        ReportWriter writer{report_path};

        std::string line;
        int line_count{0};
        while (reader.read_line(line)) {
            ++line_count;
        }

        writer.write_line("Processed " + std::to_string(line_count) + " lines.");
        std::cout << "  About to exit scope...\n";

        // Destruction order: writer first (constructed second), then reader.
        // This is deterministic. Always. Unlike C#'s GC collection order.
    }

    std::cout << "  Both resources released in reverse construction order.\n";
}

// =============================================================================
// MAIN
// =============================================================================

int main(int argc, char* argv[]) {
    std::cout << "=== Cpptude #002: File Processor ===\n";
    std::cout << "Primary KAP: RAII (Resource Acquisition Is Initialization)\n";

    try {
        if (argc == 3) {
            // User-specified input and output files
            process_log_file(argv[1], argv[2]);
            return 0;
        }

        // Create a temporary directory for sample data
        const auto tmp_dir = std::filesystem::temp_directory_path() / "cpptude_002";
        std::filesystem::create_directories(tmp_dir);

        const auto log_path = create_sample_log(tmp_dir);
        const auto report_path = tmp_dir / "report.txt";

        // Run all demonstrations
        demo_manual_open_close(log_path);
        demo_raii_normal(log_path);
        demo_raii_exception(log_path);
        demo_destruction_order(log_path, report_path);
        demo_full_pipeline(log_path, report_path);

        // Clean up temporary files
        std::filesystem::remove_all(tmp_dir);

        std::cout << "\n=== All demonstrations complete. ===\n";
        std::cout << "Key insight: RAII makes resource cleanup automatic and\n";
        std::cout << "exception-safe. You cannot forget to close a file.\n";

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
