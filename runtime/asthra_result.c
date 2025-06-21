/**
 * Asthra Programming Language Result Type v1.2
 * Pattern Matching Engine for Result<T,E> Types
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * IMPLEMENTATION FEATURES:
 * - Result<T,E> type operations
 * - Pattern matching engine with custom handlers
 * - Memory-safe result handling
 * - C17 compound literal initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "asthra_runtime.h"

// =============================================================================
// RESULT TYPE OPERATIONS
// =============================================================================

AsthraResult asthra_result_ok(void *value, size_t value_size, uint32_t type_id, AsthraOwnershipHint ownership) {
    // Use C17 compound literal with designated initializers
    return (AsthraResult){
        .tag = ASTHRA_RESULT_OK,
        .data.ok = {
            .value = value,
            .value_size = value_size,
            .value_type_id = type_id
        },
        .ownership = ownership
    };
}

AsthraResult asthra_result_err(void *error, size_t error_size, uint32_t type_id, AsthraOwnershipHint ownership) {
    // Use C17 compound literal with designated initializers
    return (AsthraResult){
        .tag = ASTHRA_RESULT_ERR,
        .data.err = {
            .error = error,
            .error_size = error_size,
            .error_type_id = type_id
        },
        .ownership = ownership
    };
}

bool asthra_result_is_ok(AsthraResult result) {
    return result.tag == ASTHRA_RESULT_OK;
}

bool asthra_result_is_err(AsthraResult result) {
    return result.tag == ASTHRA_RESULT_ERR;
}

void *asthra_result_unwrap_ok(AsthraResult result) {
    if (result.tag != ASTHRA_RESULT_OK) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_TYPE_MISMATCH, "Attempted to unwrap error as ok");
        return NULL;
    }
    return result.data.ok.value;
}

void *asthra_result_unwrap_err(AsthraResult result) {
    if (result.tag != ASTHRA_RESULT_ERR) {
        ASTHRA_SET_ERROR(ASTHRA_ERROR_TYPE_MISMATCH, "Attempted to unwrap ok as error");
        return NULL;
    }
    return result.data.err.error;
}

int asthra_result_match(AsthraResult result, AsthraMatchArm *arms, size_t arm_count) {
    for (size_t i = 0; i < arm_count; i++) {
        AsthraMatchArm *arm = &arms[i];
        bool matches = false;

        switch (arm->pattern) {
            case ASTHRA_MATCH_OK:
                matches = (result.tag == ASTHRA_RESULT_OK);
                if (matches && arm->expected_type_id != 0) {
                    matches = (result.data.ok.value_type_id == arm->expected_type_id);
                }
                break;
            case ASTHRA_MATCH_ERR:
                matches = (result.tag == ASTHRA_RESULT_ERR);
                if (matches && arm->expected_type_id != 0) {
                    matches = (result.data.err.error_type_id == arm->expected_type_id);
                }
                break;
            case ASTHRA_MATCH_WILDCARD:
                matches = true;
                break;
        }

        if (matches && arm->handler) {
            void *data = (result.tag == ASTHRA_RESULT_OK) ? 
                         result.data.ok.value : result.data.err.error;
            arm->handler(data, arm->context);
            return (int)i;
        }
    }

    return -1; // No match found
}

// =============================================================================
// RESULT TYPE CLEANUP IMPLEMENTATION
// =============================================================================

AsthraResult asthra_result_clone(AsthraResult result) {
    // For now, just return a copy - proper cloning would need deep copy
    return result;
}

void asthra_result_free(AsthraResult result) {
    if (result.ownership == ASTHRA_OWNERSHIP_GC) {
        if (result.tag == ASTHRA_RESULT_OK && result.data.ok.value) {
            asthra_free(result.data.ok.value, ASTHRA_ZONE_GC);
        } else if (result.tag == ASTHRA_RESULT_ERR && result.data.err.error) {
            asthra_free(result.data.err.error, ASTHRA_ZONE_GC);
        }
    }
}

// =============================================================================
// HELPER FUNCTIONS FOR RESULT CREATION
// =============================================================================

// Forward declaration to resolve circular dependency
AsthraResult asthra_result_err_cstr(const char *error_msg);

AsthraResult asthra_result_ok_int64(int64_t value) {
    int64_t *value_ptr = asthra_alloc(sizeof(int64_t), ASTHRA_ZONE_GC);
    if (!value_ptr) {
        return asthra_result_err_cstr("Failed to allocate memory for int64 value");
    }
    *value_ptr = value;
    return asthra_result_ok(value_ptr, sizeof(int64_t), ASTHRA_TYPE_I64, ASTHRA_OWNERSHIP_GC);
}

AsthraResult asthra_result_ok_uint64(uint64_t value) {
    uint64_t *value_ptr = asthra_alloc(sizeof(uint64_t), ASTHRA_ZONE_GC);
    if (!value_ptr) {
        return asthra_result_err_cstr("Failed to allocate memory for uint64 value");
    }
    *value_ptr = value;
    return asthra_result_ok(value_ptr, sizeof(uint64_t), ASTHRA_TYPE_U64, ASTHRA_OWNERSHIP_GC);
}

AsthraResult asthra_result_ok_double(double value) {
    double *value_ptr = asthra_alloc(sizeof(double), ASTHRA_ZONE_GC);
    if (!value_ptr) {
        return asthra_result_err_cstr("Failed to allocate memory for double value");
    }
    *value_ptr = value;
    return asthra_result_ok(value_ptr, sizeof(double), ASTHRA_TYPE_F64, ASTHRA_OWNERSHIP_GC);
}

AsthraResult asthra_result_ok_bool(bool value) {
    bool *value_ptr = asthra_alloc(sizeof(bool), ASTHRA_ZONE_GC);
    if (!value_ptr) {
        return asthra_result_err_cstr("Failed to allocate memory for bool value");
    }
    *value_ptr = value;
    return asthra_result_ok(value_ptr, sizeof(bool), ASTHRA_TYPE_BOOL, ASTHRA_OWNERSHIP_GC);
}

AsthraResult asthra_result_ok_string(AsthraString str) {
    AsthraString *str_ptr = asthra_alloc(sizeof(AsthraString), ASTHRA_ZONE_GC);
    if (!str_ptr) {
        return asthra_result_err_cstr("Failed to allocate memory for string value");
    }
    *str_ptr = str;
    return asthra_result_ok(str_ptr, sizeof(AsthraString), ASTHRA_TYPE_STRING, ASTHRA_OWNERSHIP_GC);
}

AsthraResult asthra_result_ok_cstr(const char *cstr) {
    AsthraString str = asthra_string_new(cstr);
    return asthra_result_ok_string(str);
}

AsthraResult asthra_result_err_cstr(const char *error_msg) {
    AsthraString error_str = asthra_string_new(error_msg);
    AsthraString *error_ptr = asthra_alloc(sizeof(AsthraString), ASTHRA_ZONE_GC);
    if (!error_ptr) {
        // Fallback error if we can't allocate memory for the error itself
        static AsthraString fallback_error = {0};
        return asthra_result_err(&fallback_error, sizeof(AsthraString), ASTHRA_TYPE_STRING, ASTHRA_OWNERSHIP_C);
    }
    *error_ptr = error_str;
    return asthra_result_err(error_ptr, sizeof(AsthraString), ASTHRA_TYPE_STRING, ASTHRA_OWNERSHIP_GC);
}

AsthraResult asthra_result_err_string(AsthraString error_str) {
    AsthraString *error_ptr = asthra_alloc(sizeof(AsthraString), ASTHRA_ZONE_GC);
    if (!error_ptr) {
        // Fallback error if we can't allocate memory for the error itself
        static AsthraString fallback_error = {0};
        return asthra_result_err(&fallback_error, sizeof(AsthraString), ASTHRA_TYPE_STRING, ASTHRA_OWNERSHIP_C);
    }
    *error_ptr = error_str;
    return asthra_result_err(error_ptr, sizeof(AsthraString), ASTHRA_TYPE_STRING, ASTHRA_OWNERSHIP_GC);
}

// =============================================================================
// RESULT ACCESSOR FUNCTIONS
// =============================================================================

int64_t asthra_result_unwrap_int64(AsthraResult result) {
    void *value = asthra_result_unwrap_ok(result);
    if (!value) return 0;
    return *(int64_t*)value;
}

uint64_t asthra_result_unwrap_uint64(AsthraResult result) {
    void *value = asthra_result_unwrap_ok(result);
    if (!value) return 0;
    return *(uint64_t*)value;
}

double asthra_result_unwrap_double(AsthraResult result) {
    void *value = asthra_result_unwrap_ok(result);
    if (!value) return 0.0;
    return *(double*)value;
}

bool asthra_result_unwrap_bool(AsthraResult result) {
    void *value = asthra_result_unwrap_ok(result);
    if (!value) return false;
    return *(bool*)value;
}

AsthraString asthra_result_unwrap_string(AsthraResult result) {
    void *value = asthra_result_unwrap_ok(result);
    if (!value) return (AsthraString){0};
    return *(AsthraString*)value;
}

AsthraString asthra_result_unwrap_err_string(AsthraResult result) {
    void *error = asthra_result_unwrap_err(result);
    if (!error) return (AsthraString){0};
    return *(AsthraString*)error;
}

// =============================================================================
// RESULT TRANSFORMATION FUNCTIONS
// =============================================================================

AsthraResult asthra_result_map_ok(AsthraResult result, void *(*mapper)(void *value, void *context), void *context) {
    if (result.tag != ASTHRA_RESULT_OK) {
        return result; // Pass through errors unchanged
    }
    
    void *mapped_value = mapper(result.data.ok.value, context);
    if (!mapped_value) {
        return asthra_result_err_cstr("Mapping function returned NULL");
    }
    
    return asthra_result_ok(mapped_value, result.data.ok.value_size, 
                           result.data.ok.value_type_id, result.ownership);
}

AsthraResult asthra_result_map_err(AsthraResult result, void *(*mapper)(void *error, void *context), void *context) {
    if (result.tag != ASTHRA_RESULT_ERR) {
        return result; // Pass through Ok values unchanged
    }
    
    void *mapped_error = mapper(result.data.err.error, context);
    if (!mapped_error) {
        return asthra_result_err_cstr("Error mapping function returned NULL");
    }
    
    return asthra_result_err(mapped_error, result.data.err.error_size, 
                            result.data.err.error_type_id, result.ownership);
}

AsthraResult asthra_result_and_then(AsthraResult result, AsthraResult (*func)(void *value, void *context), void *context) {
    if (result.tag != ASTHRA_RESULT_OK) {
        return result; // Pass through errors unchanged
    }
    
    return func(result.data.ok.value, context);
}

AsthraResult asthra_result_or_else(AsthraResult result, AsthraResult (*func)(void *error, void *context), void *context) {
    if (result.tag != ASTHRA_RESULT_ERR) {
        return result; // Pass through Ok values unchanged
    }
    
    return func(result.data.err.error, context);
}

// =============================================================================
// RESULT UTILITY FUNCTIONS
// =============================================================================

bool asthra_result_is_ok_and(AsthraResult result, bool (*predicate)(void *value, void *context), void *context) {
    if (result.tag != ASTHRA_RESULT_OK) {
        return false;
    }
    
    return predicate ? predicate(result.data.ok.value, context) : true;
}

bool asthra_result_is_err_and(AsthraResult result, bool (*predicate)(void *error, void *context), void *context) {
    if (result.tag != ASTHRA_RESULT_ERR) {
        return false;
    }
    
    return predicate ? predicate(result.data.err.error, context) : true;
}

void *asthra_result_unwrap_or(AsthraResult result, void *default_value) {
    if (result.tag == ASTHRA_RESULT_OK) {
        return result.data.ok.value;
    }
    return default_value;
}

void *asthra_result_unwrap_or_else(AsthraResult result, void *(*func)(void *error, void *context), void *context) {
    if (result.tag == ASTHRA_RESULT_OK) {
        return result.data.ok.value;
    }
    return func ? func(result.data.err.error, context) : NULL;
}
