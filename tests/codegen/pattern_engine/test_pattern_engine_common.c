/**
 * Asthra Programming Language Compiler
 * Pattern Engine Tests - Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Shared utilities and implementations for pattern engine tests
 */

#include "test_pattern_engine_common.h"

// Context management
void init_pattern_engine_context(PatternEngineContext* ctx) {
    ctx->variant_count = 0;
    ctx->pattern_count = 0;
    ctx->match_count = 0;
    ctx->compilation_successful = true;
    ctx->compilation_error = NULL;
    ctx->current_match_value = NULL;
    ctx->match_context = NULL;
    ctx->match_found = false;
    ctx->executed_case = -1;
    ctx->next_pattern_id = 9000;
    ctx->next_case_id = 5000;
    ctx->next_match_id = 1000;
    ctx->total_patterns_created = 0;
    ctx->total_matches_executed = 0;
    
    for (int i = 0; i < 64; i++) {
        ctx->variants[i] = (VariantValue){0};
    }
    
    for (int i = 0; i < 128; i++) {
        ctx->patterns[i] = (Pattern){0};
    }
    
    for (int i = 0; i < 32; i++) {
        ctx->matches[i] = (MatchExpression){0};
    }
}

// Variant creation functions
VariantValue* create_variant_int(PatternEngineContext* ctx, int value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_INT;
    variant->data.int_value = value;
    variant->size = sizeof(int);
    variant->match_id = ctx->next_match_id++;
    
    return variant;
}

VariantValue* create_variant_string(PatternEngineContext* ctx, const char* value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_STRING;
    variant->data.string_value = (char*)value;
    variant->size = strlen(value);
    variant->match_id = ctx->next_match_id++;
    
    return variant;
}

VariantValue* create_variant_option_some(PatternEngineContext* ctx, VariantValue* inner) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_OPTION_SOME;
    variant->data.option_value = inner;
    variant->size = sizeof(VariantValue*);
    variant->match_id = ctx->next_match_id++;
    
    return variant;
}

VariantValue* create_variant_option_none(PatternEngineContext* ctx) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_OPTION_NONE;
    variant->data.option_value = NULL;
    variant->size = 0;
    variant->match_id = ctx->next_match_id++;
    
    return variant;
}

VariantValue* create_variant_result_ok(PatternEngineContext* ctx, VariantValue* ok_value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_RESULT_OK;
    variant->data.result.ok_value = ok_value;
    variant->data.result.error_value = NULL;
    variant->size = sizeof(VariantValue*);
    variant->match_id = ctx->next_match_id++;
    
    return variant;
}

VariantValue* create_variant_result_error(PatternEngineContext* ctx, VariantValue* error_value) {
    if (ctx->variant_count >= 64) {
        return NULL;
    }
    
    VariantValue* variant = &ctx->variants[ctx->variant_count++];
    variant->tag = VARIANT_TAG_RESULT_ERROR;
    variant->data.result.ok_value = NULL;
    variant->data.result.error_value = error_value;
    variant->size = sizeof(VariantValue*);
    variant->match_id = ctx->next_match_id++;
    
    return variant;
}

// Pattern creation functions
Pattern* create_pattern_wildcard(PatternEngineContext* ctx) {
    if (ctx->pattern_count >= 128) {
        return NULL;
    }
    
    Pattern* pattern = &ctx->patterns[ctx->pattern_count++];
    pattern->type = PATTERN_WILDCARD;
    pattern->name = "_";
    pattern->sub_pattern_count = 0;
    pattern->pattern_id = ctx->next_pattern_id++;
    ctx->total_patterns_created++;
    
    return pattern;
}

Pattern* create_pattern_literal_int(PatternEngineContext* ctx, int value) {
    if (ctx->pattern_count >= 128) {
        return NULL;
    }
    
    Pattern* pattern = &ctx->patterns[ctx->pattern_count++];
    pattern->type = PATTERN_LITERAL;
    pattern->literal = create_variant_int(ctx, value);
    pattern->sub_pattern_count = 0;
    pattern->pattern_id = ctx->next_pattern_id++;
    ctx->total_patterns_created++;
    
    return pattern;
}

Pattern* create_pattern_constructor(PatternEngineContext* ctx, VariantTag tag, const char* name, Pattern* sub_pattern) {
    if (ctx->pattern_count >= 128) {
        return NULL;
    }
    
    Pattern* pattern = &ctx->patterns[ctx->pattern_count++];
    pattern->type = PATTERN_CONSTRUCTOR;
    pattern->name = name;
    pattern->expected_tag = tag;
    pattern->sub_pattern_count = 0;
    pattern->pattern_id = ctx->next_pattern_id++;
    ctx->total_patterns_created++;
    
    if (sub_pattern) {
        pattern->sub_patterns[0] = sub_pattern;
        pattern->sub_pattern_count = 1;
    }
    
    return pattern;
}

Pattern* create_pattern_variable(PatternEngineContext* ctx, const char* name) {
    if (ctx->pattern_count >= 128) {
        return NULL;
    }
    
    Pattern* pattern = &ctx->patterns[ctx->pattern_count++];
    pattern->type = PATTERN_VARIABLE;
    pattern->name = name;
    pattern->sub_pattern_count = 0;
    pattern->pattern_id = ctx->next_pattern_id++;
    ctx->total_patterns_created++;
    
    return pattern;
}

