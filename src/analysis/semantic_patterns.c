/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Unsafe Blocks and Pattern Validation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Analysis of unsafe blocks and pattern matching validation
 */

#include "semantic_patterns.h"
#include "../parser/ast_node_list.h"
#include "semantic_basic_statements.h"
#include "semantic_core.h"
#include "semantic_symbols.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// UNSAFE BLOCKS
// =============================================================================

bool analyze_unsafe_block_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt || stmt->type != AST_UNSAFE_BLOCK) {
        return false;
    }

    ASTNode *block = stmt->data.unsafe_block.block;
    if (!block || block->type != AST_BLOCK) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION, stmt->location,
                              "Unsafe block must contain a valid block");
        return false;
    }

    // Track that we're entering an unsafe context
    bool previous_unsafe_context = analyzer->in_unsafe_context;
    analyzer->in_unsafe_context = true;

    // Analyze the block contents within unsafe context
    bool result = analyze_block_statement(analyzer, block);

    // Restore previous unsafe context
    analyzer->in_unsafe_context = previous_unsafe_context;

    return result;
}

// =============================================================================
// PATTERN VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate that a pattern is compatible with the expected type and bind variables
 */
bool semantic_validate_pattern_types(SemanticAnalyzer *analyzer, ASTNode *pattern,
                                     TypeDescriptor *expected) {
    if (!analyzer || !pattern || !expected) {
        return false;
    }

    switch (pattern->type) {
    case AST_IDENTIFIER: {
        // Variable binding pattern - always matches, creates new variable
        const char *var_name = pattern->data.identifier.name;
        if (!var_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, pattern->location,
                                  "Pattern variable missing name");
            return false;
        }

        // Create symbol entry for the pattern variable
        SymbolEntry *var_symbol = symbol_entry_create(var_name, SYMBOL_VARIABLE, expected, pattern);
        if (!var_symbol) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, pattern->location,
                                  "Failed to create symbol entry for variable '%s'", var_name);
            return false;
        }

        // Add to current scope (should be the pattern scope)
        if (!symbol_table_insert_safe(analyzer->current_scope, var_name, var_symbol)) {
            symbol_entry_destroy(var_symbol);
            semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, pattern->location,
                                  "Variable '%s' already defined in this scope", var_name);
            return false;
        }

        return true;
    }

    case AST_ENUM_PATTERN: {
        // Enum pattern matching - validate enum type and variant
        const char *enum_name = pattern->data.enum_pattern.enum_name;
        const char *variant_name = pattern->data.enum_pattern.variant_name;

        if (!enum_name || !variant_name) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, pattern->location,
                                  "Enum pattern missing enum or variant name");
            return false;
        }

        // Check that expected type is an enum or Option
        if (expected->category != TYPE_ENUM && expected->category != TYPE_OPTION) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Cannot match enum pattern against non-enum type");
            return false;
        }

        // Check that enum names match
        if (strcmp(expected->name, enum_name) != 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Enum pattern '%s' does not match expected enum type '%s'",
                                  enum_name, expected->name);
            return false;
        }

        // Validate that variant exists
        if (expected->category == TYPE_OPTION) {
            // For Option types, only Some and None are valid variants
            if (strcmp(variant_name, "Some") != 0 && strcmp(variant_name, "None") != 0) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, pattern->location,
                                      "Option has no variant '%s' (only Some and None are valid)",
                                      variant_name);
                return false;
            }
        } else if (expected->data.enum_type.variants) {
            // For regular enums, look up in the variant table
            SymbolEntry *variant =
                symbol_table_lookup_safe(expected->data.enum_type.variants, variant_name);
            if (!variant) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL, pattern->location,
                                      "Enum '%s' has no variant '%s'", enum_name, variant_name);
                return false;
            }
        }

        // Handle nested pattern if present (e.g., Some(inner_pattern))
        if (pattern->data.enum_pattern.pattern) {
            // Determine the type for the nested pattern based on the enum variant
            TypeDescriptor *nested_type = NULL;

            // Special handling for Option<T>
            if (strcmp(enum_name, "Option") == 0 && strcmp(variant_name, "Some") == 0) {
                // For Option<T>, the nested type is T
                if (expected->category == TYPE_OPTION) {
                    nested_type = expected->data.option.value_type;
                } else {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, pattern->location,
                                          "Option pattern on non-Option type");
                    return false;
                }
            }
            // TODO: Add similar handling for Result<T,E> and other generic enums
            else {
                // Look up the variant's associated type from the enum type
                if (expected->data.enum_type.variants) {
                    SymbolEntry *variant_entry =
                        symbol_table_lookup_safe(expected->data.enum_type.variants, variant_name);
                    if (variant_entry && variant_entry->type) {
                        // Use the variant's associated type
                        nested_type = variant_entry->type;
                    }
                }
            }

            if (!nested_type) {
                semantic_report_error(
                    analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, pattern->location,
                    "Failed to determine type for nested pattern in variant '%s'", variant_name);
                return false;
            }

            // Recursively validate the nested pattern
            return semantic_validate_pattern_types(analyzer, pattern->data.enum_pattern.pattern,
                                                   nested_type);
        }
        // Handle variable binding if present (e.g., Some(x)) - backward compatibility
        else if (pattern->data.enum_pattern.binding) {
            const char *binding_name = pattern->data.enum_pattern.binding;

            // Determine the binding type based on the enum type
            TypeDescriptor *binding_type = NULL;

            // Special handling for Option<T>
            if (strcmp(enum_name, "Option") == 0 && strcmp(variant_name, "Some") == 0) {
                // For Option<T>, the binding type is T
                if (expected->category == TYPE_OPTION) {
                    binding_type = expected->data.option.value_type;
                    if (binding_type) {
                        type_descriptor_retain(binding_type);
                    }
                } else {
                    // This shouldn't happen if we validated the enum type correctly
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, pattern->location,
                                          "Option pattern on non-Option type");
                    return false;
                }
            }
            // TODO: Add similar handling for Result<T,E> and other generic enums
            else {
                // Look up the variant's associated type from the enum type
                if (expected->data.enum_type.variants) {
                    SymbolEntry *variant_entry =
                        symbol_table_lookup_safe(expected->data.enum_type.variants, variant_name);
                    if (variant_entry && variant_entry->type) {
                        // Use the variant's associated type
                        binding_type = variant_entry->type;
                        type_descriptor_retain(binding_type);
                    }
                }

                // Fallback if we couldn't find the variant's type
                if (!binding_type) {
                    // For compatibility with old code, default to int
                    binding_type = semantic_get_builtin_type(analyzer, "int");
                }
            }

            if (!binding_type) {
                semantic_report_error(
                    analyzer, SEMANTIC_ERROR_TYPE_INFERENCE_FAILED, pattern->location,
                    "Failed to determine type for pattern binding '%s'", binding_name);
                return false;
            }

            // Create symbol entry for the binding
            SymbolEntry *binding_symbol =
                symbol_entry_create(binding_name, SYMBOL_VARIABLE, binding_type, pattern);
            if (!binding_symbol) {
                // Release the binding type if we retained it
                if (binding_type &&
                    (strcmp(enum_name, "Option") == 0 ||
                     (expected->data.enum_type.variants &&
                      symbol_table_lookup_safe(expected->data.enum_type.variants, variant_name)))) {
                    type_descriptor_release(binding_type);
                }
                semantic_report_error(analyzer, SEMANTIC_ERROR_MEMORY_ALLOCATION, pattern->location,
                                      "Failed to create symbol entry for binding '%s'",
                                      binding_name);
                return false;
            }

            // Mark as immutable (pattern bindings are immutable by default)
            binding_symbol->flags.is_mutable = false;
            binding_symbol->flags.is_initialized = true;

            // Add to current scope
            if (!symbol_table_insert_safe(analyzer->current_scope, binding_name, binding_symbol)) {
                symbol_entry_destroy(binding_symbol);
                // Release the binding type if we retained it
                if (binding_type &&
                    (strcmp(enum_name, "Option") == 0 ||
                     (expected->data.enum_type.variants &&
                      symbol_table_lookup_safe(expected->data.enum_type.variants, variant_name)))) {
                    type_descriptor_release(binding_type);
                }
                semantic_report_error(analyzer, SEMANTIC_ERROR_DUPLICATE_SYMBOL, pattern->location,
                                      "Variable '%s' already defined in this scope", binding_name);
                return false;
            }

            // Release the retained type if we retained it (symbol now owns it)
            bool type_was_retained =
                (strcmp(enum_name, "Option") == 0 ||
                 (expected->data.enum_type.variants &&
                  symbol_table_lookup_safe(expected->data.enum_type.variants, variant_name)));
            if (type_was_retained && binding_type) {
                type_descriptor_release(binding_type);
            }
        }

        return true;
    }

    case AST_WILDCARD_PATTERN: {
        // Wildcard pattern - always matches, no variables bound
        return true;
    }

    case AST_TUPLE_PATTERN: {
        // Tuple pattern matching - validate tuple type and element patterns
        ASTNodeList *element_patterns = pattern->data.tuple_pattern.patterns;

        if (!element_patterns) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, pattern->location,
                                  "Tuple pattern missing element patterns");
            return false;
        }

        // Check that expected type is a tuple
        if (expected->category != TYPE_TUPLE) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Cannot match tuple pattern against non-tuple type");
            return false;
        }

        size_t pattern_count = ast_node_list_size(element_patterns);
        size_t expected_count = expected->data.tuple.element_count;

        // Check element count matches
        if (pattern_count != expected_count) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Tuple pattern element count mismatch: expected %zu, got %zu",
                                  expected_count, pattern_count);
            return false;
        }

        // Validate each element pattern against its corresponding type
        for (size_t i = 0; i < pattern_count; i++) {
            ASTNode *elem_pattern = ast_node_list_get(element_patterns, i);
            TypeDescriptor *elem_type = expected->data.tuple.element_types[i];

            if (!elem_pattern || !elem_type) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, pattern->location,
                                      "Invalid tuple pattern element");
                return false;
            }

            // Recursively validate element pattern
            if (!semantic_validate_pattern_types(analyzer, elem_pattern, elem_type)) {
                return false;
            }
        }

        return true;
    }

    case AST_INTEGER_LITERAL: {
        // Integer literal pattern - must match value and type
        if (!is_numeric_type(expected)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Cannot match integer literal pattern against non-numeric type");
            return false;
        }
        // TODO: Validate that the literal value is within the expected type's range
        return true;
    }

    case AST_STRING_LITERAL: {
        // String literal pattern - must match value and type
        if (!is_string_type(expected)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Cannot match string literal pattern against non-string type");
            return false;
        }
        return true;
    }

    case AST_BOOL_LITERAL: {
        // Boolean literal pattern - must match value and type
        if (!is_boolean_type(expected)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Cannot match boolean literal pattern against non-boolean type");
            return false;
        }
        return true;
    }

    case AST_FLOAT_LITERAL: {
        // Float literal pattern - must match value and type
        if (!is_numeric_type(expected)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, pattern->location,
                                  "Cannot match float literal pattern against non-numeric type");
            return false;
        }
        return true;
    }

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_EXPRESSION, pattern->location,
                              "Unsupported pattern type in if-let statement: %d", pattern->type);
        return false;
    }
}