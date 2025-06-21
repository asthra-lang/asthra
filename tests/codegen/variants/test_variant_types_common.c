/**
 * Asthra Programming Language Compiler
 * Variant Types Tests - Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Shared implementation for variant type testing utilities
 */

#include "test_variant_types_common.h"

// Context management functions
void init_variant_type_context(VariantTypeContext* ctx) {
    ctx->variant_count = 0;
    ctx->int_variants = 0;
    ctx->float_variants = 0;
    ctx->string_variants = 0;
    ctx->option_variants = 0;
    ctx->result_variants = 0;
    ctx->total_memory_used = 0;
    ctx->next_variant_id = 8000;
    
    for (int i = 0; i < 64; i++) {
        ctx->variants[i] = (VariantValue){0};
    }
}

// Variant creation functions
VariantValue* create_variant_int(VariantTypeContext* ctx, int value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_INT;
    variant->data.int_value = value;
    variant->size = sizeof(int);
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->int_variants++;
    ctx->total_memory_used += variant->size;
    
    return variant;
}

VariantValue* create_variant_float(VariantTypeContext* ctx, double value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_FLOAT;
    variant->data.float_value = value;
    variant->size = sizeof(double);
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->float_variants++;
    ctx->total_memory_used += variant->size;
    
    return variant;
}

VariantValue* create_variant_string(VariantTypeContext* ctx, const char* value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_STRING;
    variant->data.string_value = (char*)value;
    variant->size = strlen(value);
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->string_variants++;
    ctx->total_memory_used += variant->size;
    
    return variant;
}

VariantValue* create_variant_option_some(VariantTypeContext* ctx, VariantValue* inner) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_OPTION_SOME;
    variant->data.option_value = inner;
    variant->size = sizeof(VariantValue*);
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->option_variants++;
    ctx->total_memory_used += variant->size;
    
    return variant;
}

VariantValue* create_variant_option_none(VariantTypeContext* ctx) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_OPTION_NONE;
    variant->data.option_value = NULL;
    variant->size = 0;
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->option_variants++;
    // Note: None doesn't use additional memory
    
    return variant;
}

VariantValue* create_variant_result_ok(VariantTypeContext* ctx, VariantValue* ok_value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_RESULT_OK;
    variant->data.result.ok_value = ok_value;
    variant->data.result.error_value = NULL;
    variant->size = sizeof(VariantValue*);
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->result_variants++;
    ctx->total_memory_used += variant->size;
    
    return variant;
}

VariantValue* create_variant_result_error(VariantTypeContext* ctx, VariantValue* error_value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_RESULT_ERROR;
    variant->data.result.ok_value = NULL;
    variant->data.result.error_value = error_value;
    variant->size = sizeof(VariantValue*);
    variant->variant_id = ctx->next_variant_id++;
    
    ctx->result_variants++;
    ctx->total_memory_used += variant->size;
    
    return variant;
}

// Type checking functions
bool is_variant_option_type(const VariantValue* variant) {
    return variant && (variant->tag == VARIANT_TAG_OPTION_SOME || 
                      variant->tag == VARIANT_TAG_OPTION_NONE);
}

bool is_variant_result_type(const VariantValue* variant) {
    return variant && (variant->tag == VARIANT_TAG_RESULT_OK || 
                      variant->tag == VARIANT_TAG_RESULT_ERROR);
}

bool is_variant_primitive_type(const VariantValue* variant) {
    return variant && (variant->tag == VARIANT_TAG_INT || 
                      variant->tag == VARIANT_TAG_FLOAT || 
                      variant->tag == VARIANT_TAG_STRING);
}

// Unwrapping functions
VariantValue* unwrap_option_variant(const VariantValue* variant) {
    if (!variant || variant->tag != VARIANT_TAG_OPTION_SOME) {
        return NULL;
    }
    return variant->data.option_value;
}

VariantValue* unwrap_result_ok(const VariantValue* variant) {
    if (!variant || variant->tag != VARIANT_TAG_RESULT_OK) {
        return NULL;
    }
    return variant->data.result.ok_value;
}

VariantValue* unwrap_result_error(const VariantValue* variant) {
    if (!variant || variant->tag != VARIANT_TAG_RESULT_ERROR) {
        return NULL;
    }
    return variant->data.result.error_value;
}

// Utility functions
int count_variants_by_tag(const VariantTypeContext* ctx, VariantTag tag) {
    int count = 0;
    for (int i = 0; i < ctx->variant_count; i++) {
        if (ctx->variants[i].tag == tag) {
            count++;
        }
    }
    return count;
} 
