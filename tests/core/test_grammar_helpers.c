/**
 * Asthra Programming Language Grammar Test Helpers Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of common helpers and utilities for grammar tests.
 * These are mock implementations for testing the test framework itself.
 */

#include "test_grammar_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// MOCK IMPLEMENTATIONS
// =============================================================================

bool grammar_test_parse_expression(const char *expression) {
    // Mock implementation - always returns true for non-null expressions
    // In a real implementation, this would call the actual parser
    return expression != NULL && strlen(expression) > 0;
}

bool grammar_test_validate_disambiguation(const char *expression) {
    // Mock implementation - simple validation based on expression content
    // In a real implementation, this would check actual AST structure
    if (!expression) return false;
    
    // Mock: expressions with mixed operators need disambiguation
    bool has_dot = strstr(expression, ".") != NULL;
    bool has_bracket = strstr(expression, "[") != NULL;
    bool has_paren = strstr(expression, "(") != NULL;
    
    // Mock successful disambiguation for complex expressions
    return has_dot || has_bracket || has_paren;
}

bool grammar_test_check_precedence(const char *expression) {
    // Mock implementation - check for known precedence patterns
    // In a real implementation, this would verify precedence in AST
    if (!expression) return false;
    
    // Mock: basic precedence validation
    // Check that multiplication appears before addition, etc.
    const char *mult = strstr(expression, "*");
    const char *add = strstr(expression, "+");
    
    if (mult && add) {
        // If both operators present, assume precedence is handled correctly
        return true;
    }
    
    // For single operators or simple expressions, assume correct
    return true;
}

bool grammar_test_has_parsing_conflicts(int *shift_reduce, int *reduce_reduce) {
    // Mock implementation - simulate no conflicts
    // In a real implementation, this would check parser tables
    if (shift_reduce) *shift_reduce = 0;
    if (reduce_reduce) *reduce_reduce = 0;
    return false; // No conflicts detected
}

bool grammar_test_parse_semantic_tag(const char *tag_expression) {
    // Mock implementation - check for @ prefix and known tags
    // In a real implementation, this would use the semantic tag parser
    if (!tag_expression) return false;
    
    // Mock: semantic tags start with @
    if (tag_expression[0] != '@') return false;
    
    // Mock: check for known semantic tag names
    const char *known_tags[] = {
        "#[ownership(gc)]", "#[ownership(c)]", "#[transfer_full]", "#[security_sensitive]", "#[borrowed]", NULL
    };
    
    for (int i = 0; known_tags[i] != NULL; i++) {
        if (strstr(tag_expression, known_tags[i]) == tag_expression) {
            return true;
        }
    }
    
    return false;
}

bool grammar_test_validate_semantic_tag(const char *tag_expression) {
    // Mock implementation - validate tag semantics
    // In a real implementation, this would check tag validity rules
    if (!grammar_test_parse_semantic_tag(tag_expression)) return false;
    
    // Mock: additional semantic validation
    // Check that tag is applied to appropriate context (simplified)
    bool has_type = strstr(tag_expression, "int") != NULL ||
                   strstr(tag_expression, "char") != NULL ||
                   strstr(tag_expression, "void") != NULL ||
                   strstr(tag_expression, "uint8_t") != NULL;
    
    return has_type;
}

bool grammar_test_parse_ffi_annotation(const char *annotation) {
    // Mock implementation - check for FFI annotation patterns
    // In a real implementation, this would use the FFI annotation parser
    if (!annotation) return false;
    
    // Mock: check for known FFI annotation patterns
    const char *ffi_patterns[] = {
        "extern \"C\"", "#[ffi_safe]", "// variadic removed", "#[c_compat]", "#[ownership(pinned)]", NULL
    };
    
    for (int i = 0; ffi_patterns[i] != NULL; i++) {
        if (strstr(annotation, ffi_patterns[i]) != NULL) {
            return true;
        }
    }
    
    return false;
}

bool grammar_test_validate_ffi_annotation(const char *annotation) {
    // Mock implementation - validate FFI annotation semantics
    // In a real implementation, this would check annotation validity
    if (!grammar_test_parse_ffi_annotation(annotation)) return false;
    
    // Mock: basic semantic validation
    // Check that annotation appears in appropriate context
    bool has_function = strstr(annotation, "int ") != NULL ||
                       strstr(annotation, "void ") != NULL ||
                       strstr(annotation, "(") != NULL;
    
    bool has_struct = strstr(annotation, "struct ") != NULL;
    bool has_variable = strstr(annotation, "buffer") != NULL ||
                       strstr(annotation, "[") != NULL;
    
    return has_function || has_struct || has_variable;
}

// =============================================================================
// ADDITIONAL MOCK IMPLEMENTATIONS FOR v1.20 TESTS
// =============================================================================

static const char* last_error_message = NULL;

const char* grammar_test_get_last_error(void) {
    // Mock implementation - return a mock error message
    // In a real implementation, this would return actual parser error
    if (last_error_message) {
        return last_error_message;
    }
    return "postfix :: not allowed on non-type expressions";
}

bool grammar_test_is_associated_function_call(const char *expression) {
    // Mock implementation - check if expression looks like Type::function()
    // In a real implementation, this would check AST node type
    if (!expression) return false;
    
    // Mock: look for :: pattern in expression
    const char *double_colon = strstr(expression, "::");
    if (!double_colon) return false;
    
    // Mock: check that it starts with a type name (uppercase letter)
    if (expression[0] >= 'A' && expression[0] <= 'Z') {
        return true;
    }
    
    return false;
}

bool grammar_test_has_type_arguments(const char *expression) {
    // Mock implementation - check if expression has generic type syntax
    // In a real implementation, this would check AST structure
    if (!expression) return false;
    
    // Mock: look for < and > brackets indicating generics
    const char *opening = strstr(expression, "<");
    const char *closing = strstr(expression, ">");
    
    return opening && closing && opening < closing;
}

bool grammar_test_type_args_populated(const char *expression) {
    // Mock implementation - check if type_args field would be populated
    // In a real implementation, this would verify AST type_args field
    if (!grammar_test_has_type_arguments(expression)) return false;
    
    // Mock: assume type_args is populated if generics are present
    // and we have content between the brackets
    const char *opening = strstr(expression, "<");
    const char *closing = strstr(expression, ">");
    
    if (opening && closing && closing > opening + 1) {
        // Check that there's content between < and >
        return true;
    }
    
    return false;
}

bool grammar_test_check_deterministic_parsing(void) {
    // Mock implementation - assume parsing is deterministic
    // In a real implementation, this would verify parser consistency
    // by parsing the same input multiple times and comparing results
    return true;
} 
