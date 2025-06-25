/**
 * Asthra Programming Language Compiler
 * Pattern Engine Tests - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared definitions and utilities for pattern engine tests
 */

#ifndef TEST_PATTERN_ENGINE_COMMON_H
#define TEST_PATTERN_ENGINE_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Variant type definitions (subset for pattern matching)
typedef enum {
    VARIANT_TAG_NONE,
    VARIANT_TAG_INT,
    VARIANT_TAG_FLOAT,
    VARIANT_TAG_STRING,
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
        char *string_value;
        struct VariantValue *option_value;
        struct {
            struct VariantValue *ok_value;
            struct VariantValue *error_value;
        } result;
    } data;
    size_t size;
    uint64_t match_id;
} VariantValue;

// Pattern matching structures
typedef enum {
    PATTERN_WILDCARD,    // _
    PATTERN_LITERAL,     // 42, "hello", true
    PATTERN_VARIABLE,    // x, name
    PATTERN_CONSTRUCTOR, // Some(x), Ok(value)
    PATTERN_TUPLE,       // (x, y, z)
    PATTERN_LIST,        // [head | tail]
    PATTERN_GUARD,       // pattern if condition
    PATTERN_COUNT
} PatternType;

typedef struct Pattern {
    PatternType type;
    const char *name;                // For variables and constructors
    VariantValue *literal;           // For literal patterns
    struct Pattern *sub_patterns[8]; // For nested patterns
    int sub_pattern_count;
    bool (*guard_function)(const VariantValue *); // For guards
    VariantTag expected_tag;                      // For constructor patterns
    uint64_t pattern_id;
} Pattern;

typedef struct {
    Pattern *pattern;
    void (*action)(const VariantValue *, void *context);
    bool is_catch_all;
    int action_count;
    uint64_t case_id;
} MatchCase;

typedef struct {
    MatchCase cases[16];
    int case_count;
    bool is_exhaustive;
    VariantTag covered_tags[VARIANT_TAG_COUNT];
    int covered_count;
    uint64_t match_expression_id;

    // Statistics
    int total_executions;
    int successful_matches;
    int catch_all_hits;
} MatchExpression;

// Main context structure
typedef struct {
    VariantValue variants[64];
    int variant_count;
    Pattern patterns[128];
    int pattern_count;
    MatchExpression matches[32];
    int match_count;

    // Pattern compilation state
    bool compilation_successful;
    const char *compilation_error;

    // Runtime state
    VariantValue *current_match_value;
    void *match_context;
    bool match_found;
    int executed_case;

    // Statistics
    uint64_t next_pattern_id;
    uint64_t next_case_id;
    uint64_t next_match_id;
    int total_patterns_created;
    int total_matches_executed;
} PatternEngineContext;

// Function declarations

// Context management
void init_pattern_engine_context(PatternEngineContext *ctx);

// Variant creation functions
VariantValue *create_variant_int(PatternEngineContext *ctx, int value);
VariantValue *create_variant_string(PatternEngineContext *ctx, const char *value);
VariantValue *create_variant_option_some(PatternEngineContext *ctx, VariantValue *inner);
VariantValue *create_variant_option_none(PatternEngineContext *ctx);
VariantValue *create_variant_result_ok(PatternEngineContext *ctx, VariantValue *ok_value);
VariantValue *create_variant_result_error(PatternEngineContext *ctx, VariantValue *error_value);

// Pattern creation functions
Pattern *create_pattern_wildcard(PatternEngineContext *ctx);
Pattern *create_pattern_literal_int(PatternEngineContext *ctx, int value);
Pattern *create_pattern_constructor(PatternEngineContext *ctx, VariantTag tag, const char *name,
                                    Pattern *sub_pattern);
Pattern *create_pattern_variable(PatternEngineContext *ctx, const char *name);
Pattern *create_pattern_with_guard(PatternEngineContext *ctx, Pattern *base_pattern,
                                   bool (*guard_fn)(const VariantValue *));

// Pattern matching logic
bool match_pattern(const Pattern *pattern, const VariantValue *value);

// Guard functions
bool positive_guard(const VariantValue *value);
bool even_guard(const VariantValue *value);
bool large_number_guard(const VariantValue *value);

// Match expression functions
MatchExpression *create_match_expression(PatternEngineContext *ctx);
void add_match_case(MatchExpression *match, Pattern *pattern,
                    void (*action)(const VariantValue *, void *));
bool execute_match(PatternEngineContext *ctx, MatchExpression *match, const VariantValue *value);

// Action functions
void dummy_action(const VariantValue *value, void *context);
void specific_action(const VariantValue *value, void *context);

#endif // TEST_PATTERN_ENGINE_COMMON_H
