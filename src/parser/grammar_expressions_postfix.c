/**
 * Asthra Programming Language Compiler - Postfix Expressions
 * Postfix operations (field access, array indexing, function calls)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include "parser_token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// POSTFIX EXPRESSIONS
// =============================================================================

ASTNode *parse_postfix_expr(Parser *parser) {
    if (!parser)
        return NULL;

    ASTNode *expr = parse_primary(parser);
    if (!expr)
        return NULL;

    while (true) {
        if (match_token(parser, TOKEN_DOT)) {
            SourceLocation op_loc = parser->current_token.location;
            advance_token(parser);

            char *field_name = NULL;

            // Check for tuple element access (.0, .1, etc.) or regular field access
            if (match_token(parser, TOKEN_INTEGER)) {
                // Tuple element access: convert integer to string
                int64_t index = parser->current_token.data.integer.value;
                if (index < 0) {
                    report_error(parser, "Tuple element index cannot be negative");
                    ast_free_node(expr);
                    return NULL;
                }

                // Convert integer to string for field_name
                field_name = malloc(32); // More than enough for any integer
                if (!field_name) {
                    ast_free_node(expr);
                    return NULL;
                }
                snprintf(field_name, 32, "%lld", (long long)index);
                advance_token(parser);
            } else if (match_token(parser, TOKEN_FLOAT)) {
                // Check if this is actually a tuple index that was misparsed as a float
                // For example, .1 might be parsed as FLOAT(0.1) in nested tuple access
                double float_val = parser->current_token.data.float_val.value;

                // Check if this is a float like 0.X, 0.XY, etc. that represents nested tuple access
                if (float_val >= 0.0 && float_val < 1.0) {
                    // This represents nested tuple access like .0.1 tokenized as FLOAT(0.1)
                    // We need to create the intermediate field access for .0

                    // First, create field access for .0
                    field_name = strdup("0");
                    if (!field_name) {
                        ast_free_node(expr);
                        return NULL;
                    }

                    ASTNode *first_access = ast_create_node(AST_FIELD_ACCESS, op_loc);
                    if (!first_access) {
                        free(field_name);
                        ast_free_node(expr);
                        return NULL;
                    }

                    first_access->data.field_access.object = expr;
                    first_access->data.field_access.field_name = field_name;

                    // Now handle the remaining decimal part
                    // For 0.1, we want to extract "1"
                    // For 0.12, we want to extract "12", etc.
                    char float_str[32];
                    snprintf(float_str, sizeof(float_str), "%.10f", float_val);

                    // Find the decimal point and extract digits after it
                    char *decimal_part = strchr(float_str, '.');
                    if (decimal_part && *(decimal_part + 1) != '\0') {
                        decimal_part++; // Skip the decimal point

                        // Remove trailing zeros
                        int len = strlen(decimal_part);
                        while (len > 0 && decimal_part[len - 1] == '0') {
                            decimal_part[len - 1] = '\0';
                            len--;
                        }

                        // The remaining digits are the tuple index
                        field_name = strdup(decimal_part);
                        if (!field_name) {
                            ast_free_node(first_access);
                            return NULL;
                        }
                    } else {
                        // This shouldn't happen for valid float representation
                        report_error(parser, "Invalid tuple index format");
                        ast_free_node(first_access);
                        return NULL;
                    }

                    advance_token(parser);

                    // Update expr to be the intermediate access
                    expr = first_access;
                } else {
                    report_error(parser, "Expected field name or tuple index after '.'");
                    ast_free_node(expr);
                    return NULL;
                }
            } else if (match_token(parser, TOKEN_IDENTIFIER)) {
                field_name = strdup(parser->current_token.data.identifier.name);
                advance_token(parser);

                // Check if this could be an enum constructor (uppercase identifier followed by
                // parenthesis)
                if (expr->type == AST_IDENTIFIER) {
                    const char *enum_name = expr->data.identifier.name;
                    // Check if enum name starts with uppercase (type convention)
                    if (enum_name && enum_name[0] >= 'A' && enum_name[0] <= 'Z' &&
                        match_token(parser, TOKEN_LEFT_PAREN)) {
                        // This looks like an enum constructor call
                        advance_token(parser); // consume '('

                        // Parse constructor arguments
                        ASTNode *value = NULL;
                        if (!match_token(parser, TOKEN_RIGHT_PAREN)) {
                            // Parse first argument
                            ASTNode *first_arg = parse_expr(parser);
                            if (!first_arg) {
                                free(field_name);
                                ast_free_node(expr);
                                return NULL;
                            }

                            // Check for additional arguments
                            if (match_token(parser, TOKEN_COMMA)) {
                                // Multiple arguments - create a tuple literal
                                ASTNodeList *arg_list = ast_node_list_create(4);
                                if (!arg_list) {
                                    ast_free_node(first_arg);
                                    free(field_name);
                                    ast_free_node(expr);
                                    return NULL;
                                }

                                // Add first argument to list
                                ast_node_list_add(&arg_list, first_arg);

                                // Parse remaining arguments
                                while (match_token(parser, TOKEN_COMMA)) {
                                    advance_token(parser);

                                    ASTNode *arg = parse_expr(parser);
                                    if (!arg) {
                                        ast_node_list_destroy(arg_list);
                                        free(field_name);
                                        ast_free_node(expr);
                                        return NULL;
                                    }

                                    ast_node_list_add(&arg_list, arg);
                                }

                                // Create tuple literal for multiple values
                                value = ast_create_node(AST_TUPLE_LITERAL, op_loc);
                                if (!value) {
                                    ast_node_list_destroy(arg_list);
                                    free(field_name);
                                    ast_free_node(expr);
                                    return NULL;
                                }
                                value->data.tuple_literal.elements = arg_list;
                            } else {
                                // Single argument
                                value = first_arg;
                            }
                        }

                        if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                            if (value)
                                ast_free_node(value);
                            free(field_name);
                            ast_free_node(expr);
                            return NULL;
                        }

                        // Create enum variant node
                        ASTNode *enum_variant = ast_create_node(AST_ENUM_VARIANT, op_loc);
                        if (!enum_variant) {
                            if (value)
                                ast_free_node(value);
                            free(field_name);
                            ast_free_node(expr);
                            return NULL;
                        }

                        enum_variant->data.enum_variant.enum_name = strdup(enum_name);
                        enum_variant->data.enum_variant.variant_name = field_name;
                        enum_variant->data.enum_variant.value = value;

                        ast_free_node(expr); // Free the identifier node
                        expr = enum_variant;
                        continue; // Continue with postfix processing
                    }
                }

                // Regular field access
            } else {
                report_error(parser, "Expected field name or tuple index after '.'");
                ast_free_node(expr);
                return NULL;
            }

            if (!field_name) {
                ast_free_node(expr);
                return NULL;
            }

            ASTNode *field_access = ast_create_node(AST_FIELD_ACCESS, op_loc);
            if (!field_access) {
                free(field_name);
                ast_free_node(expr);
                return NULL;
            }

            field_access->data.field_access.object = expr;
            field_access->data.field_access.field_name = field_name;

            expr = field_access;
        } else if (match_token(parser, TOKEN_LEFT_BRACKET)) {
            SourceLocation op_loc = parser->current_token.location;
            advance_token(parser);

            // Check for slice syntax (colon without expression for full slice [:])
            if (match_token(parser, TOKEN_COLON)) {
                advance_token(parser); // consume ':'

                // Parse end expression if present
                ASTNode *end = NULL;
                if (!match_token(parser, TOKEN_RIGHT_BRACKET)) {
                    end = parse_expr(parser);
                    if (!end) {
                        ast_free_node(expr);
                        return NULL;
                    }
                }

                if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
                    ast_free_node(expr);
                    if (end)
                        ast_free_node(end);
                    return NULL;
                }

                // Create slice expression with NULL start
                ASTNode *slice = ast_create_node(AST_SLICE_EXPR, op_loc);
                if (!slice) {
                    ast_free_node(expr);
                    if (end)
                        ast_free_node(end);
                    return NULL;
                }

                slice->data.slice_expr.array = expr;
                slice->data.slice_expr.start = NULL;
                slice->data.slice_expr.end = end;

                expr = slice;
            } else {
                // Parse first expression (could be index or slice start)
                ASTNode *first = parse_expr(parser);
                if (!first) {
                    ast_free_node(expr);
                    return NULL;
                }

                // Check if this is a slice operation
                if (match_token(parser, TOKEN_COLON)) {
                    advance_token(parser); // consume ':'

                    // Parse end expression if present
                    ASTNode *end = NULL;
                    if (!match_token(parser, TOKEN_RIGHT_BRACKET)) {
                        end = parse_expr(parser);
                        if (!end) {
                            ast_free_node(expr);
                            ast_free_node(first);
                            return NULL;
                        }
                    }

                    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
                        ast_free_node(expr);
                        ast_free_node(first);
                        if (end)
                            ast_free_node(end);
                        return NULL;
                    }

                    // Create slice expression
                    ASTNode *slice = ast_create_node(AST_SLICE_EXPR, op_loc);
                    if (!slice) {
                        ast_free_node(expr);
                        ast_free_node(first);
                        if (end)
                            ast_free_node(end);
                        return NULL;
                    }

                    slice->data.slice_expr.array = expr;
                    slice->data.slice_expr.start = first;
                    slice->data.slice_expr.end = end;

                    expr = slice;
                } else {
                    // Regular array indexing
                    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
                        ast_free_node(expr);
                        ast_free_node(first);
                        return NULL;
                    }

                    ASTNode *array_access = ast_create_node(AST_INDEX_ACCESS, op_loc);
                    if (!array_access) {
                        ast_free_node(expr);
                        ast_free_node(first);
                        return NULL;
                    }

                    array_access->data.index_access.array = expr;
                    array_access->data.index_access.index = first;

                    expr = array_access;
                }
            }
        } else if (match_token(parser, TOKEN_LEFT_PAREN)) {
            // Function call - Parse according to current grammar: ArgList <- Expr (',' Expr)* /
            // 'none'
            SourceLocation op_loc = parser->current_token.location;
            advance_token(parser);

            ASTNode **args = NULL;
            size_t arg_count = 0;
            size_t arg_capacity = 4;

            // Check for explicit 'none' marker for empty argument lists
            if (match_token(parser, TOKEN_NONE)) {
                // Explicit none for empty argument list (semantic clarity)
                advance_token(parser);
                args = NULL;
                arg_count = 0;
            } else if (match_token(parser, TOKEN_VOID)) {
                // Legacy compatibility error with helpful message
                report_error(
                    parser,
                    "Unexpected 'void' in function arguments. Use 'none' for empty argument lists");
                ast_free_node(expr);
                return NULL;
            } else {
                // Parse actual function arguments
                args = malloc(arg_capacity * sizeof(ASTNode *));
                if (!args) {
                    ast_free_node(expr);
                    return NULL;
                }

                while (!match_token(parser, TOKEN_RIGHT_PAREN) && !at_end(parser)) {
                    ASTNode *arg = parse_expr(parser);
                    if (!arg) {
                        for (size_t i = 0; i < arg_count; i++) {
                            ast_free_node(args[i]);
                        }
                        free(args);
                        ast_free_node(expr);
                        return NULL;
                    }

                    if (arg_count >= arg_capacity) {
                        arg_capacity *= 2;
                        args = realloc(args, arg_capacity * sizeof(ASTNode *));
                        if (!args) {
                            ast_free_node(arg);
                            ast_free_node(expr);
                            return NULL;
                        }
                    }

                    args[arg_count++] = arg;

                    if (!match_token(parser, TOKEN_COMMA)) {
                        break;
                    } else {
                        advance_token(parser);
                    }
                }
            }

            if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
                if (args) {
                    for (size_t i = 0; i < arg_count; i++) {
                        ast_free_node(args[i]);
                    }
                    free(args);
                }
                ast_free_node(expr);
                return NULL;
            }

            ASTNode *call = ast_create_node(AST_CALL_EXPR, op_loc);
            if (!call) {
                if (args) {
                    for (size_t i = 0; i < arg_count; i++) {
                        ast_free_node(args[i]);
                    }
                    free(args);
                }
                ast_free_node(expr);
                return NULL;
            }

            // Special handling for associated function calls
            if (expr->type == AST_ASSOCIATED_FUNC_CALL) {
                // For associated function calls, populate the arguments directly
                // instead of wrapping in another call expression
                if (args && arg_count > 0) {
                    expr->data.associated_func_call.args = ast_node_list_create(arg_count);
                    if (expr->data.associated_func_call.args) {
                        for (size_t i = 0; i < arg_count; i++) {
                            ast_node_list_add(&expr->data.associated_func_call.args, args[i]);
                        }
                    }
                    free(args);
                } else {
                    expr->data.associated_func_call.args = NULL;
                }

                // Free the call node we don't need
                ast_free_node(call);
                // expr remains the same but now has arguments
            } else {
                // Regular function call
                call->data.call_expr.function = expr;

                // Convert args array to ASTNodeList
                if (args && arg_count > 0) {
                    call->data.call_expr.args = ast_node_list_create(arg_count);
                    if (call->data.call_expr.args) {
                        for (size_t i = 0; i < arg_count; i++) {
                            ast_node_list_add(&call->data.call_expr.args, args[i]);
                        }
                    }
                    free(args);
                } else {
                    call->data.call_expr.args = NULL;
                }

                expr = call;
            }
        } else if (check_token(parser, TOKEN_LEFT_BRACE) &&
                   (expr->type == AST_STRUCT_TYPE || expr->type == AST_ENUM_TYPE ||
                    expr->type == AST_IDENTIFIER)) {
            // Struct literal handling with improved lookahead to avoid for-loop conflicts

            bool is_struct_literal = false;

            Token next_token = peek_token_ahead(parser, 1); // Token after {

            if (next_token.type == TOKEN_RIGHT_BRACE) {
                // Empty struct literal: StructName { }
                is_struct_literal = true;
            } else if (next_token.type == TOKEN_IDENTIFIER && expr->type == AST_IDENTIFIER) {
                // For non-empty struct literals, use heuristics to distinguish from other
                // constructs This is a conservative approach that checks for common patterns

                const char *struct_name = expr->data.identifier.name;
                if (struct_name) {
                    // Check if this identifier is a registered type (struct or enum)
                    ASTNode *symbol = lookup_symbol(parser, struct_name);
                    if (symbol &&
                        (symbol->type == AST_STRUCT_DECL || symbol->type == AST_ENUM_DECL)) {
                        is_struct_literal = true;
                    } else {
                        // Don't use uppercase heuristic - it incorrectly treats
                        // uppercase constants like DEBUG_MODE followed by { as struct literals
                        // Only parse as struct literal if we have a registered type
                        // For lowercase identifiers that aren't registered types,
                        // be more conservative to avoid false positives with for-loops

                        // Check for common struct-like field names
                        const char *field_name = next_token.data.identifier.name;
                        if (field_name &&
                            (strcmp(field_name, "width") == 0 ||
                             strcmp(field_name, "height") == 0 || strcmp(field_name, "size") == 0 ||
                             strcmp(field_name, "length") == 0 ||
                             strcmp(field_name, "count") == 0 || strcmp(field_name, "id") == 0 ||
                             strcmp(field_name, "key") == 0 || strcmp(field_name, "message") == 0 ||
                             strcmp(field_name, "error") == 0 || strcmp(field_name, "field") == 0 ||
                             strcmp(field_name, "fields") == 0 ||
                             strcmp(field_name, "level") == 0)) {
                            is_struct_literal = true;
                        }
                        // If no common field patterns, assume it's not a struct literal
                        // This helps avoid parsing "for x in collection {" as a struct literal
                    }
                }
            }

            if (is_struct_literal) {
                // Parse as struct literal
                SourceLocation struct_loc = expr->location;
                char *struct_name = NULL;
                ASTNodeList *type_args = NULL;

                if (expr->type == AST_IDENTIFIER) {
                    // Simple struct literal: StructName { ... }
                    struct_name = strdup(expr->data.identifier.name);
                    type_args = NULL;
                } else if (expr->type == AST_STRUCT_TYPE) {
                    // Generic struct literal: StructName<T> { ... }
                    struct_name = strdup(expr->data.struct_type.name);
                    type_args = expr->data.struct_type.type_args;
                    expr->data.struct_type.type_args = NULL; // Transfer ownership
                } else if (expr->type == AST_ENUM_TYPE) {
                    // Potential generic struct literal parsed as enum type: StructName<T> { ... }
                    struct_name = strdup(expr->data.enum_type.name);
                    type_args = expr->data.enum_type.type_args;
                    expr->data.enum_type.type_args = NULL; // Transfer ownership
                }

                // We've only checked for TOKEN_LEFT_BRACE, not consumed it
                // The struct literal parser will consume it

                // Parse struct literal
                ASTNode *struct_literal = parse_struct_literal_with_name_and_type_args(
                    parser, struct_name, type_args, struct_loc);

                if (struct_literal) {
                    // Success! Free the original expression and use struct literal
                    ast_free_node(expr);
                    expr = struct_literal;
                } else {
                    // Failed to parse as struct literal
                    // Since we're in expression context, this is an error
                    // The error has already been reported by
                    // parse_struct_literal_with_name_and_type_args
                    ast_free_node(expr);
                    return NULL;
                }
            } else {
                // Not a struct literal - stop postfix parsing
                break;
            }
        } else if (match_token(parser, TOKEN_DOUBLE_COLON)) {
            // Reject postfix :: usage - this violates current grammar
            // Check if this might be an enum variant usage and provide helpful message
            bool might_be_enum_variant = false;
            if (expr->type == AST_IDENTIFIER) {
                // Simplified check - if the next token after :: is an identifier,
                // this might be intended as an enum variant
                might_be_enum_variant = peek_token(parser).type == TOKEN_IDENTIFIER;
            }

            if (might_be_enum_variant) {
                report_error(parser, "Invalid postfix '::' usage. Use '.' instead of '::' for enum "
                                     "variants (e.g., Result.Ok instead of Result::Ok)");
            } else {
                report_error(parser,
                             "Invalid postfix '::' usage. Use '::' only for type-level associated "
                             "functions like 'Type::function()' or 'Type<T>::function()'");
            }
            ast_free_node(expr);
            return NULL;
        } else {
            break;
        }
    }

    return expr;
}