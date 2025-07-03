/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Impl blocks and method declarations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include "grammar_statements.h"
#include "grammar_toplevel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// IMPL BLOCK AND METHOD DECLARATION PARSING
// =============================================================================

ASTNode *parse_impl_block(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_IMPL)) {
        return NULL;
    }

    // Parse struct name
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected struct name after 'impl'");
        return NULL;
    }

    char *struct_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        free(struct_name);
        return NULL;
    }

    // Parse methods
    ASTNode **methods = NULL;
    size_t method_count = 0;
    size_t method_capacity = 4;

    methods = malloc(method_capacity * sizeof(ASTNode *));
    if (!methods) {
        free(struct_name);
        return NULL;
    }

    // Parse method declarations until we hit the closing brace
    while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
        // Parse annotations first (methods can have annotations)
        ASTNodeList *annotations = parse_annotation_list(parser);

        // Parse method visibility - must be explicit
        VisibilityType method_visibility = VISIBILITY_PRIVATE;
        if (match_token(parser, TOKEN_PUB)) {
            method_visibility = VISIBILITY_PUBLIC;
            advance_token(parser);
        } else if (match_token(parser, TOKEN_PRIV)) {
            method_visibility = VISIBILITY_PRIVATE;
            advance_token(parser);
        } else if (match_token(parser, TOKEN_FN)) {
            // Method without visibility modifier
            report_error(parser, "methods must have explicit visibility");

            // Free annotations on error
            if (annotations) {
                ast_node_list_destroy(annotations);
            }

            // Free methods array
            for (size_t i = 0; i < method_count; i++) {
                ast_free_node(methods[i]);
            }
            free(methods);
            free(struct_name);
            return NULL;
        }

        // Expect function declaration
        if (!match_token(parser, TOKEN_FN)) {
            report_error(parser, "Expected method declaration in impl block");

            // Free annotations on error
            if (annotations) {
                ast_node_list_destroy(annotations);
            }

            // Free methods array
            for (size_t i = 0; i < method_count; i++) {
                ast_free_node(methods[i]);
            }
            free(methods);
            free(struct_name);
            return NULL;
        }

        // Parse method declaration
        ASTNode *method = parse_method_decl(parser);
        if (!method) {
            // Free annotations on error
            if (annotations) {
                ast_node_list_destroy(annotations);
            }

            // Free methods array
            for (size_t i = 0; i < method_count; i++) {
                ast_free_node(methods[i]);
            }
            free(methods);
            free(struct_name);
            return NULL;
        }

        // Set annotations and visibility on the method
        method->data.method_decl.annotations = annotations;
        method->data.method_decl.visibility = method_visibility;

        // Expand methods array if needed
        if (method_count >= method_capacity) {
            method_capacity *= 2;
            methods = realloc(methods, method_capacity * sizeof(ASTNode *));
            if (!methods) {
                ast_free_node(method);
                free(struct_name);
                return NULL;
            }
        }

        methods[method_count++] = method;
    }

    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        for (size_t i = 0; i < method_count; i++) {
            ast_free_node(methods[i]);
        }
        free(methods);
        free(struct_name);
        return NULL;
    }

    // Create impl block node
    ASTNode *node = ast_create_node(AST_IMPL_BLOCK, start_loc);
    if (!node) {
        for (size_t i = 0; i < method_count; i++) {
            ast_free_node(methods[i]);
        }
        free(methods);
        free(struct_name);
        return NULL;
    }

    node->data.impl_block.struct_name = struct_name;
    node->data.impl_block.annotations =
        NULL; // impl blocks don't have annotations in current design

    // Convert methods array to ASTNodeList
    if (methods && method_count > 0) {
        node->data.impl_block.methods = ast_node_list_create(method_count);
        if (node->data.impl_block.methods) {
            for (size_t i = 0; i < method_count; i++) {
                ast_node_list_add(&node->data.impl_block.methods, methods[i]);
            }
        }
        free(methods);
    } else {
        node->data.impl_block.methods = NULL;
    }

    return node;
}

ASTNode *parse_method_decl(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_FN)) {
        return NULL;
    }

    // Parse method name
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected method name");
        return NULL;
    }

    char *method_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        free(method_name);
        return NULL;
    }

    // Parse method parameter list according to v1.25 grammar: MethodParamList <- SelfParam (','
    // Param)* / ParamList
    ASTNodeList *params = NULL;
    bool is_instance_method = false;

    if (match_token(parser, TOKEN_NONE)) {
        // Explicit none for empty parameter list (v1.19 semantic clarity)
        advance_token(parser);
        params = NULL; // No parameters
        is_instance_method = false;
    } else if (match_token(parser, TOKEN_VOID)) {
        // v1.18 compatibility error with helpful message
        report_error(
            parser,
            "Unexpected 'void' in method parameter list. Use 'none' for empty parameters in v1.19");
        free(method_name);
        return NULL;
    } else if (match_token(parser, TOKEN_SELF)) {
        // Instance method with self parameter: SelfParam (',' Param)*
        is_instance_method = true;
        params = ast_node_list_create(2);
        if (!params) {
            free(method_name);
            return NULL;
        }

        // Create self parameter
        SourceLocation self_loc = parser->current_token.location;
        advance_token(parser);

        ASTNode *self_param = ast_create_node(AST_PARAM_DECL, self_loc);
        if (!self_param) {
            ast_node_list_destroy(params);
            free(method_name);
            return NULL;
        }

        self_param->data.param_decl.name = strdup("self");
        self_param->data.param_decl.type = NULL; // self type is inferred
        self_param->data.param_decl.annotations = NULL;

        ast_node_list_add(&params, self_param);

        // Parse additional parameters after self: (',' Param)*
        while (match_token(parser, TOKEN_COMMA)) {
            advance_token(parser);

            ASTNode *param = parse_param(parser);
            if (!param) {
                ast_node_list_destroy(params);
                free(method_name);
                return NULL;
            }

            ast_node_list_add(&params, param);
        }
    } else {
        // Static method using regular ParamList
        is_instance_method = false;
        params = ast_node_list_create(2);
        if (!params) {
            free(method_name);
            return NULL;
        }

        do {
            ASTNode *param = parse_param(parser);
            if (!param) {
                ast_node_list_destroy(params);
                free(method_name);
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
        ast_node_list_destroy(params);
        free(method_name);
        return NULL;
    }

    // Parse optional return type
    ASTNode *return_type = NULL;
    if (match_token(parser, TOKEN_ARROW)) {
        advance_token(parser);
        return_type = parse_type(parser);
        if (!return_type) {
            ast_node_list_destroy(params);
            free(method_name);
            return NULL;
        }
    }

    // Parse method body
    ASTNode *body = parse_block(parser);
    if (!body) {
        if (return_type)
            ast_free_node(return_type);
        ast_node_list_destroy(params);
        free(method_name);
        return NULL;
    }

    // Create method declaration node
    ASTNode *node = ast_create_node(AST_METHOD_DECL, start_loc);
    if (!node) {
        ast_free_node(body);
        if (return_type)
            ast_free_node(return_type);
        ast_node_list_destroy(params);
        free(method_name);
        return NULL;
    }

    node->data.method_decl.name = method_name;
    node->data.method_decl.return_type = return_type;
    node->data.method_decl.body = body;
    node->data.method_decl.is_instance_method = is_instance_method;
    node->data.method_decl.annotations = NULL; // Will be set by caller

    // Assign parameter list directly (v1.18 implementation)
    node->data.method_decl.params = params;

    return node;
}
