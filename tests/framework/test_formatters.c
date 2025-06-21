/**
 * Asthra Programming Language - Enhanced Test Formatters
 * Comprehensive test output formatting with multiple formats and styles
 *
 * Phase 4: Test Framework Enhancement
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Enhanced test output formatting including JSON, XML, colored console output,
 * and detailed reporting for developer productivity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include "test_framework.h"
#include "test_formatters.h"

// =============================================================================
// FORMATTER CONFIGURATION
// =============================================================================

// AsthraTestOutputFormat is already defined in test_formatters.h

typedef struct {
    AsthraTestOutputFormat format;
    bool use_colors;
    bool show_timing;
    bool show_memory;
    bool verbose_output;
    bool include_stack_traces;
    FILE* output_file;
    const char* output_filename;
} AsthraTestFormatterConfig;

// ANSI color codes for console output
#define ANSI_RESET          "\033[0m"
#define ANSI_BOLD           "\033[1m"
#define ANSI_RED            "\033[31m"
#define ANSI_GREEN          "\033[32m"
#define ANSI_YELLOW         "\033[33m"
#define ANSI_BLUE           "\033[34m"
#define ANSI_MAGENTA        "\033[35m"
#define ANSI_CYAN           "\033[36m"
#define ANSI_WHITE          "\033[37m"
#define ANSI_GRAY           "\033[90m"
#define ANSI_BRIGHT_RED     "\033[91m"
#define ANSI_BRIGHT_GREEN   "\033[92m"
#define ANSI_BRIGHT_YELLOW  "\033[93m"
#define ANSI_BRIGHT_BLUE    "\033[94m"

// Global formatter configuration
static AsthraTestFormatterConfig g_formatter_config = {
    .format = ASTHRA_FORMAT_CONSOLE,
    .use_colors = true,
    .show_timing = true,
    .show_memory = false,
    .verbose_output = false,
    .include_stack_traces = false,
    .output_file = NULL,
    .output_filename = NULL
};

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

static const char* get_result_symbol(AsthraTestResult result, bool use_unicode) {
    switch (result) {
        case ASTHRA_TEST_PASS:
            return use_unicode ? "✅" : "[PASS]";
        case ASTHRA_TEST_FAIL:
            return use_unicode ? "❌" : "[FAIL]";
        case ASTHRA_TEST_SKIP:
            return use_unicode ? "⏸️" : "[SKIP]";
        case ASTHRA_TEST_ERROR:
            return use_unicode ? "💥" : "[ERROR]";
        default:
            return use_unicode ? "❓" : "[UNKNOWN]";
    }
}

static const char* get_result_color(AsthraTestResult result) {
    if (!g_formatter_config.use_colors) return "";
    
    switch (result) {
        case ASTHRA_TEST_PASS:
            return ANSI_BRIGHT_GREEN;
        case ASTHRA_TEST_FAIL:
            return ANSI_BRIGHT_RED;
        case ASTHRA_TEST_SKIP:
            return ANSI_BRIGHT_YELLOW;
        case ASTHRA_TEST_ERROR:
            return ANSI_MAGENTA;
        default:
            return ANSI_GRAY;
    }
}

static void format_duration(char* buffer, size_t buffer_size, uint64_t duration_ns) {
    if (duration_ns < 1000) {
        snprintf(buffer, buffer_size, "%llu ns", (unsigned long long)duration_ns);
    } else if (duration_ns < 1000000) {
        snprintf(buffer, buffer_size, "%.1f μs", (double)duration_ns / 1000.0);
    } else if (duration_ns < 1000000000) {
        snprintf(buffer, buffer_size, "%.1f ms", (double)duration_ns / 1000000.0);
    } else {
        snprintf(buffer, buffer_size, "%.2f s", (double)duration_ns / 1000000000.0);
    }
}

static FILE* get_output_file(void) {
    return g_formatter_config.output_file ? g_formatter_config.output_file : stdout;
}

// =============================================================================
// CONSOLE FORMATTER
// =============================================================================

static void format_console_test_result(const AsthraTestContext* context) {
    FILE* output = get_output_file();
    const char* color = get_result_color(context->result);
    const char* symbol = get_result_symbol(context->result, true);
    const char* reset = g_formatter_config.use_colors ? ANSI_RESET : "";
    
    fprintf(output, "%s%s %s%s", color, symbol, context->metadata.name, reset);
    
    if (g_formatter_config.show_timing && context->duration_ns > 0) {
        char duration_str[64];
        format_duration(duration_str, sizeof(duration_str), context->duration_ns);
        fprintf(output, " %s(%s)%s", ANSI_GRAY, duration_str, reset);
    }
    
    if (context->result == ASTHRA_TEST_FAIL && context->error_message) {
        fprintf(output, "\n    %sError:%s %s", ANSI_RED, reset, context->error_message);
    }
    
    if (context->result == ASTHRA_TEST_SKIP && context->error_message) {
        fprintf(output, " %s- %s%s", ANSI_YELLOW, context->error_message, reset);
    }
    
    fprintf(output, "\n");
    
    if (g_formatter_config.verbose_output && context->assertions_in_test > 0) {
        fprintf(output, "    %sAssertions: %zu%s\n", 
                ANSI_GRAY, context->assertions_in_test, reset);
    }
}

static void format_console_suite_header(const char* suite_name) {
    FILE* output = get_output_file();
    const char* blue = g_formatter_config.use_colors ? ANSI_BRIGHT_BLUE : "";
    const char* reset = g_formatter_config.use_colors ? ANSI_RESET : "";
    
    fprintf(output, "\n%s=== %s ===%s\n", blue, suite_name, reset);
}

static void format_console_suite_summary(const AsthraTestStatistics* stats) {
    FILE* output = get_output_file();
    const char* reset = g_formatter_config.use_colors ? ANSI_RESET : "";
    
    size_t passed = atomic_load(&stats->tests_passed);
    size_t failed = atomic_load(&stats->tests_failed);
    size_t skipped = atomic_load(&stats->tests_skipped);
    size_t total = passed + failed + skipped;
    
    fprintf(output, "\n%s=== Test Summary ===%s\n", 
            g_formatter_config.use_colors ? ANSI_BOLD : "", reset);
    
    if (failed == 0) {
        fprintf(output, "%s✅ All tests passed!%s\n", 
                g_formatter_config.use_colors ? ANSI_BRIGHT_GREEN : "", reset);
    } else {
        fprintf(output, "%s❌ Some tests failed!%s\n", 
                g_formatter_config.use_colors ? ANSI_BRIGHT_RED : "", reset);
    }
    
    fprintf(output, "Tests run: %zu, Passed: %s%zu%s, Failed: %s%zu%s, Skipped: %s%zu%s\n",
            total,
            passed > 0 ? (g_formatter_config.use_colors ? ANSI_GREEN : "") : "",
            passed, reset,
            failed > 0 ? (g_formatter_config.use_colors ? ANSI_RED : "") : "",
            failed, reset,
            skipped > 0 ? (g_formatter_config.use_colors ? ANSI_YELLOW : "") : "",
            skipped, reset);
    
    if (g_formatter_config.show_timing) {
        char duration_str[64];
        format_duration(duration_str, sizeof(duration_str), atomic_load(&stats->total_duration_ns));
        fprintf(output, "Total time: %s\n", duration_str);
    }
}

// =============================================================================
// JSON FORMATTER
// =============================================================================

static void format_json_escape_string(FILE* output, const char* str) {
    if (!str) {
        fprintf(output, "null");
        return;
    }
    
    fprintf(output, "\"");
    for (const char* p = str; *p; p++) {
        switch (*p) {
            case '"': fprintf(output, "\\\""); break;
            case '\\': fprintf(output, "\\\\"); break;
            case '\n': fprintf(output, "\\n"); break;
            case '\r': fprintf(output, "\\r"); break;
            case '\t': fprintf(output, "\\t"); break;
            default: fputc(*p, output); break;
        }
    }
    fprintf(output, "\"");
}

static void format_json_test_result(FILE* output, const AsthraTestContext* context, bool is_first) {
    if (!is_first) fprintf(output, ",\n");
    
    fprintf(output, "    {\n");
    fprintf(output, "      \"name\": ");
    format_json_escape_string(output, context->metadata.name);
    fprintf(output, ",\n");
    
    fprintf(output, "      \"result\": \"%s\",\n", 
            context->result == ASTHRA_TEST_PASS ? "pass" :
            context->result == ASTHRA_TEST_FAIL ? "fail" :
            context->result == ASTHRA_TEST_SKIP ? "skip" : "error");
    
    fprintf(output, "      \"duration_ns\": %llu,\n", (unsigned long long)context->duration_ns);
    fprintf(output, "      \"assertions\": %zu,\n", context->assertions_in_test);
    
    if (context->error_message) {
        fprintf(output, "      \"error_message\": ");
        format_json_escape_string(output, context->error_message);
        fprintf(output, ",\n");
    }
    
    fprintf(output, "      \"file\": ");
    format_json_escape_string(output, context->metadata.file);
    fprintf(output, ",\n");
    fprintf(output, "      \"line\": %d\n", context->metadata.line);
    fprintf(output, "    }");
}

static void format_json_suite_header(FILE* output, const char* suite_name) {
    fprintf(output, "{\n");
    fprintf(output, "  \"suite_name\": ");
    format_json_escape_string(output, suite_name);
    fprintf(output, ",\n");
    fprintf(output, "  \"tests\": [\n");
}

static void format_json_suite_summary(FILE* output, const AsthraTestStatistics* stats) {
    fprintf(output, "\n  ],\n");
    fprintf(output, "  \"summary\": {\n");
    fprintf(output, "    \"total\": %llu,\n", 
            (unsigned long long)(atomic_load(&stats->tests_passed) + 
            atomic_load(&stats->tests_failed) + 
            atomic_load(&stats->tests_skipped)));
    fprintf(output, "    \"passed\": %llu,\n", (unsigned long long)atomic_load(&stats->tests_passed));
    fprintf(output, "    \"failed\": %llu,\n", (unsigned long long)atomic_load(&stats->tests_failed));
    fprintf(output, "    \"skipped\": %llu,\n", (unsigned long long)atomic_load(&stats->tests_skipped));
    fprintf(output, "    \"duration_ns\": %llu\n", (unsigned long long)atomic_load(&stats->total_duration_ns));
    fprintf(output, "  }\n");
    fprintf(output, "}\n");
}

// =============================================================================
// TAP FORMATTER (Test Anything Protocol)
// =============================================================================

static int tap_test_number = 1;

static void format_tap_suite_header(FILE* output, size_t total_tests) {
    fprintf(output, "1..%zu\n", total_tests);
    tap_test_number = 1;
}

static void format_tap_test_result(FILE* output, const AsthraTestContext* context) {
    const char* status = (context->result == ASTHRA_TEST_PASS) ? "ok" : "not ok";
    
    fprintf(output, "%s %d - %s", status, tap_test_number++, context->metadata.name);
    
    if (context->result == ASTHRA_TEST_SKIP) {
        fprintf(output, " # SKIP");
        if (context->error_message) {
            fprintf(output, " %s", context->error_message);
        }
    } else if (context->result == ASTHRA_TEST_FAIL && context->error_message) {
        fprintf(output, "\n  ---\n");
        fprintf(output, "  message: '%s'\n", context->error_message);
        fprintf(output, "  severity: fail\n");
        fprintf(output, "  data:\n");
        fprintf(output, "    got: (failure)\n");
        fprintf(output, "    expect: (success)\n");
        fprintf(output, "  ...\n");
    }
    
    fprintf(output, "\n");
}

// =============================================================================
// JUNIT XML FORMATTER
// =============================================================================

static void format_junit_suite_header(FILE* output, const char* suite_name, size_t total_tests) {
    fprintf(output, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(output, "<testsuite name=\"%s\" tests=\"%zu\">\n", suite_name, total_tests);
}

static void format_junit_test_result(FILE* output, const AsthraTestContext* context) {
    double duration_seconds = (double)context->duration_ns / 1000000000.0;
    
    fprintf(output, "  <testcase name=\"%s\" classname=\"%s\" time=\"%.6f\">\n",
            context->metadata.name, context->metadata.file, duration_seconds);
    
    if (context->result == ASTHRA_TEST_FAIL) {
        fprintf(output, "    <failure message=\"%s\">\n", 
                context->error_message ? context->error_message : "Test failed");
        fprintf(output, "      %s\n", 
                context->error_message ? context->error_message : "No details available");
        fprintf(output, "    </failure>\n");
    } else if (context->result == ASTHRA_TEST_SKIP) {
        fprintf(output, "    <skipped message=\"%s\"/>\n", 
                context->error_message ? context->error_message : "Test skipped");
    }
    
    fprintf(output, "  </testcase>\n");
}

static void format_junit_suite_summary(FILE* output, const AsthraTestStatistics* stats) {
    fprintf(output, "</testsuite>\n");
}

// =============================================================================
// PUBLIC FORMATTER INTERFACE
// =============================================================================

void asthra_test_formatter_set_config(AsthraTestOutputFormat format, bool use_colors, const char* output_file) {
    g_formatter_config.format = format;
    g_formatter_config.use_colors = use_colors;
    
    if (g_formatter_config.output_file && g_formatter_config.output_file != stdout) {
        fclose(g_formatter_config.output_file);
        g_formatter_config.output_file = NULL;
    }
    
    if (output_file) {
        g_formatter_config.output_file = fopen(output_file, "w");
        g_formatter_config.output_filename = output_file;
    } else {
        g_formatter_config.output_file = NULL;
        g_formatter_config.output_filename = NULL;
    }
}

void asthra_test_formatter_set_verbose(bool verbose) {
    g_formatter_config.verbose_output = verbose;
}

void asthra_test_formatter_set_timing(bool show_timing) {
    g_formatter_config.show_timing = show_timing;
}

void format_test_result(const AsthraTestContext* context) {
    switch (g_formatter_config.format) {
        case ASTHRA_FORMAT_CONSOLE:
        case ASTHRA_FORMAT_PLAIN:
            format_console_test_result(context);
            break;
        case ASTHRA_FORMAT_JSON:
            // JSON formatting needs to be handled at the suite level
            break;
        case ASTHRA_FORMAT_TAP:
            format_tap_test_result(get_output_file(), context);
            break;
        case ASTHRA_FORMAT_JUNIT:
            format_junit_test_result(get_output_file(), context);
            break;
        default:
            format_console_test_result(context);
            break;
    }
}

void format_suite_header(const char* suite_name, size_t total_tests) {
    FILE* output = get_output_file();
    
    switch (g_formatter_config.format) {
        case ASTHRA_FORMAT_CONSOLE:
        case ASTHRA_FORMAT_PLAIN:
            format_console_suite_header(suite_name);
            break;
        case ASTHRA_FORMAT_JSON:
            format_json_suite_header(output, suite_name);
            break;
        case ASTHRA_FORMAT_TAP:
            format_tap_suite_header(output, total_tests);
            break;
        case ASTHRA_FORMAT_JUNIT:
            format_junit_suite_header(output, suite_name, total_tests);
            break;
        default:
            format_console_suite_header(suite_name);
            break;
    }
}

void format_suite_summary(const AsthraTestStatistics* stats) {
    FILE* output = get_output_file();
    
    switch (g_formatter_config.format) {
        case ASTHRA_FORMAT_CONSOLE:
        case ASTHRA_FORMAT_PLAIN:
            format_console_suite_summary(stats);
            break;
        case ASTHRA_FORMAT_JSON:
            format_json_suite_summary(output, stats);
            break;
        case ASTHRA_FORMAT_TAP:
            // TAP doesn't need a summary
            break;
        case ASTHRA_FORMAT_JUNIT:
            format_junit_suite_summary(output, stats);
            break;
        default:
            format_console_suite_summary(stats);
            break;
    }
}

void format_json_test_results(const AsthraTestContext* contexts, size_t count) {
    FILE* output = get_output_file();
    
    for (size_t i = 0; i < count; i++) {
        format_json_test_result(output, &contexts[i], i == 0);
    }
}

void asthra_test_formatter_cleanup(void) {
    if (g_formatter_config.output_file && g_formatter_config.output_file != stdout) {
        fclose(g_formatter_config.output_file);
        g_formatter_config.output_file = NULL;
    }
}

// Legacy compatibility function
void format_test_result_legacy(const char *name, int passed) {
    printf("%s %s\n", 
           passed ? (g_formatter_config.use_colors ? "✅" : "[PASS]") : 
                    (g_formatter_config.use_colors ? "❌" : "[FAIL]"), 
           name);
}