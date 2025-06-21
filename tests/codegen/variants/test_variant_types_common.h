/**
 * Asthra Programming Language Compiler
 * Variant Types Tests - Common Definitions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Shared definitions and utilities for variant type testing
 */

#ifndef TEST_VARIANT_TYPES_COMMON_H
#define TEST_VARIANT_TYPES_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Variant type definitions
typedef enum {
    VARIANT_TAG_NONE,
    VARIANT_TAG_INT,
    VARIANT_TAG_FLOAT,
    VARIANT_TAG_STRING,
    VARIANT_TAG_STRUCT,
    VARIANT_TAG_LIST,
    VARIANT_TAG_OPTION_SOME,
    VARIANT_TAG_OPTION_NONE,
    VARIANT_TAG_RESULT_OK,
    VARIANT_TAG_RESULT_ERROR,
    VARIANT_TAG_COUNT
} VariantTag;

typedef struct VariantValue {
    VariantTag tag;
    union {
        int int_value;
        double float_value;
        char* string_value;
        void* struct_value;
        struct VariantValue* list_value;
        struct VariantValue* option_value;
        struct {
            struct VariantValue* ok_value;
            struct VariantValue* error_value;
        } result;
    } data;
    size_t size;
    uint64_t variant_id;
} VariantValue;

// Main context structure for variant type management
typedef struct {
    VariantValue variants[64];
    int variant_count;
    
    // Type tracking
    int int_variants;
    int float_variants;
    int string_variants;
    int option_variants;
    int result_variants;
    
    // Memory management
    size_t total_memory_used;
    
    // Statistics
    uint64_t next_variant_id;
} VariantTypeContext;

// Context management functions
void init_variant_type_context(VariantTypeContext* ctx);

// Variant creation functions
VariantValue* create_variant_int(VariantTypeContext* ctx, int value);
VariantValue* create_variant_float(VariantTypeContext* ctx, double value);
VariantValue* create_variant_string(VariantTypeContext* ctx, const char* value);
VariantValue* create_variant_option_some(VariantTypeContext* ctx, VariantValue* inner);
VariantValue* create_variant_option_none(VariantTypeContext* ctx);
VariantValue* create_variant_result_ok(VariantTypeContext* ctx, VariantValue* ok_value);
VariantValue* create_variant_result_error(VariantTypeContext* ctx, VariantValue* error_value);

// Type checking functions
bool is_variant_option_type(const VariantValue* variant);
bool is_variant_result_type(const VariantValue* variant);
bool is_variant_primitive_type(const VariantValue* variant);

// Unwrapping functions
VariantValue* unwrap_option_variant(const VariantValue* variant);
VariantValue* unwrap_result_ok(const VariantValue* variant);
VariantValue* unwrap_result_error(const VariantValue* variant);

// Utility functions
int count_variants_by_tag(const VariantTypeContext* ctx, VariantTag tag);

#endif // TEST_VARIANT_TYPES_COMMON_H 
