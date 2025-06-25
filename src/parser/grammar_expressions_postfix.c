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
                // Regular field access
                field_name = strdup(parser->current_token.data.identifier.name);
                advance_token(parser);
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
        } else if (match_token(parser, TOKEN_LEFT_BRACE) &&
                   (expr->type == AST_IDENTIFIER || expr->type == AST_STRUCT_TYPE ||
                    expr->type == AST_ENUM_TYPE)) {
            // Struct literal: Identifier { ... }, StructType<T> { ... }, or EnumType<T> { ... }
            // Use safer lookahead to distinguish from blocks that happen to follow identifiers

            bool is_struct_literal = false;

            // Look ahead to check if this is actually a struct literal
            // A struct literal has the pattern: field_name : expression
            // Use peek to avoid consuming tokens
            Token peek1 = peek_token(parser); // This should be the token after '{'

            if (peek1.type == TOKEN_RIGHT_BRACE) {
                // Empty braces {} - not a struct literal in this context
                is_struct_literal = false;
            } else if (peek1.type == TOKEN_IDENTIFIER) {
                // We have { identifier ..., need to check if followed by colon
                // We need to peek further ahead, but we only have single token lookahead
                // So we'll use a conservative heuristic: assume it's a struct literal
                // only if we're parsing a known struct type expression

                // For now, be conservative and only treat it as struct literal if
                // the expression is a struct type (not just any identifier)
                if (expr->type == AST_STRUCT_TYPE || expr->type == AST_ENUM_TYPE) {
                    // Generic types are more likely to be struct literals
                    is_struct_literal = true;
                } else if (expr->type == AST_IDENTIFIER) {
                    // For plain identifiers, check if it starts with uppercase (type convention)
                    const char *name = expr->data.identifier.name;
                    if (name && name[0] >= 'A' && name[0] <= 'Z') {
                        // Uppercase identifiers are likely type names
                        is_struct_literal = true;
                    } else {
                        // Lowercase identifiers in match context are likely not struct literals
                        is_struct_literal = false;
                    }
                } else {
                    is_struct_literal = false;
                }
            } else {
                // Not starting with identifier, definitely not a struct literal
                is_struct_literal = false;
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

                // Free the original expression since we're replacing it
                ast_free_node(expr);

                // Parse the struct literal with the extracted name and type args
                ASTNode *struct_literal = parse_struct_literal_with_name_and_type_args(
                    parser, struct_name, type_args, struct_loc);
                if (!struct_literal) {
                    return NULL;
                }

                expr = struct_literal;
            } else {
                // Not a struct literal - stop postfix parsing and let the caller handle the brace
                break;
            }
        } else if (match_token(parser, TOKEN_DOUBLE_COLON)) {
            // Reject postfix :: usage - this violates current grammar
            report_error(parser,
                         "Invalid postfix '::' usage. Use '::' only for type-level associated "
                         "functions like 'Type::function()' or 'Type<T>::function()'");
            ast_free_node(expr);
            return NULL;
        } else {
            break;
        }
    }

    return expr;
}