/**
 * Asthra Programming Language
 * Gperf Integration Test Common Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_gperf_common.h"

// Sample Asthra keywords for testing
const KeywordTest test_keywords[] = {
    {"fn", 1, true},      {"let", 2, true},    {"mut", 3, true},       {"if", 4, true},
    {"else", 5, true},    {"while", 6, true},  {"for", 7, true},       {"in", 8, true},
    {"return", 9, true},  {"break", 10, true}, {"continue", 11, true}, {"struct", 12, true},
    {"enum", 13, true},   {"impl", 14, true},  {"trait", 15, true},    {"pub", 16, true},
    {"mod", 17, true},    {"use", 18, true},   {"as", 19, true},       {"const", 20, true},
    {"static", 21, true}, {"match", 22, true}, {"true", 23, true},     {"false", 24, true},
    {"null", 25, true},   {"i8", 26, true},    {"i16", 27, true},      {"i32", 28, true},
    {"i64", 29, true},    {"u8", 30, true},    {"u16", 31, true},      {"u32", 32, true},
    {"u64", 33, true},    {"f32", 34, true},   {"f64", 35, true},      {"bool", 36, true},
    {"char", 37, true},   {"str", 38, true},   {"self", 39, true},     {"Self", 40, true}};

const size_t test_keywords_count = sizeof(test_keywords) / sizeof(test_keywords[0]);

// Common utility functions
bool gperf_file_exists(const char *filepath) {
    return access(filepath, F_OK) == 0;
}

bool check_file_substantial(const char *filepath, size_t min_size) {
    struct stat st;
    if (stat(filepath, &st) != 0) {
        return false;
    }
    return st.st_size >= (off_t)min_size;
}

int run_command_with_timeout(const char *command, double timeout_seconds) {
    // For simplicity, using system() directly
    // In a production environment, you might want proper timeout handling
    (void)timeout_seconds; // Suppress unused parameter warning
    int result = system(command);
    if (result == -1) {
        return -1; // System call failed
    }
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
}
