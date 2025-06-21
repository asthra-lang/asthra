/**
 * Asthra Programming Language Runtime - TOML Support Module
 * FFI Support for stdlib::toml package
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides C runtime support for TOML parsing and generation
 * to support the stdlib::toml Asthra package.
 */

#ifndef ASTHRA_STDLIB_TOML_SUPPORT_H
#define ASTHRA_STDLIB_TOML_SUPPORT_H

#include "types/asthra_runtime_result.h"
#include "core/asthra_runtime_core.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TOML VALUE TYPES
// =============================================================================

/**
 * TOML value type enumeration
 */
typedef enum {
    ASTHRA_TOML_STRING = 0,
    ASTHRA_TOML_INTEGER = 1,
    ASTHRA_TOML_FLOAT = 2,
    ASTHRA_TOML_BOOLEAN = 3,
    ASTHRA_TOML_DATETIME = 4,
    ASTHRA_TOML_ARRAY = 5,
    ASTHRA_TOML_TABLE = 6,
    ASTHRA_TOML_INLINE_TABLE = 7
} AsthraTOMLValueType;

/**
 * TOML value structure
 */
typedef struct AsthraTOMLValue {
    AsthraTOMLValueType type;
    union {
        char* string_val;
        int64_t integer_val;
        double float_val;
        bool boolean_val;
        char* datetime_val;
        struct {
            struct AsthraTOMLValue* items;
            size_t count;
            size_t capacity;
        } array_val;
        struct AsthraTOMLTable* table_val;
    } data;
    AsthraOwnershipHint ownership;
} AsthraTOMLValue;

/**
 * TOML table structure (key-value pairs)
 */
typedef struct AsthraTOMLTable {
    char** keys;
    AsthraTOMLValue* values;
    size_t count;
    size_t capacity;
    AsthraOwnershipHint ownership;
} AsthraTOMLTable;

/**
 * TOML document structure
 */
typedef struct AsthraTOMLDocument {
    AsthraTOMLTable* root_table;
    char** parse_errors;
    size_t error_count;
    bool has_errors;
    AsthraOwnershipHint ownership;
} AsthraTOMLDocument;

/**
 * TOML configuration structure
 */
typedef struct AsthraTOMLConfig {
    bool preserve_order;
    bool allow_mixed_arrays;
    bool strict_mode;
    size_t max_depth;
} AsthraTOMLConfig;

// =============================================================================
// CORE TOML PARSING FUNCTIONS
// =============================================================================

/**
 * Parse TOML string into document
 * @param input TOML string to parse
 * @return Result containing parsed document or error
 */
AsthraResult asthra_toml_parse_string(const char* input);

/**
 * Parse TOML string with configuration
 * @param input TOML string to parse
 * @param config Parsing configuration
 * @return Result containing parsed document or error
 */
AsthraResult asthra_toml_parse_string_with_config(const char* input, const AsthraTOMLConfig* config);

/**
 * Parse TOML from file
 * @param filename Path to TOML file
 * @return Result containing parsed document or error
 */
AsthraResult asthra_toml_parse_file(const char* filename);

/**
 * Generate TOML string from document
 * @param document TOML document to serialize
 * @return Result containing TOML string or error
 */
AsthraResult asthra_toml_generate_string(const AsthraTOMLDocument* document);

/**
 * Generate TOML string with formatting options
 * @param document TOML document to serialize
 * @param config Generation configuration
 * @return Result containing TOML string or error
 */
AsthraResult asthra_toml_generate_string_with_config(const AsthraTOMLDocument* document, const AsthraTOMLConfig* config);

// =============================================================================
// TOML VALUE CREATION FUNCTIONS
// =============================================================================

/**
 * Create TOML string value
 * @param str String value
 * @return TOML value containing string
 */
AsthraTOMLValue* asthra_toml_string_value(const char* str);

/**
 * Create TOML integer value
 * @param val Integer value
 * @return TOML value containing integer
 */
AsthraTOMLValue* asthra_toml_integer_value(int64_t val);

/**
 * Create TOML float value
 * @param val Float value
 * @return TOML value containing float
 */
