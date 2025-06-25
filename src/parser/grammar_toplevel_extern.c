/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Extern declarations for FFI
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h" // Add include for SafeFFIAnnotation parser
#include "grammar_toplevel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// EXTERN DECLARATION PARSING
// =============================================================================

ASTNode *parse_extern_decl(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_EXTERN)) {
        return NULL;
    }

    // Optional library name string
    char *extern_name = NULL;
    if (match_token(parser, TOKEN_STRING)) {
        extern_name = strdup(parser->current_token.data.string.value);
        advance_token(parser);
    }

    if (!expect_token(parser, TOKEN_FN)) {
        free(extern_name);
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected function name");
        free(extern_name);
        return NULL;
    }

    char *function_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        free(function_name);
        free(extern_name);
        return NULL;
    }

    // Parse extern parameter list according to v1.25 grammar: ExternParamList <- ExternParam (','
    // ExternParam)* / 'none'
    ASTNodeList *params = NULL;

    if (match_token(parser, TOKEN_NONE)) {
        // Explicit none for empty parameter list (v1.25 semantic clarity)
        advance_token(parser);
        params = NULL; // No parameters
    } else if (match_token(parser, TOKEN_VOID)) {
        // Legacy compatibility error with helpful message
        report_error(parser,
                     "Unexpected 'void' in extern parameter list. Use 'none' for empty parameters");
        return NULL;
    } else {
        // Parse actual extern parameters
        params = ast_node_list_create(2);
        if (!params) {
            free(function_name);
            free(extern_name);
            return NULL;
        }

        do {
            // Check for variadic function syntax and provide helpful error
            if (match_token(parser, TOKEN_ELLIPSIS)) {
                report_error(parser, "Variadic functions are not supported in Asthra. "
                                     "Use explicit function overloads instead. "
                                     "See documentation for C interop patterns.");
                ast_node_list_destroy(params);
                free(function_name);
                free(extern_name);
                return NULL;
            }

            ASTNode *param = parse_extern_param(parser);
            if (!param) {
                ast_node_list_destroy(params);
                free(function_name);
                free(extern_name);
                return NULL;
            }

            ast_node_list_add(&params, param);

            if (match_token(parser, TOKEN_COMMA)) {
                advance_token(parser);
            } else {
                break;
            }
        } while (!at_end(parser) && !match_token(parser, TOKEN_RIGHT_PAREN));
    }

    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        if (params)
            ast_node_list_destroy(params);
        free(function_name);
        free(extern_name);
        return NULL;
    }

    // Parse return type with optional SafeFFIAnnotation
    ASTNode *return_type = NULL;
    ASTNode *return_annotation = NULL;

    if (match_token(parser, TOKEN_ARROW)) {
        advance_token(parser);

        // Check for SafeFFIAnnotation before type (Grammar: '->' SafeFFIAnnotation? Type)
        if (match_token(parser, TOKEN_HASH)) {
            return_annotation = parse_safe_ffi_annotation(parser);
            if (!return_annotation) {
                free(function_name);
                free(extern_name);
                return NULL;
            }
        }

        return_type = parse_type(parser);
        if (!return_type) {
            free(function_name);
            free(extern_name);
            if (return_annotation)
                ast_free_node(return_annotation);
            return NULL;
        }
    }

    // Expect semicolon to end extern declaration
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        free(function_name);
        free(extern_name);
        if (return_type)
            ast_free_node(return_type);
        if (return_annotation)
            ast_free_node(return_annotation);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_EXTERN_DECL, start_loc);
    if (!node) {
        free(function_name);
        free(extern_name);
        if (return_type)
            ast_free_node(return_type);
        if (return_annotation)
            ast_free_node(return_annotation);
        return NULL;
    }

    node->data.extern_decl.name = function_name;
    node->data.extern_decl.extern_name = extern_name;
    node->data.extern_decl.params = params;
    node->data.extern_decl.return_type = return_type;

    // Store return type annotation in annotations list
    if (return_annotation) {
        node->data.extern_decl.annotations = ast_node_list_create(1);
        if (node->data.extern_decl.annotations) {
            ast_node_list_add(&node->data.extern_decl.annotations, return_annotation);
        } else {
            ast_free_node(return_annotation);
        }
    } else {
        node->data.extern_decl.annotations = NULL;
    }

    return node;
}

// =============================================================================
// EXTERN PARAMETER PARSING (v1.18 Grammar Implementation)
// =============================================================================

ASTNode *parse_extern_param(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Parse optional SafeFFIAnnotation (Grammar: SafeFFIAnnotation? SimpleIdent ':' Type)
    ASTNode *annotation = NULL;
    if (match_token(parser, TOKEN_HASH)) {
        annotation = parse_safe_ffi_annotation(parser);
        if (!annotation) {
            return NULL;
        }
    }

    // v1.25: Check for invalid mut in extern parameter (extern parameters are always immutable)
    if (match_token(parser, TOKEN_MUT)) {
        report_error(parser, "Extern function parameters cannot have 'mut' keyword. "
                             "Parameters are always immutable in Asthra. "
                             "Mutable access to extern function parameters should be handled "
                             "through pointer types.");
        if (annotation)
            ast_free_node(annotation);
        return NULL;
    }

    // Parse parameter name (SimpleIdent)
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected extern parameter name");
        if (annotation)
            ast_free_node(annotation);
        return NULL;
    }

    char *param_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    // Parse colon
    if (!expect_token(parser, TOKEN_COLON)) {
        free(param_name);
        if (annotation)
            ast_free_node(annotation);
        return NULL;
    }

    // Parse parameter type
    ASTNode *param_type = parse_type(parser);
    if (!param_type) {
        free(param_name);
        if (annotation)
            ast_free_node(annotation);
        return NULL;
    }

    // Create extern parameter declaration node
    ASTNode *param = ast_create_node(AST_PARAM_DECL, start_loc);
    if (!param) {
        free(param_name);
        ast_free_node(param_type);
        if (annotation)
            ast_free_node(annotation);
        return NULL;
    }

    param->data.param_decl.name = param_name;
    param->data.param_decl.type = param_type;

    // Store annotation in annotations list if present
    if (annotation) {
        param->data.param_decl.annotations = ast_node_list_create(1);
        if (param->data.param_decl.annotations) {
            ast_node_list_add(&param->data.param_decl.annotations, annotation);
        } else {
            ast_free_node(annotation);
            param->data.param_decl.annotations = NULL;
        }
    } else {
        param->data.param_decl.annotations = NULL;
    }

    return param;
}
