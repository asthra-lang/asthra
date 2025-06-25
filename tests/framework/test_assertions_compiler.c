/**
 * Asthra Programming Language
 * Test Framework - Compiler-Specific Assertions Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of compiler-specific assertion functions
 */

#include "test_assertions_compiler.h"

// =============================================================================
// COMPILER-SPECIFIC ASSERTION IMPLEMENTATIONS
// =============================================================================

bool assert_token_equals(AsthraTestContext *context, const Token *token,
                         const TokenExpectation *expected) {
    if (!context || !token || !expected) {
        return false;
    }

    // Check token type
    if (token->type != expected->type) {
        asthra_test_assert_int_eq(context, token->type, expected->type,
                                  "Token type mismatch: expected %d, got %d", expected->type,
                                  token->type);
        return false;
    }

    // Check token value if provided - use the correct union field
    if (expected->value) {
        const char *actual_value = NULL;

        switch (token->type) {
        case TOKEN_STRING:
            actual_value = token->data.string.value;
            break;
        case TOKEN_IDENTIFIER:
            actual_value = token->data.identifier.name;
            break;
        default:
            // For other token types, we can't easily compare string values
            break;
        }

        if (actual_value) {
            if (strcmp(actual_value, expected->value) != 0) {
                asthra_test_assert_string_eq(context, actual_value, expected->value,
                                             "Token value mismatch: expected '%s', got '%s'",
                                             expected->value, actual_value);
                return false;
            }
        }
    }

    // Check position information if provided
    if (expected->line > 0 && token->location.line != expected->line) {
        asthra_test_assert_int_eq(context, token->location.line, expected->line,
                                  "Token line mismatch: expected %zu, got %zu", expected->line,
                                  token->location.line);
        return false;
    }

    if (expected->column > 0 && token->location.column != expected->column) {
        asthra_test_assert_int_eq(context, token->location.column, expected->column,
                                  "Token column mismatch: expected %zu, got %zu", expected->column,
                                  token->location.column);
        return false;
    }

    return true;
}

bool assert_ast_structure(AsthraTestContext *context, const ASTNode *node,
                          const AstExpectation *expected) {
    if (!context || !node || !expected) {
        return false;
    }

    // Check node type
    if (node->type != expected->type) {
        asthra_test_assert_int_eq(context, node->type, expected->type,
                                  "AST node type mismatch: expected %d, got %d", expected->type,
                                  node->type);
        return false;
    }

    // Check child count if specified (this would need to be implemented based on actual AST
    // structure)
    if (expected->child_count > 0) {
        size_t actual_children = 0;
        // This would need to be implemented based on the actual AST node structure
        // For now, we'll skip this check
        if (actual_children != expected->child_count) {
            asthra_test_assert_size_eq(context, actual_children, expected->child_count,
                                       "AST child count mismatch: expected %zu, got %zu",
                                       expected->child_count, actual_children);
            return false;
        }
    }

    // Check node name if provided (this depends on the specific AST node type)
    if (expected->name) {
        const char *actual_name = NULL;

        switch (node->type) {
        case AST_IDENTIFIER:
            actual_name = node->data.identifier.name;
            break;
        case AST_BASE_TYPE:
            actual_name = node->data.base_type.name;
            break;
        default:
            // Other node types may not have names
            break;
        }

        if (actual_name) {
            if (strcmp(actual_name, expected->name) != 0) {
                asthra_test_assert_string_eq(context, actual_name, expected->name,
                                             "AST node name mismatch: expected '%s', got '%s'",
                                             expected->name, actual_name);
                return false;
            }
        }
    }

    return true;
}

bool tokens_equal(const Token *token1, const Token *token2) {
    if (!token1 || !token2) {
        return token1 == token2;
    }

    if (token1->type != token2->type) {
        return false;
    }

    // Compare values based on token type
    switch (token1->type) {
    case TOKEN_STRING:
        if (token1->data.string.value && token2->data.string.value) {
            return strcmp(token1->data.string.value, token2->data.string.value) == 0;
        }
        return token1->data.string.value == token2->data.string.value;

    case TOKEN_IDENTIFIER:
        if (token1->data.identifier.name && token2->data.identifier.name) {
            return strcmp(token1->data.identifier.name, token2->data.identifier.name) == 0;
        }
        return token1->data.identifier.name == token2->data.identifier.name;

    case TOKEN_INTEGER:
        return token1->data.integer.value == token2->data.integer.value;

    case TOKEN_FLOAT:
        return token1->data.float_val.value == token2->data.float_val.value;

    case TOKEN_CHAR:
        return token1->data.character.value == token2->data.character.value;

    default:
        // For other token types, just compare types (already done above)
        return true;
    }
}

bool ast_has_structure(const ASTNode *node, ASTNodeType expected_type, size_t expected_children) {
    if (!node) {
        return false;
    }

    if (node->type != expected_type) {
        return false;
    }

    // Count children (this would depend on actual AST structure)
    size_t actual_children = 0;
    // Implementation would traverse and count children based on node type

    return actual_children == expected_children;
}
