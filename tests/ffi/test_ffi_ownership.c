/**
 * Asthra Programming Language v1.2 Enhanced FFI Tests
 * Ownership Transfer Testing
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for ownership transfer mechanisms including borrow, move, and copy semantics.
 */

#include "../core/test_comprehensive.h"
#include "../runtime/ffi.h"
#include "../runtime/memory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// =============================================================================
// OWNERSHIP TRANSFER DATA STRUCTURES
// =============================================================================

// Mock ownership transfer annotations
typedef enum {
    FFI_OWNERSHIP_BORROW,
    FFI_OWNERSHIP_MOVE,
    FFI_OWNERSHIP_COPY
} FFIOwnershipMode;

// Mock FFI call context
typedef struct {
    FFIOwnershipMode ownership_mode;
    bool memory_managed;
    size_t data_size;
    void *cleanup_func;
} FFICallContext;

typedef void (*cleanup_c_func_t)(void*);

static void test_c_cleanup(void *ptr) {
    if (ptr) free(ptr);
}

// =============================================================================
// OWNERSHIP TRANSFER TESTS
// =============================================================================

AsthraTestResult test_ffi_ownership_borrow(AsthraV12TestContext *ctx) {
    // Test borrow ownership mode
    
    FFICallContext borrow_ctx = {
        .ownership_mode = FFI_OWNERSHIP_BORROW,
        .memory_managed = false,
        .data_size = 0,
        .cleanup_func = NULL
    };
    
    // Test borrowing a string
    char test_string[] = "Borrowed string";
    
    // Mock function that borrows string (doesn't take ownership)
    size_t get_string_length(const char *str, FFICallContext *ctx) {
        // Verify borrow mode
        if (ctx->ownership_mode != FFI_OWNERSHIP_BORROW) {
            return 0;
        }
        
        return str ? strlen(str) : 0;
    }
    
    size_t length = get_string_length(test_string, &borrow_ctx);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, length == strlen(test_string),
                           "Borrowed string length should be %zu, got %zu", 
                           strlen(test_string), length)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Original string should still be valid after borrow
    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(test_string, "Borrowed string") == 0,
                           "Original string should remain unchanged after borrow")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test borrowing with multiple references
    const char *ref1 = test_string;
    const char *ref2 = test_string;
    
    size_t len1 = get_string_length(ref1, &borrow_ctx);
    size_t len2 = get_string_length(ref2, &borrow_ctx);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, len1 == len2,
                           "Multiple borrows should return same result")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_ownership_move(AsthraV12TestContext *ctx) {
    // Test move ownership mode
    
    FFICallContext move_ctx = {
        .ownership_mode = FFI_OWNERSHIP_MOVE,
        .memory_managed = true,
        .data_size = 0,
        .cleanup_func = test_c_cleanup
    };
    
    // Test moving ownership of allocated memory
    char *allocated_string = malloc(50);
    if (!allocated_string) {
        return ASTHRA_TEST_ERROR;
    }
    
    strcpy(allocated_string, "Moved string");
    
    // Mock function that takes ownership
    char* take_ownership(char *str, FFICallContext *ctx) {
        if (ctx->ownership_mode != FFI_OWNERSHIP_MOVE) {
            return NULL;
        }
        
        // Function now owns the string
        ctx->data_size = strlen(str) + 1;
        
        // Transform the string
        for (char *p = str; *p; p++) {
            if (*p >= 'a' && *p <= 'z') {
                *p = *p - 'a' + 'A';
            }
        }
        
        return str; // Return the same pointer, now owned by callee
    }
    
    char *moved_string = take_ownership(allocated_string, &move_ctx);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, moved_string != NULL,
                           "Move operation should return non-NULL")) {
        free(allocated_string);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(moved_string, "MOVED STRING") == 0,
                           "Moved string should be uppercase")) {
        free(moved_string);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, move_ctx.data_size == 13,
                           "Move context should track data size")) {
        free(moved_string);
        return ASTHRA_TEST_FAIL;
    }
    
    // Cleanup using context
    if (move_ctx.cleanup_func) {
        ((cleanup_c_func_t)move_ctx.cleanup_func)(moved_string);
    }
    
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_ffi_ownership_copy(AsthraV12TestContext *ctx) {
    // Test copy ownership mode
    
    FFICallContext copy_ctx = {
        .ownership_mode = FFI_OWNERSHIP_COPY,
        .memory_managed = true,
        .data_size = 0,
        .cleanup_func = test_c_cleanup
    };
    
    // Test copying data
    int original_data[5] = {10, 20, 30, 40, 50};
    
    // Mock function that copies data
    int* copy_array(const int *src, size_t count, FFICallContext *ctx) {
        if (ctx->ownership_mode != FFI_OWNERSHIP_COPY) {
            return NULL;
        }
        
        int *copy = malloc(count * sizeof(int));
        if (!copy) return NULL;
        
        memcpy(copy, src, count * sizeof(int));
        ctx->data_size = count * sizeof(int);
        
        return copy;
    }
    
    int *copied_array = copy_array(original_data, 5, &copy_ctx);
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, copied_array != NULL,
                           "Copy operation should return non-NULL")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, copied_array != original_data,
                           "Copied array should be different pointer")) {
        free(copied_array);
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify copy contents
    for (int i = 0; i < 5; i++) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, copied_array[i] == original_data[i],
                               "Copied element %d should match original", i)) {
            free(copied_array);
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Modify original to verify independence
    original_data[0] = 999;
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, copied_array[0] == 10,
                           "Copied array should be independent of original")) {
        free(copied_array);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, copy_ctx.data_size == 5 * sizeof(int),
                           "Copy context should track correct size")) {
        free(copied_array);
        return ASTHRA_TEST_FAIL;
    }
    
    // Cleanup
    if (copy_ctx.cleanup_func) {
        ((cleanup_c_func_t)copy_ctx.cleanup_func)(copied_array);
    }
    
    return ASTHRA_TEST_PASS;
} 
