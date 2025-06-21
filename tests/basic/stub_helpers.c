/**
 * Helper and utility stub functions for comprehensive tests
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

// Forward declaration
struct TypeInfo;
typedef struct TypeInfo TypeInfo;

// Test framework stub
bool asthra_test_assert_bool(void* ctx, bool value, const char* msg, ...) {
    (void)ctx;
    (void)msg;
    return value;
}

// Type info functions
TypeInfo* create_type_info_from_descriptor(void* desc) {
    (void)desc;
    return malloc(1);  // Just allocate a minimal stub
}

void* get_builtin_type_descriptor(const char* name) {
    (void)name;
    return malloc(1);
}

void* get_promoted_integer_type(void* type) {
    (void)type;
    return malloc(1);
}

void* get_promoted_type(void* type1, void* type2) {
    (void)type1;
    (void)type2;
    return malloc(1);
}

void* type_info_from_descriptor(void* desc) {
    (void)desc;
    return malloc(1);
}

// Symbol table functions
void* symbol_entry_create(const char* name, void* type) {
    (void)name;
    (void)type;
    return malloc(1);
}

void symbol_entry_destroy(void* entry) {
    free(entry);
}

// Const value functions
bool check_const_dependency_cycle(void* analyzer, void* node) {
    (void)analyzer;
    (void)node;
    return true;
}

void const_value_destroy(void* value) {
    free(value);
}

void* create_const_value(void* type, void* value) {
    (void)type;
    (void)value;
    return malloc(1);
}

bool evaluate_const_expression(void* analyzer, void* expr, void** result) {
    (void)analyzer;
    (void)expr;
    *result = malloc(1);
    return true;
}

void* lookup_const_value(void* table, const char* name) {
    (void)table;
    (void)name;
    return NULL;
}

bool register_const_value(void* table, const char* name, void* value) {
    (void)table;
    (void)name;
    (void)value;
    return true;
}

// Type checking functions
bool is_numeric_type(void* type) {
    (void)type;
    return true;
}

bool is_integral_type(void* type) {
    (void)type;
    return true;
}

bool types_compatible(void* type1, void* type2) {
    (void)type1;
    (void)type2;
    return true;
}

// Module loading
void* resolve_import_path(const char* path) {
    (void)path;
    return NULL;
}

bool load_module(void* analyzer, const char* path) {
    (void)analyzer;
    (void)path;
    return true;
}

// Arena allocator
void* arena_create_child(void* parent) {
    (void)parent;
    return malloc(1);
}

void arena_destroy_child(void* arena) {
    free(arena);
}

// Performance validation
void performance_validation_run_complete(void) {
    // No-op
}