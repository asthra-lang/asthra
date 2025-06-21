/**
 * Asthra Programming Language
 * Common Error Framework for Tools
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ERROR_FRAMEWORK_H
#define ASTHRA_ERROR_FRAMEWORK_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// Error severity levels
typedef enum {
    ERROR_INFO,
    ERROR_WARNING,
    ERROR_ERROR,
    ERROR_CRITICAL
} ErrorSeverity;

// Error context structure
typedef struct {
    const char *file_name;      // Source file name
    size_t line_number;         // Line number (0 if not applicable)
    size_t column_number;       // Column number (0 if not applicable)
    const char *function_name;  // Function name (optional)
} ErrorContext;

// Error message structure
typedef struct {
    ErrorSeverity severity;     // Error severity level
    const char *message;        // Error message
    const char *suggestion;     // Suggested fix (optional)
    ErrorContext context;       // Error context
    uint64_t timestamp_ms;      // Timestamp when error occurred
} ErrorMessage;

// Error framework structure
typedef struct {
    const char *tool_name;      // Tool name for error reporting
    ErrorMessage *errors;       // Dynamic array of errors
    size_t error_count;         // Number of errors
    size_t error_capacity;      // Capacity of error array
    bool warnings_as_errors;    // Whether to treat warnings as errors
    ErrorSeverity min_severity; // Minimum severity to report
} ErrorFramework;

// Error framework functions
ErrorFramework* error_create_framework(const char *tool_name);
void error_destroy_framework(ErrorFramework *framework);

// Error reporting functions
void error_report(ErrorFramework *framework, ErrorSeverity severity, 
                 const char *message, const char *suggestion,
                 const char *file_name, size_t line_number, size_t column_number);

void error_report_simple(ErrorFramework *framework, ErrorSeverity severity, const char *message);

// Error query functions
size_t error_get_count(const ErrorFramework *framework, ErrorSeverity severity);
size_t error_get_total_count(const ErrorFramework *framework);
bool error_has_errors(const ErrorFramework *framework);
bool error_has_critical_errors(const ErrorFramework *framework);

// Error output functions
void error_print_summary(const ErrorFramework *framework, bool verbose);
void error_print_json(const ErrorFramework *framework);
void error_print_all(const ErrorFramework *framework);

// Configuration functions
void error_set_warnings_as_errors(ErrorFramework *framework, bool enabled);
void error_set_min_severity(ErrorFramework *framework, ErrorSeverity min_severity);

// Utility functions
const char* error_severity_to_string(ErrorSeverity severity);
ErrorSeverity error_string_to_severity(const char *severity_str);

// Convenience macros for common error patterns
#define ERROR_REPORT_INFO(framework, msg) \
    error_report_simple(framework, ERROR_INFO, msg)

#define ERROR_REPORT_WARNING(framework, msg) \
    error_report_simple(framework, ERROR_WARNING, msg)

#define ERROR_REPORT_ERROR(framework, msg) \
    error_report_simple(framework, ERROR_ERROR, msg)

#define ERROR_REPORT_CRITICAL(framework, msg) \
    error_report_simple(framework, ERROR_CRITICAL, msg)

#endif // ASTHRA_ERROR_FRAMEWORK_H 
