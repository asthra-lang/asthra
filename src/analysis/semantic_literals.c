/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Literal Expressions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Analysis of literal expressions including identifiers, numbers, strings, chars, etc.
 */

#include "semantic_literals.h"
#include "semantic_core.h"
#include "semantic_utilities.h"
#include "semantic_types.h"
#include "semantic_type_helpers.h"
#include "semantic_diagnostics.h"
#include "semantic_builtins.h"
#include "type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// =============================================================================
// LITERAL ANALYSIS
// =============================================================================

bool analyze_identifier_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr || expr->type != AST_IDENTIFIER) {
        return false;
    }
    
    const char *identifier_name = expr->data.identifier.name;
    if (!identifier_name) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             expr->location, "Invalid identifier: missing name");
        return false;
    }
    
    // Check if identifier exists in symbol table
    SymbolEntry *symbol = semantic_resolve_identifier(analyzer, identifier_name);
    if (!symbol) {
        semantic_report_undefined_with_similar(analyzer, expr->location, identifier_name, analyzer->current_scope);
        return false;
    }
    
    // Mark symbol as used for unused variable detection
    symbol->flags.is_used = true;
    
    // Set type information on the identifier node
    if (symbol->type) {
        // Create type info from the symbol's type descriptor
        // TODO: When type-as-value support is implemented, SYMBOL_TYPE may need special handling
        expr->type_info = create_type_info_from_descriptor(symbol->type);
        
        if (!expr->type_info) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                 "Failed to create type info for identifier '%s'", identifier_name);
            return false;
        }
    }
    
    return true;
}

bool analyze_literal_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return false;
    }
    
    // Handle character literals with explicit type annotation enforcement
    if (expr->type == AST_CHAR_LITERAL) {
        return analyze_char_literal(analyzer, expr);
    }
    
    // Handle string literals including multi-line strings (Phase 4)
    if (expr->type == AST_STRING_LITERAL) {
        return analyze_string_literal(analyzer, expr);
    }
    
    // Handle other literal types
    switch (expr->type) {
        case AST_INTEGER_LITERAL: {
            TypeDescriptor *int_type = semantic_get_builtin_type(analyzer, "int");
            if (!int_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                     "Failed to get builtin type 'int'");
                return false;
            }
            expr->type_info = create_type_info_from_descriptor(int_type);
            return expr->type_info != NULL;
        }
        
        case AST_FLOAT_LITERAL: {
            TypeDescriptor *float_type = semantic_get_builtin_type(analyzer, "float");
            if (!float_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                     "Failed to get builtin type 'float'");
                return false;
            }
            expr->type_info = create_type_info_from_descriptor(float_type);
            return expr->type_info != NULL;
        }
        
        case AST_BOOL_LITERAL: {
            TypeDescriptor *bool_type = semantic_get_builtin_type(analyzer, "bool");
            if (!bool_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                     "Failed to get builtin type 'bool'");
                return false;
            }
            expr->type_info = create_type_info_from_descriptor(bool_type);
            return expr->type_info != NULL;
        }
        
        case AST_UNIT_LITERAL: {
            TypeDescriptor *unit_type = semantic_get_builtin_type(analyzer, "void");
            if (!unit_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, expr->location,
                                     "Failed to get builtin type 'void'");
                return false;
            }
            expr->type_info = create_type_info_from_descriptor(unit_type);
            return expr->type_info != NULL;
        }
        
        default:
            // Already handled char and string literals above
            return true;
    }
}

// =============================================================================
// CHARACTER LITERAL ANALYSIS (Phase 4 Implementation)
// =============================================================================

bool analyze_char_literal(SemanticAnalyzer *analyzer, ASTNode *literal) {
    if (!analyzer || !literal || literal->type != AST_CHAR_LITERAL) {
        return false;
    }
    
    // Validate character value is in valid range
    uint32_t char_value = literal->data.char_literal.value;
    if (!is_valid_character_value(char_value)) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_LITERAL,
                             literal->location,
                             "Invalid character literal value: %u (exceeds valid character range)", 
                             char_value);
        return false;
    }
    
    // Character literals must be used in a context that provides explicit type information
    // This enforces the requirement for explicit type annotations
    if (!has_explicit_type_context(analyzer, literal)) {
        // In test mode, allow character literals without explicit type context
        if (!analyzer->config.test_mode) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_ANNOTATION_REQUIRED,
                                 literal->location,
                                 "Character literal requires explicit type annotation. "
                                 "Use: let var: char = '%c';", 
                                 char_value < 128 ? (char)char_value : '?');
            return false;
        }
    }
    
    // Set the type information for the character literal
    literal->type_info = get_builtin_char_type(analyzer);
    if (!literal->type_info) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                             literal->location,
                             "Failed to get character type information");
        return false;
    }
    
    return true;
}

// Helper function to validate character values
bool is_valid_character_value(uint32_t value) {
    // Accept ASCII range (0-127) and extended ASCII (0-255)
    // For Unicode support, we could extend this to 0x10FFFF
    return value <= 255;
}

// Helper function to check if character literal has explicit type context
bool has_explicit_type_context(SemanticAnalyzer *analyzer, ASTNode *literal) {
    (void)literal; // Avoid unused parameter warning for now
    
    // TODO: Implement proper context checking
    // For now, we'll check if we're in a variable declaration with explicit type
    // This is a simplified implementation - in a full implementation, we'd need to
    // traverse up the AST to find the parent context
    
    // Check if the current scope has type annotation requirements enabled
    if (analyzer->config.strict_mode) {
        // In strict mode, require explicit type annotations
        // For now, return false to enforce explicit typing
        // In practice, this would check the parent AST node context
        return false;
    }
    
    return true; // Allow implicit typing in non-strict mode
}

