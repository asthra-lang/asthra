/**
 * Asthra Programming Language Runtime v1.2 - Types Module
 * Result Types and Pattern Matching Engine
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides Result<T,E> type functionality and
 * pattern matching support for error handling.
 */

#ifndef ASTHRA_RUNTIME_RESULT_H
#define ASTHRA_RUNTIME_RESULT_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RESULT<T,E> TYPE AND PATTERN MATCHING ENGINE
// =============================================================================

// Result type representation
typedef enum { ASTHRA_RESULT_OK, ASTHRA_RESULT_ERR } AsthraResultTag;

typedef struct {
    AsthraResultTag tag;
    union {
        struct {
            void *value;
            size_t value_size;
            uint32_t value_type_id;
        } ok;
        struct {
            void *error;
            size_t error_size;
            uint32_t error_type_id;
        } err;
    } data;
    AsthraOwnershipHint ownership;
} AsthraResult;

// =============================================================================
// RESULT CREATION AND BASIC OPERATIONS
// =============================================================================

// Result operations
AsthraResult asthra_result_ok(void *value, size_t value_size, uint32_t type_id,
                              AsthraOwnershipHint ownership);
AsthraResult asthra_result_err(void *error, size_t error_size, uint32_t type_id,
                               AsthraOwnershipHint ownership);
bool asthra_result_is_ok(AsthraResult result);
bool asthra_result_is_err(AsthraResult result);
void *asthra_result_unwrap_ok(AsthraResult result);
void *asthra_result_unwrap_err(AsthraResult result);
AsthraResult asthra_result_clone(AsthraResult result);
void asthra_result_free(AsthraResult result);

// =============================================================================
// RESULT HELPER FUNCTIONS FOR BASIC TYPES
// =============================================================================

// Helper functions for result creation (basic types only)
AsthraResult asthra_result_ok_int64(int64_t value);
AsthraResult asthra_result_ok_uint64(uint64_t value);
AsthraResult asthra_result_ok_double(double value);
AsthraResult asthra_result_ok_bool(bool value);
AsthraResult asthra_result_ok_cstr(const char *cstr);
AsthraResult asthra_result_err_cstr(const char *error_msg);

// Result accessor functions (basic types only)
int64_t asthra_result_unwrap_int64(AsthraResult result);
uint64_t asthra_result_unwrap_uint64(AsthraResult result);
double asthra_result_unwrap_double(AsthraResult result);
bool asthra_result_unwrap_bool(AsthraResult result);

// =============================================================================
// RESULT TRANSFORMATION FUNCTIONS
// =============================================================================

// Result transformation functions
AsthraResult asthra_result_map_ok(AsthraResult result, void *(*mapper)(void *value, void *context),
                                  void *context);
AsthraResult asthra_result_map_err(AsthraResult result, void *(*mapper)(void *error, void *context),
                                   void *context);
AsthraResult asthra_result_and_then(AsthraResult result,
                                    AsthraResult (*func)(void *value, void *context),
                                    void *context);
AsthraResult asthra_result_or_else(AsthraResult result,
                                   AsthraResult (*func)(void *error, void *context), void *context);

// Result utility functions
bool asthra_result_is_ok_and(AsthraResult result, bool (*predicate)(void *value, void *context),
                             void *context);
bool asthra_result_is_err_and(AsthraResult result, bool (*predicate)(void *error, void *context),
                              void *context);
void *asthra_result_unwrap_or(AsthraResult result, void *default_value);
void *asthra_result_unwrap_or_else(AsthraResult result, void *(*func)(void *error, void *context),
                                   void *context);

// =============================================================================
// PATTERN MATCHING SUPPORT
// =============================================================================

// Pattern matching support
typedef enum { ASTHRA_MATCH_OK, ASTHRA_MATCH_ERR, ASTHRA_MATCH_WILDCARD } AsthraMatchPattern;

typedef struct {
    AsthraMatchPattern pattern;
    uint32_t expected_type_id; // 0 for any type
    void (*handler)(void *data, void *context);
    void *context;
} AsthraMatchArm;

int asthra_result_match(AsthraResult result, AsthraMatchArm *arms, size_t arm_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_RESULT_H
