/**
 * Utility stub implementations
 * Minimal implementations to satisfy the linker for testing
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>

// Test framework stubs
bool asthra_test_assert_bool(void* ctx, bool value, const char* msg, ...) {
    (void)ctx;
    (void)msg;
    return value;
}

// Symbol table stubs  
void* symbol_table_push_scope(void* table) {
    (void)table;
    return table;
}

void symbol_table_pop_scope(void* table) {
    (void)table;
}

bool symbol_table_add_symbol(void* table, const char* name, void* info) {
    (void)table;
    (void)name;
    (void)info;
    return true;
}

bool symbol_table_add_alias(void* table, const char* alias, const char* target) {
    (void)table;
    (void)alias;
    (void)target;
    return true;
}

// Symbol entry stubs
void* symbol_entry_create(const char* name, void* type, int kind) {
    (void)name;
    (void)type;
    (void)kind;
    return malloc(1);
}

void symbol_entry_destroy(void* entry) {
    free(entry);
}

// Arena allocator stubs
void* arena_alloc(void* arena, size_t size) {
    (void)arena;
    return malloc(size);
}

void arena_reset(void* arena) {
    (void)arena;
}

void* arena_create(size_t size) {
    (void)size;
    return malloc(1);
}

void arena_destroy(void* arena) {
    free(arena);
}