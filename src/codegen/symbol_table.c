/**
 * Asthra Programming Language Compiler
 * Symbol Table Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "code_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Hash function for symbol names
static size_t hash_string(const char *str) {
    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (size_t)c;
    }
    return hash;
}

// =============================================================================
// SYMBOL TABLE OPERATIONS
// =============================================================================

LocalSymbolTable *local_symbol_table_create(size_t initial_capacity) {
    LocalSymbolTable *table = malloc(sizeof(LocalSymbolTable));
    if (!table) return NULL;
    
    table->buckets = calloc(initial_capacity, sizeof(LocalVariable*));
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    
    table->bucket_count = initial_capacity;
    table->variable_count = 0;
    table->current_stack_offset = 0;
    
    return table;
}

void local_symbol_table_destroy(LocalSymbolTable *table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        LocalVariable *var = table->buckets[i];
        while (var) {
            LocalVariable *next = var->next;
            free(var->name);
            free(var);
            var = next;
        }
    }
    
    free(table->buckets);
    free(table);
}

LocalVariable *find_local_variable(CodeGenerator *generator, const char *name) {
    if (!generator || !generator->symbol_table || !name) return NULL;
    
    LocalSymbolTable *table = generator->symbol_table;
    size_t hash = hash_string(name) % table->bucket_count;
    
    LocalVariable *var = table->buckets[hash];
    while (var) {
        if (strcmp(var->name, name) == 0) {
            return var;
        }
        var = var->next;
    }
    
    return NULL;
}

bool add_local_variable(CodeGenerator *generator, const char *name, TypeDescriptor *type, size_t size) {
    if (!generator || !generator->symbol_table || !name) return false;
    
    LocalSymbolTable *table = generator->symbol_table;
    
    // Check if variable already exists
    if (find_local_variable(generator, name)) {
        return false; // Variable already exists
    }
    
    LocalVariable *var = malloc(sizeof(LocalVariable));
    if (!var) return false;
    
    var->name = strdup(name);
    if (!var->name) {
        free(var);
        return false;
    }
    
    var->type = type;
    var->size = size;
    var->is_parameter = false;
    
    // Allocate stack space (growing downward)
    table->current_stack_offset -= (int32_t)size;
    var->offset = table->current_stack_offset;
    
    // Add to hash table
    size_t hash = hash_string(name) % table->bucket_count;
    var->next = table->buckets[hash];
    table->buckets[hash] = var;
    table->variable_count++;
    
    return true;
}

bool add_function_parameter(CodeGenerator *generator, const char *name, TypeDescriptor *type, size_t size, int32_t offset) {
    if (!generator || !generator->symbol_table || !name) return false;
    
    LocalSymbolTable *table = generator->symbol_table;
    
    // Check if parameter already exists
    if (find_local_variable(generator, name)) {
        return false; // Parameter already exists
    }
    
    LocalVariable *var = malloc(sizeof(LocalVariable));
    if (!var) return false;
    
    var->name = strdup(name);
    if (!var->name) {
        free(var);
        return false;
    }
    
    var->type = type;
    var->size = size;
    var->is_parameter = true;
    var->offset = offset;
    
    // Add to hash table
    size_t hash = hash_string(name) % table->bucket_count;
    var->next = table->buckets[hash];
    table->buckets[hash] = var;
    table->variable_count++;
    
    return true;
}

bool remove_local_variable(CodeGenerator *generator, const char *name) {
    if (!generator || !generator->symbol_table || !name) return false;
    
    LocalSymbolTable *table = generator->symbol_table;
    size_t hash = hash_string(name) % table->bucket_count;
    
    LocalVariable *var = table->buckets[hash];
    LocalVariable *prev = NULL;
    
    while (var) {
        if (strcmp(var->name, name) == 0) {
            if (prev) {
                prev->next = var->next;
            } else {
                table->buckets[hash] = var->next;
            }
            
            free(var->name);
            free(var);
            table->variable_count--;
            return true;
        }
        
        prev = var;
        var = var->next;
    }
    
    return false; // Variable not found
}

LocalVariable *create_local_variable(CodeGenerator *generator, const char *name, Register reg) {
    if (!generator || !generator->symbol_table || !name) return NULL;
    
    LocalSymbolTable *table = generator->symbol_table;
    
    // Check if variable already exists
    if (find_local_variable(generator, name)) {
        return NULL; // Variable already exists
    }
    
    LocalVariable *var = malloc(sizeof(LocalVariable));
    if (!var) return NULL;
    
    var->name = strdup(name);
    if (!var->name) {
        free(var);
        return NULL;
    }
    
    var->type = NULL; // Type will be inferred from context
    var->size = 8;    // Default to 8 bytes (pointer size)
    var->is_parameter = false;
    
    // Allocate stack space (growing downward)
    table->current_stack_offset -= (int32_t)var->size;
    var->offset = table->current_stack_offset;
    
    // Add to hash table
    size_t hash = hash_string(name) % table->bucket_count;
    var->next = table->buckets[hash];
    table->buckets[hash] = var;
    table->variable_count++;
    
    return var;
} 
