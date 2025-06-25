/**
 * Asthra Programming Language Runtime v1.2 - Enum Support Module
 * Enum Runtime Support and Tagged Union Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides runtime support for enum variant construction,
 * pattern matching, and Result/Option type operations.
 */

#ifndef ASTHRA_ENUM_SUPPORT_H
#define ASTHRA_ENUM_SUPPORT_H

#include "core/asthra_runtime_core.h"
#include "types/asthra_runtime_result.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ENUM VARIANT RUNTIME SUPPORT
// =============================================================================

/**
 * Generic enum variant structure for tagged unions
 */
typedef struct {
    uint32_t tag; // Variant tag (0-based)
    union {
        void *value_ptr;       // Pointer to associated data
        uint64_t value_inline; // Inline value for small types
    } data;
    size_t value_size;             // Size of associated data
    uint32_t value_type_id;        // Runtime type identifier
    AsthraOwnershipHint ownership; // Memory ownership semantics
} AsthraEnumVariant;

// =============================================================================
// BASIC ENUM OPERATIONS
// =============================================================================

/**
 * Check if enum variant matches expected tag
 * @param enum_value Pointer to enum variant
 * @param expected_tag Expected variant tag
 * @return true if tag matches, false otherwise
 */
bool Asthra_enum_is_variant(void *enum_value, uint32_t expected_tag);

/**
 * Get associated data from enum variant
 * @param enum_value Pointer to enum variant
 * @return Pointer to associated data or NULL if none
 */
void *Asthra_enum_get_data(void *enum_value);

/**
 * Get variant tag from enum value
 * @param enum_value Pointer to enum variant
 * @return Variant tag value
 */
uint32_t Asthra_enum_get_tag(void *enum_value);

/**
 * Get size of associated data
 * @param enum_value Pointer to enum variant
 * @return Size of associated data in bytes
 */
size_t Asthra_enum_get_data_size(void *enum_value);

/**
 * Create enum variant with associated data
 * @param tag Variant tag
 * @param data Pointer to associated data (can be NULL)
 * @param data_size Size of data in bytes
 * @param type_id Runtime type identifier
 * @param ownership Memory ownership semantics
 * @return Created enum variant
 */
AsthraEnumVariant Asthra_enum_create_variant(uint32_t tag, void *data, size_t data_size,
                                             uint32_t type_id, AsthraOwnershipHint ownership);

/**
 * Free enum variant and associated resources
 * @param variant Pointer to enum variant to free
 */
void Asthra_enum_free_variant(AsthraEnumVariant *variant);

// =============================================================================
// RESULT<T,E> SPECIFIC FUNCTIONS
// =============================================================================

/**
 * Check if Result is Ok variant
 * @param result Pointer to Result enum
 * @return true if Ok, false if Err
 */
bool Asthra_result_is_ok(void *result);

/**
 * Check if Result is Err variant
 * @param result Pointer to Result enum
 * @return true if Err, false if Ok
 */
bool Asthra_result_is_err(void *result);

/**
 * Unwrap Ok value from Result (unsafe - caller must verify it's Ok)
 * @param result Pointer to Result enum
 * @return Pointer to Ok value
 */
void *Asthra_result_unwrap_ok(void *result);

/**
 * Unwrap Err value from Result (unsafe - caller must verify it's Err)
 * @param result Pointer to Result enum
 * @return Pointer to Err value
 */
void *Asthra_result_unwrap_err(void *result);

/**
 * Create Result.Ok variant
 * @param value Pointer to Ok value
 * @param value_size Size of value
 * @param value_type_id Type ID of value
 * @param ownership Memory ownership
 * @return Result enum with Ok variant
 */
AsthraEnumVariant Asthra_result_create_ok(void *value, size_t value_size, uint32_t value_type_id,
                                          AsthraOwnershipHint ownership);

/**
 * Create Result.Err variant
 * @param error Pointer to error value
 * @param error_size Size of error
 * @param error_type_id Type ID of error
 * @param ownership Memory ownership
 * @return Result enum with Err variant
 */
AsthraEnumVariant Asthra_result_create_err(void *error, size_t error_size, uint32_t error_type_id,
                                           AsthraOwnershipHint ownership);

// =============================================================================
// OPTION<T> SPECIFIC FUNCTIONS
// =============================================================================

/**
 * Check if Option is Some variant
 * @param option Pointer to Option enum
 * @return true if Some, false if None
 */
bool Asthra_option_is_some(void *option);

/**
 * Check if Option is None variant
 * @param option Pointer to Option enum
 * @return true if None, false if Some
 */
bool Asthra_option_is_none(void *option);

/**
 * Unwrap Some value from Option (unsafe - caller must verify it's Some)
 * @param option Pointer to Option enum
 * @return Pointer to Some value
 */
void *Asthra_option_unwrap(void *option);

/**
 * Create Option.Some variant
 * @param value Pointer to Some value
 * @param value_size Size of value
 * @param value_type_id Type ID of value
 * @param ownership Memory ownership
 * @return Option enum with Some variant
 */
AsthraEnumVariant Asthra_option_create_some(void *value, size_t value_size, uint32_t value_type_id,
                                            AsthraOwnershipHint ownership);

/**
 * Create Option.None variant
 * @return Option enum with None variant
 */
AsthraEnumVariant Asthra_option_create_none(void);

// =============================================================================
// PATTERN MATCHING SUPPORT
// =============================================================================

/**
 * Pattern match callback function type
 * @param variant Pointer to enum variant being matched
 * @param user_data User-provided context data
 * @return Match result (implementation-specific)
 */
typedef int (*AsthraEnumMatchCallback)(AsthraEnumVariant *variant, void *user_data);

/**
 * Pattern match arm structure
 */
typedef struct {
    uint32_t tag;                     // Tag to match (or UINT32_MAX for wildcard)
    AsthraEnumMatchCallback callback; // Callback function for this arm
    void *user_data;                  // User data passed to callback
} AsthraEnumMatchArm;

/**
 * Execute pattern matching on enum variant
 * @param variant Pointer to enum variant
 * @param arms Array of match arms
 * @param arm_count Number of match arms
 * @return Result from matched callback or -1 if no match
 */
int Asthra_enum_pattern_match(AsthraEnumVariant *variant, AsthraEnumMatchArm *arms,
                              size_t arm_count);

// =============================================================================
// ENUM VARIANT CONSTANTS
// =============================================================================

// Standard Result<T,E> variant tags
#define ASTHRA_RESULT_TAG_OK 0
#define ASTHRA_RESULT_TAG_ERR 1

// Standard Option<T> variant tags
#define ASTHRA_OPTION_TAG_SOME 0
#define ASTHRA_OPTION_TAG_NONE 1

// Wildcard tag for pattern matching
#define ASTHRA_ENUM_TAG_WILDCARD UINT32_MAX

// =============================================================================
// DEBUG AND VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate enum variant structure
 * @param variant Pointer to enum variant
 * @return true if valid, false otherwise
 */
bool Asthra_enum_validate_variant(AsthraEnumVariant *variant);

/**
 * Generate debug string representation of enum variant
 * @param variant Pointer to enum variant
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of characters written (excluding null terminator)
 */
size_t Asthra_enum_debug_string(AsthraEnumVariant *variant, char *buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ENUM_SUPPORT_H
