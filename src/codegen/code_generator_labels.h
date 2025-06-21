/**
 * Asthra Programming Language Compiler
 * Code Generation Label Management Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_LABELS_H
#define ASTHRA_CODE_GENERATOR_LABELS_H

#include "code_generator_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// LABEL MANAGEMENT
// =============================================================================

// Label types
typedef enum {
    LABEL_FUNCTION,
    LABEL_BASIC_BLOCK,
    LABEL_LOOP_START,
    LABEL_LOOP_END,
    LABEL_BRANCH_TARGET,
    LABEL_STRING_LITERAL,
    LABEL_COUNT
} LabelType;

// Label entry
typedef struct LabelEntry {
    char *name;
    LabelType type;
    size_t address;  // Relative address in instruction buffer
    bool is_resolved;
    struct LabelEntry *next;  // For hash table chaining
} LabelEntry;

// Label manager with thread-safe operations
struct LabelManager {
    LabelEntry **buckets;
    size_t bucket_count;
    atomic_size_t label_count;
    atomic_uint_fast32_t next_label_id;
    
    // Thread safety
    pthread_rwlock_t rwlock;
};

// =============================================================================
// LABEL MANAGEMENT WITH THREAD SAFETY
// =============================================================================

LabelManager *label_manager_create(size_t initial_capacity);
void label_manager_destroy(LabelManager *manager);

// Thread-safe label operations
char *label_manager_create_label(LabelManager *manager, LabelType type, const char *prefix);
bool label_manager_define_label(LabelManager *manager, const char *name, size_t address);
bool label_manager_resolve_label(LabelManager *manager, const char *name, size_t *address);
bool label_manager_is_defined(LabelManager *manager, const char *name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_LABELS_H 
