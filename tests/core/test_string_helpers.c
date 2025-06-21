/**
 * Asthra Programming Language v1.2 String Operations Test Helpers Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Implementation of common helpers for string operation tests.
 */

#include "test_string_helpers.h"

// =============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
// =============================================================================

TestString* test_string_create(const char *str) {
    TestString *ts = malloc(sizeof(TestString));
    if (!ts) return NULL;
    
    ts->length = strlen(str);
    ts->capacity = ts->length + 1;
    ts->data = malloc(ts->capacity);
    if (!ts->data) {
        free(ts);
        return NULL;
    }
    
    strcpy(ts->data, str);
    ts->is_gc_managed = false;
    return ts;
}

void test_string_destroy(TestString *ts) {
    if (!ts) return;
    if (ts->data) free(ts->data);
    free(ts);
}

TestString* test_string_concat(const TestString *a, const TestString *b) {
    if (!a || !b) return NULL;
    
    size_t new_length = a->length + b->length;
    TestString *result = malloc(sizeof(TestString));
    if (!result) return NULL;
    
    result->length = new_length;
    result->capacity = new_length + 1;
    result->data = malloc(result->capacity);
    if (!result->data) {
        free(result);
        return NULL;
    }
    
    strcpy(result->data, a->data);
    strcat(result->data, b->data);
    result->is_gc_managed = false;
    return result;
}

// =============================================================================
// HELPER FUNCTION IMPLEMENTATIONS END
// ============================================================================= 
