/**
 * Asthra Programming Language Compiler
 * Argument list management with C17 flexible arrays
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../compiler.h"

// C17 compile-time validation of implementation assumptions
ASTHRA_STATIC_ASSERT(sizeof(void*) >= sizeof(size_t), 
                     "Pointer size must be at least as large as size_t");
ASTHRA_STATIC_ASSERT(sizeof(AsthraCompilerOptions) > 0, 
                     "AsthraCompilerOptions must have non-zero size");

// =============================================================================
// ARGUMENT LIST MANAGEMENT (C17 FLEXIBLE ARRAYS)
// =============================================================================

struct AsthraArgumentList *asthra_argument_list_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 8; // Default capacity
    }
    
#if ASTHRA_C17_AVAILABLE
    // Use C17 flexible array member
    size_t size = sizeof(struct AsthraArgumentList) + initial_capacity * sizeof(const char*);
    struct AsthraArgumentList *list = malloc(size);
    if (!list) {
        return NULL;
    }
    
    // C17 designated initializer
    *list = (struct AsthraArgumentList) {
        .count = 0,
        .capacity = initial_capacity
    };
#else
    // Fallback for older compilers
    struct AsthraArgumentList *list = malloc(sizeof(struct AsthraArgumentList));
    if (!list) {
        return NULL;
    }
    
    list->arguments = malloc(initial_capacity * sizeof(const char*));
    if (!list->arguments) {
        free(list);
        return NULL;
    }
    
    list->count = 0;
    list->capacity = initial_capacity;
#endif
    
    return list;
}

void asthra_argument_list_destroy(struct AsthraArgumentList *list) {
    if (!list) {
        return;
    }
    
#if !ASTHRA_C17_AVAILABLE
    // Only need to free arguments array in fallback mode
    free(list->arguments);
#endif
    
    free(list);
}

bool asthra_argument_list_add(struct AsthraArgumentList **list_ptr, const char *argument) {
    if (!list_ptr || !*list_ptr || !argument) {
        return false;
    }
    
    struct AsthraArgumentList *list = *list_ptr;
    
    // Resize if needed
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity * 2;
        
#if ASTHRA_C17_AVAILABLE
        // Reallocate with flexible array member
        size_t new_size = sizeof(struct AsthraArgumentList) + new_capacity * sizeof(const char*);
        struct AsthraArgumentList *new_list = realloc(list, new_size);
        if (!new_list) {
            return false;
        }
        new_list->capacity = new_capacity;
        *list_ptr = new_list;
        list = new_list;
#else
        // Reallocate arguments array in fallback mode
        const char **new_arguments = realloc(list->arguments, new_capacity * sizeof(const char*));
        if (!new_arguments) {
            return false;
        }
        list->arguments = new_arguments;
        list->capacity = new_capacity;
#endif
    }
    
    list->arguments[list->count++] = argument;
    return true;
}

size_t asthra_argument_list_count(const struct AsthraArgumentList *list) {
    return list ? list->count : 0;
}

const char *asthra_argument_list_get(const struct AsthraArgumentList *list, size_t index) {
    if (!list || index >= list->count) {
        return NULL;
    }
    return list->arguments[index];
} 