AsthraTOMLValue* asthra_toml_float_value(double val);

/**
 * Create TOML boolean value
 * @param val Boolean value
 * @return TOML value containing boolean
 */
AsthraTOMLValue* asthra_toml_boolean_value(bool val);

/**
 * Create TOML array value
 * @param values Array of TOML values
 * @param count Number of values
 * @return TOML value containing array
 */
AsthraTOMLValue* asthra_toml_array_value(AsthraTOMLValue** values, size_t count);

/**
 * Create TOML table value
 * @param table TOML table
 * @return TOML value containing table
 */
AsthraTOMLValue* asthra_toml_table_value(AsthraTOMLTable* table);

// =============================================================================
// TOML TABLE OPERATIONS
// =============================================================================

/**
 * Create new TOML table
 * @return New empty TOML table
 */
AsthraTOMLTable* asthra_toml_table_create(void);

/**
 * Get value from TOML table by key
 * @param table TOML table
 * @param key Key to look up
 * @return TOML value or NULL if not found
 */
AsthraTOMLValue* asthra_toml_table_get(const AsthraTOMLTable* table, const char* key);

/**
 * Set value in TOML table
 * @param table TOML table
 * @param key Key to set
 * @param value Value to set
 * @return Result indicating success or failure
 */
AsthraResult asthra_toml_table_set(AsthraTOMLTable* table, const char* key, AsthraTOMLValue* value);

/**
 * Check if TOML table contains key
 * @param table TOML table
 * @param key Key to check
 * @return true if key exists, false otherwise
 */
bool asthra_toml_table_contains_key(const AsthraTOMLTable* table, const char* key);

/**
 * Get number of entries in TOML table
 * @param table TOML table
 * @return Number of key-value pairs
 */
size_t asthra_toml_table_size(const AsthraTOMLTable* table);

// =============================================================================
// TOML DOCUMENT OPERATIONS
// =============================================================================

/**
 * Create new TOML document
 * @return New empty TOML document
 */
AsthraTOMLDocument* asthra_toml_document_create(void);

/**
 * Get root table from TOML document
 * @param document TOML document
 * @return Root table
 */
AsthraTOMLTable* asthra_toml_document_get_root(const AsthraTOMLDocument* document);

/**
 * Check if TOML document has parse errors
 * @param document TOML document
 * @return true if document has errors, false otherwise
 */
bool asthra_toml_document_has_errors(const AsthraTOMLDocument* document);

/**
 * Get parse errors from TOML document
 * @param document TOML document
 * @param error_count Output parameter for number of errors
 * @return Array of error messages
 */
const char** asthra_toml_document_get_errors(const AsthraTOMLDocument* document, size_t* error_count);

// =============================================================================
// TOML CONFIGURATION
// =============================================================================

/**
 * Create default TOML configuration
 * @return Default configuration structure
 */
AsthraTOMLConfig asthra_toml_default_config(void);

/**
 * Create lenient TOML configuration
 * @return Lenient configuration structure
 */
AsthraTOMLConfig asthra_toml_lenient_config(void);

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

/**
 * Free TOML value and its contents
 * @param value TOML value to free
 */
void asthra_toml_value_free(AsthraTOMLValue* value);

/**
 * Free TOML table and its contents
 * @param table TOML table to free
 */
void asthra_toml_table_free(AsthraTOMLTable* table);

/**
 * Free TOML document and its contents
 * @param document TOML document to free
 */
void asthra_toml_document_free(AsthraTOMLDocument* document);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get TOML value type as string
 * @param type TOML value type
 * @return String representation of type
 */
const char* asthra_toml_value_type_string(AsthraTOMLValueType type);

/**
 * Check if TOML value is primitive (not array or table)
 * @param value TOML value to check
 * @return true if primitive, false otherwise
 */
bool asthra_toml_value_is_primitive(const AsthraTOMLValue* value);

/**
 * Clone TOML value (deep copy)
 * @param value TOML value to clone
 * @return Cloned TOML value
 */
AsthraTOMLValue* asthra_toml_value_clone(const AsthraTOMLValue* value);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_STDLIB_TOML_SUPPORT_H 