// Helper function to get the built-in character type
TypeInfo* get_builtin_char_type(SemanticAnalyzer *analyzer) {
    if (!analyzer) {
        return NULL;
    }
    
    // Look for the char type in builtin types and convert to TypeInfo
    for (size_t i = 0; i < analyzer->builtin_type_count; i++) {
        TypeDescriptor *type = analyzer->builtin_types[i];
        if (type && type->name && strcmp(type->name, "char") == 0) {
            // Convert TypeDescriptor to TypeInfo
            return type_info_from_descriptor(type);
        }
    }
    
    // If not found, try to get it from the global type registry
    extern TypeInfo *TYPE_INFO_CHAR; // Defined in type_info_builtins.c
    return TYPE_INFO_CHAR;
}

// =============================================================================
// STRING LITERAL ANALYSIS (Phase 4 Implementation)
// =============================================================================

bool analyze_string_literal(SemanticAnalyzer *analyzer, ASTNode *literal) {
    if (!analyzer || !literal || literal->type != AST_STRING_LITERAL) {
        return false;
    }
    
    const char *string_value = literal->data.string_literal.value;
    if (!string_value) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_LITERAL,
                             literal->location,
                             "String literal has null value");
        return false;
    }
    
    // Check if this is a multi-line string that needs indentation normalization
    if (is_multiline_string_literal(string_value)) {
        // Normalize indentation for multi-line strings
        char *normalized = normalize_multiline_indentation(string_value);
        if (normalized && normalized != string_value) {
            // Replace the string value with normalized version
            // Note: This modifies the AST, which should be done carefully
            // In a production implementation, we might want to store the normalized
            // value separately or mark the node for later processing
            free(literal->data.string_literal.value);
            literal->data.string_literal.value = normalized;
        }
    }
    
    // Set the type information for the string literal
    literal->type_info = get_builtin_string_type(analyzer);
    if (!literal->type_info) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL,
                             literal->location,
                             "Failed to get string type information");
        return false;
    }
    
    // Mark as constant expression
    literal->flags.is_constant_expr = true;
    literal->flags.has_side_effects = false;
    literal->flags.is_lvalue = false;
    
    return true;
}

// Helper functions for string literal analysis (Phase 4)
bool is_multiline_string_literal(const char *value) {
    if (!value) return false;
    
    // Check if the string contains newlines, which indicates it's multi-line
    return strchr(value, '\n') != NULL;
}

char* normalize_multiline_indentation(const char* content) {
    if (!content) return NULL;
    
    // Detect common indentation level
    size_t common_indent = detect_common_indentation(content);
    
    // Strip common indentation if any was found
    if (common_indent > 0) {
        return strip_common_indentation(content, common_indent);
    }
    
    // Return a copy of the original string if no normalization needed
    return strdup(content);
}

size_t detect_common_indentation(const char* content) {
    if (!content) return 0;
    
    size_t min_indent = SIZE_MAX;
    const char *line_start = content;
    bool found_non_empty_line = false;
    
    // Skip leading newlines
    while (*line_start == '\n') {
        line_start++;
    }
    
    while (*line_start) {
        const char *line_end = strchr(line_start, '\n');
        if (!line_end) {
            line_end = line_start + strlen(line_start);
        }
        
        // Skip empty lines
        if (line_end > line_start) {
            // Count leading whitespace
            size_t indent = 0;
            const char *p = line_start;
            while (p < line_end && (*p == ' ' || *p == '\t')) {
                indent++;
                p++;
            }
            
            // Only consider lines that have non-whitespace content
            if (p < line_end) {
                found_non_empty_line = true;
                if (indent < min_indent) {
                    min_indent = indent;
                }
            }
        }
        
        if (*line_end == '\n') {
            line_start = line_end + 1;
        } else {
            break;
        }
    }
    
    return found_non_empty_line ? min_indent : 0;
}

char* strip_common_indentation(const char* content, size_t indent_level) {
    if (!content || indent_level == 0) {
        return strdup(content);
    }
    
    size_t content_len = strlen(content);
    char *result = malloc(content_len + 1);
    if (!result) return NULL;
    
    const char *src = content;
    char *dst = result;
    
    // Skip leading newlines
    while (*src == '\n') {
        *dst++ = *src++;
    }
    
    while (*src) {
        const char *line_end = strchr(src, '\n');
        if (!line_end) {
            line_end = src + strlen(src);
        }
        
        // Skip empty lines as-is
        if (line_end == src) {
            if (*line_end == '\n') {
                *dst++ = *src++;
            }
            continue;
        }
        
        // Strip indentation from non-empty lines
        size_t to_strip = 0;
        const char *p = src;
        while (p < line_end && (*p == ' ' || *p == '\t') && to_strip < indent_level) {
            to_strip++;
            p++;
        }
        
        // Copy the rest of the line
        src += to_strip;
        while (src < line_end) {
            *dst++ = *src++;
        }
        // Copy the newline if it exists
        if (*src == '\n') {
            *dst++ = *src++;
        }
    }
    
    *dst = '\0';
    return result;
}

TypeInfo* get_builtin_string_type(SemanticAnalyzer *analyzer) {
    if (!analyzer) {
        return NULL;
    }
    
    // Look for the string type in builtin types and convert to TypeInfo
    for (size_t i = 0; i < analyzer->builtin_type_count; i++) {
        TypeDescriptor *type = analyzer->builtin_types[i];
        if (type && type->name && strcmp(type->name, "string") == 0) {
            // Convert TypeDescriptor to TypeInfo
            return type_info_from_descriptor(type);
        }
    }
    
    // If not found, try to get it from the global type registry
    extern TypeInfo *TYPE_INFO_STRING; // Defined in type_info_builtins.c
    return TYPE_INFO_STRING;
}