Pattern* create_pattern_with_guard(PatternEngineContext* ctx, Pattern* base_pattern, bool (*guard_fn)(const VariantValue*)) {
    if (ctx->pattern_count >= 128) {
        return NULL;
    }
    
    Pattern* pattern = &ctx->patterns[ctx->pattern_count++];
    pattern->type = PATTERN_GUARD;
    pattern->guard_function = guard_fn;
    pattern->sub_pattern_count = 0;
    pattern->pattern_id = ctx->next_pattern_id++;
    ctx->total_patterns_created++;
    
    if (base_pattern) {
        pattern->sub_patterns[0] = base_pattern;
        pattern->sub_pattern_count = 1;
    }
    
    return pattern;
}

// Pattern matching logic
bool match_pattern(const Pattern* pattern, const VariantValue* value) {
    if (!pattern || !value) {
        return false;
    }
    
    switch (pattern->type) {
        case PATTERN_WILDCARD:
            return true;
            
        case PATTERN_LITERAL:
            if (!pattern->literal || pattern->literal->tag != value->tag) {
                return false;
            }
            
            switch (value->tag) {
                case VARIANT_TAG_INT:
                    return pattern->literal->data.int_value == value->data.int_value;
                case VARIANT_TAG_FLOAT:
                    return pattern->literal->data.float_value == value->data.float_value;
                case VARIANT_TAG_STRING:
                    return strcmp(pattern->literal->data.string_value, value->data.string_value) == 0;
                default:
                    return false;
            }
            
        case PATTERN_VARIABLE:
            // Variables always match and bind the value
            return true;
            
        case PATTERN_CONSTRUCTOR:
            if (pattern->expected_tag != value->tag) {
                return false;
            }
            
            // Check sub-patterns for constructed types
            if (pattern->sub_pattern_count > 0) {
                switch (value->tag) {
                    case VARIANT_TAG_OPTION_SOME:
                        return match_pattern(pattern->sub_patterns[0], value->data.option_value);
                    case VARIANT_TAG_RESULT_OK:
                        return match_pattern(pattern->sub_patterns[0], value->data.result.ok_value);
                    case VARIANT_TAG_RESULT_ERROR:
                        return match_pattern(pattern->sub_patterns[0], value->data.result.error_value);
                    default:
                        return false;
                }
            }
            
            return true;
            
        case PATTERN_GUARD:
            // Check the base pattern first, then the guard
            if (pattern->sub_pattern_count > 0) {
                if (!match_pattern(pattern->sub_patterns[0], value)) {
                    return false;
                }
            }
            
            return pattern->guard_function ? pattern->guard_function(value) : true;
            
        default:
            return false;
    }
}

// Guard functions
bool positive_guard(const VariantValue* value) {
    return value && value->tag == VARIANT_TAG_INT && value->data.int_value > 0;
}

bool even_guard(const VariantValue* value) {
    return value && value->tag == VARIANT_TAG_INT && (value->data.int_value % 2) == 0;
}

bool large_number_guard(const VariantValue* value) {
    return value && value->tag == VARIANT_TAG_INT && value->data.int_value > 100;
}

// Match expression functions
MatchExpression* create_match_expression(PatternEngineContext* ctx) {
    if (ctx->match_count >= 32) {
        return NULL;
    }
    
    MatchExpression* match = &ctx->matches[ctx->match_count++];
    match->case_count = 0;
    match->is_exhaustive = false;
    match->covered_count = 0;
    match->match_expression_id = ctx->next_match_id++;
    match->total_executions = 0;
    match->successful_matches = 0;
    match->catch_all_hits = 0;
    
    return match;
}

void add_match_case(MatchExpression* match, Pattern* pattern, void (*action)(const VariantValue*, void*)) {
    if (!match || match->case_count >= 16) {
        return;
    }
    
    MatchCase* case_item = &match->cases[match->case_count++];
    case_item->pattern = pattern;
    case_item->action = action;
    case_item->is_catch_all = (pattern->type == PATTERN_WILDCARD);
    case_item->action_count = 0;
    case_item->case_id = 6000 + match->case_count;
    
    // Track covered tags for exhaustiveness checking
    if (pattern->type == PATTERN_CONSTRUCTOR || pattern->type == PATTERN_LITERAL) {
        VariantTag tag = (pattern->type == PATTERN_CONSTRUCTOR) ? 
            pattern->expected_tag : pattern->literal->tag;
        
        bool already_covered = false;
        for (int i = 0; i < match->covered_count; i++) {
            if (match->covered_tags[i] == tag) {
                already_covered = true;
                break;
            }
        }
        
        if (!already_covered && match->covered_count < VARIANT_TAG_COUNT) {
            match->covered_tags[match->covered_count++] = tag;
        }
    }
    
    // Check if this makes the match exhaustive
    if (case_item->is_catch_all) {
        match->is_exhaustive = true;
    }
}

bool execute_match(PatternEngineContext* ctx, MatchExpression* match, const VariantValue* value) {
    ctx->current_match_value = (VariantValue*)value;
    ctx->match_found = false;
    ctx->executed_case = -1;
    
    match->total_executions++;
    ctx->total_matches_executed++;
    
    for (int i = 0; i < match->case_count; i++) {
        MatchCase* case_item = &match->cases[i];
        
        if (match_pattern(case_item->pattern, value)) {
            case_item->action(value, ctx->match_context);
            case_item->action_count++;
            ctx->match_found = true;
            ctx->executed_case = i;
            match->successful_matches++;
            
            if (case_item->is_catch_all) {
                match->catch_all_hits++;
            }
            
            return true;
        }
    }
    
    return false; // No match found
}

// Action functions
void dummy_action(const VariantValue* value, void* context) {
    int* counter = (int*)context;
    if (counter) {
        (*counter)++;
    }
}

void specific_action(const VariantValue* value, void* context) {
    int* counter = (int*)context;
    if (counter) {
        (*counter) += 10;
    }
} 
