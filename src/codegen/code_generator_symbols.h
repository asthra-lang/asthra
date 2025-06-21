/**
 * Asthra Programming Language Compiler
 * Code Generation Symbol Table Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_SYMBOLS_H
#define ASTHRA_CODE_GENERATOR_SYMBOLS_H

#include "code_generator_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SYMBOL TABLE AND LOCAL VARIABLES
// =============================================================================

// Local variable information for code generation
typedef struct LocalVariable {
    char *name;
    TypeDescriptor *type;
    int32_t offset;  // Stack offset from RBP
    size_t size;     // Size in bytes
    bool is_parameter;
    struct LocalVariable *next;  // For linked list or hash table
} LocalVariable;

// Symbol table for local variables
typedef struct LocalSymbolTable {
    LocalVariable **buckets;
    size_t bucket_count;
    size_t variable_count;
    int32_t current_stack_offset;
} LocalSymbolTable;

// =============================================================================
// SYMBOL TABLE OPERATIONS
// =============================================================================

LocalSymbolTable *local_symbol_table_create(size_t initial_capacity);
void local_symbol_table_destroy(LocalSymbolTable *table);
bool add_local_variable(CodeGenerator *generator, const char *name, TypeDescriptor *type, size_t size);
bool add_function_parameter(CodeGenerator *generator, const char *name, TypeDescriptor *type, size_t size, int32_t offset);
bool remove_local_variable(CodeGenerator *generator, const char *name);
LocalVariable *find_local_variable(CodeGenerator *generator, const char *name);
LocalVariable *create_local_variable(CodeGenerator *generator, const char *name, Register reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_SYMBOLS_H 
