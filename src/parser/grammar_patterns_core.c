/**
 * Asthra Programming Language Compiler - Core Pattern Grammar Implementation
 * Main pattern parsing logic
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_patterns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Debug logging disabled - pattern parsing issue resolved
// #define DEBUG_PARSER 1

// Forward declarations for functions in other pattern modules
extern ASTNode *parse_enum_pattern_impl(Parser *parser, char *name, SourceLocation start_loc);

// =============================================================================
// CORE PATTERN PARSING
// =============================================================================

ASTNode *parse_pattern(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Debug trace disabled
    // fprintf(stderr, "TRACE: parse_pattern called with token type=%d\n",
    // parser->current_token.type);

#ifdef DEBUG_PARSER
    fprintf(stderr, "parse_pattern: current token type=%d\n", parser->current_token.type);
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        fprintf(stderr, "parse_pattern: identifier='%s'\n",
                parser->current_token.data.identifier.name);
    }
    // Validate parser state integrity
    if (parser->current_token.type == TOKEN_ERROR) {
        fprintf(stderr, "parse_pattern: WARNING - parser in error state\n");
    }
    if (at_end(parser)) {
        fprintf(stderr, "parse_pattern: WARNING - parser at end of input\n");
    }
#endif

    // Additional parser state validation to catch corruption early
    if (parser->current_token.type == TOKEN_ERROR) {
        report_error(parser, "Invalid token state detected in pattern parsing");
        return NULL;
    }

    // Handle 'mut' modifier for mutable bindings in patterns
    bool is_mutable = false;
    if (match_token(parser, TOKEN_MUT)) {
        is_mutable = true;
        advance_token(parser);
        // Update start location after consuming 'mut'
        start_loc = parser->current_token.location;
    }

    // Handle enum patterns like Result.Ok(value) and Option.Some(value)
    // Also handle Result and Option keywords for enum patterns
    if (match_token(parser, TOKEN_IDENTIFIER) || match_token(parser, TOKEN_RESULT) ||
        match_token(parser, TOKEN_OPTION)) {
        char *name;
        if (parser->current_token.type == TOKEN_RESULT) {
            name = strdup("Result");
        } else if (parser->current_token.type == TOKEN_OPTION) {
            name = strdup("Option");
        } else {
            name = strdup(parser->current_token.data.identifier.name);
        }

        advance_token(parser);

#ifdef DEBUG_PARSER
        fprintf(stderr, "parse_pattern: after advance, current token type=%d\n",
                parser->current_token.type);
        if (parser->current_token.type == TOKEN_DOT) {
            fprintf(stderr, "parse_pattern: found DOT token, routing to enum pattern\n");
        }
#endif

        // Strengthen enum pattern detection with multiple validation approaches
        bool is_enum_pattern = false;

        // Primary check: direct token type comparison (most reliable)
        if (parser->current_token.type == TOKEN_DOT) {
            is_enum_pattern = true;
        }
        // Secondary check: use match_token function (backup validation)
        else if (match_token(parser, TOKEN_DOT)) {
            is_enum_pattern = true;
        }
        // Tertiary check: use check_token function (additional validation)
        else if (check_token(parser, TOKEN_DOT)) {
            is_enum_pattern = true;
        }

        // Additional enum pattern detection: look ahead for enum-like patterns
        // This handles cases where current token corruption might occur
        if (!is_enum_pattern) {
            Token next_token = peek_token(parser);
            if (next_token.type == TOKEN_DOT) {
                // This suggests we might have missed a token due to state corruption
                // Let's try to recover by checking if this looks like an enum pattern
#ifdef DEBUG_PARSER
                fprintf(stderr, "parse_pattern: detected potential enum pattern via peek-ahead\n");
#endif
                is_enum_pattern = true;
            }
        }

        // Check for invalid double colon usage in patterns
        if (match_token(parser, TOKEN_DOUBLE_COLON)) {
            report_error(parser, "Invalid '::' usage in pattern. Use '.' instead of '::' for enum "
                                 "variants (e.g., Result.Ok instead of Result::Ok)");
            free(name);
            return NULL;
        }

        if (is_enum_pattern) {
            // This is an enum pattern: EnumName.Variant(binding)
            // Note: Enhanced detection logic to handle token synchronization issues
            // that can occur after enum expression parsing
#ifdef DEBUG_PARSER
            fprintf(stderr, "parse_pattern: routing to enum pattern for '%s'\n", name);
#endif
            advance_token(parser); // consume the dot
            return parse_enum_pattern_impl(parser, name, start_loc);
        } else if (match_token(parser, TOKEN_LESS_THAN)) {
            // Struct patterns with type args are no longer supported
            report_error(parser, "Struct patterns like 'Point<T> { ... }' are no longer supported "
                                 "in match statements. Use a simple identifier binding (e.g., 'p') "
                                 "and access fields in the match arm body (e.g., 'p.x')");
            free(name);
            return NULL;
        } else if (match_token(parser, TOKEN_LEFT_BRACE)) {
            // Struct patterns are no longer supported
            report_error(parser, "Struct patterns like 'Point { x, y }' are no longer supported in "
                                 "match statements. Use a simple identifier binding (e.g., 'p') "
                                 "and access fields in the match arm body (e.g., 'p.x')");
            free(name);
            return NULL;
        } else if (match_token(parser, TOKEN_LEFT_PAREN)) {
            // This looks like a function call pattern, which is not supported
            // Users must use qualified syntax like Option.Some(x)
            report_error(parser, "Unqualified enum variant patterns are not allowed. Use qualified "
                                 "syntax like Option.Some(x) or Option.None");
            free(name);
            return NULL;
        } else {
#ifdef DEBUG_PARSER
            fprintf(stderr,
                    "parse_pattern: no specific pattern detected, checking identifier/wildcard for "
                    "'%s'\n",
                    name);
            fprintf(stderr, "parse_pattern: current token after routing checks: type=%d\n",
                    parser->current_token.type);
            if (parser->current_token.type == TOKEN_LEFT_BRACE) {
                fprintf(stderr, "parse_pattern: ERROR - Found LEFT_BRACE but struct pattern "
                                "condition failed!\n");
                fprintf(stderr,
                        "parse_pattern: match_token(TOKEN_LESS_THAN)=%d, "
                        "match_token(TOKEN_LEFT_BRACE)=%d\n",
                        match_token(parser, TOKEN_LESS_THAN),
                        match_token(parser, TOKEN_LEFT_BRACE));
            }
#endif

            // Additional heuristic-based enum pattern detection
            // Check if this looks like a known enum type that might have been missed
            if ((strcmp(name, "Result") == 0 || strcmp(name, "Option") == 0) &&
                !match_token(parser, TOKEN_LEFT_BRACE) && !match_token(parser, TOKEN_LESS_THAN)) {
                // This looks like an enum type without proper dot detection
                // Try to see if the next token after this point is an identifier (variant name)
                Token next_token = peek_token(parser);
                if (next_token.type == TOKEN_IDENTIFIER) {
#ifdef DEBUG_PARSER
                    fprintf(stderr,
                            "parse_pattern: heuristic enum pattern detection for '%s' - attempting "
                            "recovery\n",
                            name);
#endif
                    // This might be an enum pattern where the dot got lost
                    // Create an error but try to continue with enum parsing if possible
                    report_error(parser, "Possible malformed enum pattern - expected '.' between "
                                         "enum name and variant");
                    // Don't return NULL, continue to see if we can recover
                }
            }

            // Check for wildcard pattern
            if (strcmp(name, "_") == 0) {
                free(name);
                ASTNode *node = ast_create_node(AST_WILDCARD_PATTERN, start_loc);
                if (!node)
                    return NULL;
                return node;
            }

            // Simple identifier pattern
            // fprintf(stderr, "parse_pattern: creating identifier pattern for '%s'\n", name);
            ASTNode *node = ast_create_node(AST_IDENTIFIER, start_loc);
            if (!node) {
                free(name);
                return NULL;
            }

            node->data.identifier.name = name;
            // Note: For patterns, mutability is stored differently than regular identifiers
            // This might need to be handled at a higher level in the AST
            // For now, we'll just parse it correctly
            return node;
        }
    }

    // Note: Struct patterns have been removed from the language
    // Anonymous patterns like { x, y } are not supported

    // Handle tuple patterns: (pattern1, pattern2, ...)
    if (match_token(parser, TOKEN_LEFT_PAREN)) {
        advance_token(parser);

        // Check for unit pattern ()
        if (match_token(parser, TOKEN_RIGHT_PAREN)) {
            advance_token(parser);
            // Unit pattern - treat as wildcard or special case
            ASTNode *node = ast_create_node(AST_WILDCARD_PATTERN, start_loc);
            if (!node)
                return NULL;
            return node;
        }

        // Parse tuple pattern
        ASTNodeList *patterns = ast_node_list_create(2);
        if (!patterns)
            return NULL;

        // Parse first pattern
        ASTNode *first_pattern = parse_pattern(parser);
        if (!first_pattern) {
            ast_node_list_destroy(patterns);
            return NULL;
        }

        // Check for comma - if no comma, it's just a parenthesized pattern
        if (!match_token(parser, TOKEN_COMMA)) {
            // Just a parenthesized pattern
            if (expect_token(parser, TOKEN_RIGHT_PAREN)) {
                ast_node_list_destroy(patterns);
                return first_pattern;
            } else {
                ast_free_node(first_pattern);
                ast_node_list_destroy(patterns);
                return NULL;
            }
        }

        // We have a comma, so this is a tuple pattern
        ast_node_list_add(&patterns, first_pattern);
        advance_token(parser); // consume comma

        // Parse remaining patterns
        do {
            ASTNode *pattern = parse_pattern(parser);
            if (!pattern) {
                // Clean up
                for (size_t i = 0; i < patterns->count; i++) {
                    ast_free_node(patterns->nodes[i]);
                }
                ast_node_list_destroy(patterns);
                return NULL;
            }

            ast_node_list_add(&patterns, pattern);

            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser) && !match_token(parser, TOKEN_RIGHT_PAREN));

        // Expect closing paren
        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
            // Clean up
            for (size_t i = 0; i < patterns->count; i++) {
                ast_free_node(patterns->nodes[i]);
            }
            ast_node_list_destroy(patterns);
            return NULL;
        }

        // Create tuple pattern node
        ASTNode *node = ast_create_node(AST_TUPLE_PATTERN, start_loc);
        if (!node) {
            // Clean up
            for (size_t i = 0; i < patterns->count; i++) {
                ast_free_node(patterns->nodes[i]);
            }
            ast_node_list_destroy(patterns);
            return NULL;
        }

        node->data.tuple_pattern.patterns = patterns;
        return node;
    }

    // Note: Wildcard patterns are now handled in the identifier pattern section above

    // Handle literals and other patterns
    if (match_token(parser, TOKEN_INTEGER)) {
        int64_t value = parser->current_token.data.integer.value;
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, start_loc);
        if (!node)
            return NULL;

        node->data.integer_literal.value = value;
        node->flags.is_constant_expr = true; // Integer literals are compile-time constants
        return node;
    }

    if (match_token(parser, TOKEN_STRING)) {
        char *value = strdup(parser->current_token.data.string.value);
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_STRING_LITERAL, start_loc);
        if (!node) {
            free(value);
            return NULL;
        }

        node->data.string_literal.value = value;
        node->flags.is_constant_expr = true; // String literals are compile-time constants
        return node;
    }

    if (match_token(parser, TOKEN_BOOL_TRUE)) {
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
        if (!node)
            return NULL;

        node->data.bool_literal.value = true;
        node->flags.is_constant_expr = true; // Boolean literals are compile-time constants
        return node;
    }

    if (match_token(parser, TOKEN_BOOL_FALSE)) {
        advance_token(parser);

        ASTNode *node = ast_create_node(AST_BOOL_LITERAL, start_loc);
        if (!node)
            return NULL;

        node->data.bool_literal.value = false;
        node->flags.is_constant_expr = true; // Boolean literals are compile-time constants
        return node;
    }

    report_error(parser, "Expected pattern");
    return NULL;
}