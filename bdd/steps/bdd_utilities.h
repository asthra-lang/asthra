#ifndef BDD_UTILITIES_H
#define BDD_UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Common BDD Utilities
 * Provides reusable functions for BDD test implementations
 */

// Command execution utilities
char* bdd_execute_command(const char* command, int* exit_code);

// File operations utilities
char* bdd_read_file_contents(const char* filename);
int bdd_write_file_contents(const char* filename, const char* content);

// Compiler utilities
const char* bdd_find_asthra_compiler(void);
int bdd_compiler_available(void);

// String cleanup utilities
void bdd_cleanup_string(char** str);
void bdd_cleanup_strings(char** strings, int count);

// Output validation utilities
int bdd_output_contains(const char* output, const char* expected);
int bdd_output_not_contains(const char* output, const char* unexpected);
void bdd_assert_output_contains(const char* output, const char* expected);
void bdd_assert_output_not_contains(const char* output, const char* unexpected);

// Source file management
void bdd_create_temp_source_file(const char* filename, const char* content);
void bdd_create_temp_directory(const char* dirname);
void bdd_cleanup_temp_files(void);

// Compilation utilities
int bdd_compile_source_file(const char* source_file, const char* output_file, const char* flags);

// Get temp file paths
const char* bdd_get_temp_source_file(void);
const char* bdd_get_temp_executable(void);

#endif // BDD_UTILITIES_H