/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Program, package declarations, and imports
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include "grammar_statements.h"
#include "grammar_toplevel.h"
#include "parser_error.h" // Phase 1: Import System Enhancement
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// PROGRAM STRUCTURE PRODUCTIONS
// =============================================================================

ASTNode *parse_program(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;
    ASTNode *program = ast_create_node(AST_PROGRAM, start_loc);
    if (!program)
        return NULL;

    // Parse package declaration
    program->data.program.package_decl = parse_package_decl(parser);
    if (!program->data.program.package_decl) {
        ast_free_node(program);
        return NULL;
    }

    // Parse imports
    ASTNode **imports = NULL;
    size_t import_count = 0;
    size_t import_capacity = 4;

    imports = malloc(import_capacity * sizeof(ASTNode *));
    if (!imports) {
        ast_free_node(program);
        return NULL;
    }

    while (match_token(parser, TOKEN_IMPORT)) {
        ASTNode *import = parse_import_decl(parser);
        if (!import) {
            if (!parser->config.allow_incomplete_parse) {
                for (size_t i = 0; i < import_count; i++) {
                    ast_free_node(imports[i]);
                }
                free(imports);
                ast_free_node(program);
                return NULL;
            }
            synchronize(parser);
            continue;
        }

        if (import_count >= import_capacity) {
            import_capacity *= 2;
            imports = realloc(imports, import_capacity * sizeof(ASTNode *));
            if (!imports) {
                ast_free_node(import);
                ast_free_node(program);
                return NULL;
            }
        }

        imports[import_count++] = import;
    }

    // Convert array to ASTNodeList
    program->data.program.imports = ast_node_list_create(import_count);
    if (!program->data.program.imports) {
        for (size_t i = 0; i < import_count; i++) {
            ast_free_node(imports[i]);
        }
        free(imports);
        ast_free_node(program);
        return NULL;
    }

    for (size_t i = 0; i < import_count; i++) {
        ast_node_list_add(&program->data.program.imports, imports[i]);
    }
    free(imports); // Free the temporary array

    // Parse top-level declarations
    ASTNode **declarations = NULL;
    size_t declaration_count = 0;
    size_t declaration_capacity = 8;

    declarations = malloc(declaration_capacity * sizeof(ASTNode *));
    if (!declarations) {
        ast_free_node(program);
        return NULL;
    }

    while (!at_end(parser)) {
        // Use parse_top_level_decl which handles annotations properly
        ASTNode *decl = parse_top_level_decl(parser);

        if (decl) {
            if (declaration_count >= declaration_capacity) {
                declaration_capacity *= 2;
                declarations = realloc(declarations, declaration_capacity * sizeof(ASTNode *));
                if (!declarations) {
                    ast_free_node(decl);
                    ast_free_node(program);
                    return NULL;
                }
            }

            declarations[declaration_count++] = decl;
        } else if (!parser->config.allow_incomplete_parse && parser->had_error) {
            // In strict mode, fail immediately on parse errors
            for (size_t i = 0; i < declaration_count; i++) {
                ast_free_node(declarations[i]);
            }
            free(declarations);
            ast_free_node(program);
            return NULL;
        }
    }

    // Convert array to ASTNodeList
    program->data.program.declarations = ast_node_list_create(declaration_count);
    if (!program->data.program.declarations) {
        for (size_t i = 0; i < declaration_count; i++) {
            ast_free_node(declarations[i]);
        }
        free(declarations);
        ast_free_node(program);
        return NULL;
    }

    for (size_t i = 0; i < declaration_count; i++) {
        ast_node_list_add(&program->data.program.declarations, declarations[i]);
    }
    free(declarations); // Free the temporary array

    return program;
}

ASTNode *parse_package_decl(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_PACKAGE)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected package name");
        return NULL;
    }

    char *package_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        free(package_name);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_PACKAGE_DECL, start_loc);
    if (!node) {
        free(package_name);
        return NULL;
    }

    node->data.package_decl.name = package_name;

    return node;
}

ASTNode *parse_import_decl(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_IMPORT)) {
        return NULL;
    }

    char *import_path = NULL;
    char *alias = NULL;

    if (match_token(parser, TOKEN_STRING)) {
        import_path = strdup(parser->current_token.data.string.value);
        advance_token(parser);

        // Phase 1: Enhanced parser-level validation - Early validation
        if (!validate_import_path_format(import_path)) {
            // Determine specific error type for better reporting
            ImportPathErrorType error_type = IMPORT_ERROR_INVALID_STDLIB;

            if (strlen(import_path) == 0) {
                error_type = IMPORT_ERROR_EMPTY_PATH;
            } else if (strstr(import_path, "  ") || import_path[0] == ' ' ||
                       import_path[strlen(import_path) - 1] == ' ') {
                error_type = IMPORT_ERROR_WHITESPACE;
            } else if (strncmp(import_path, "internal/", 9) == 0) {
                error_type = IMPORT_ERROR_INTERNAL_ACCESS_DENIED;
            } else if (strstr(import_path, "github.com/") || strstr(import_path, "gitlab.com/")) {
                error_type = IMPORT_ERROR_INVALID_GITHUB;
            } else if (strncmp(import_path, "stdlib/", 7) == 0) {
                error_type = IMPORT_ERROR_INVALID_STDLIB;
            } else if (import_path[0] == '.' &&
                       (import_path[1] == '/' ||
                        (import_path[1] == '.' && import_path[2] == '/'))) {
                error_type = IMPORT_ERROR_INVALID_LOCAL;
            }

            report_import_path_error(parser, error_type, import_path, start_loc.line);
            free(import_path);
            return NULL;
        }
    } else {
        report_error(parser, "Expected import path string");
        return NULL;
    }

    // Parse optional alias: "as identifier"
    if (match_token(parser, TOKEN_AS)) {
        advance_token(parser);

        if (!match_token(parser, TOKEN_IDENTIFIER)) {
            report_error(parser, "Expected identifier after 'as' in import alias");
            free(import_path);
            return NULL;
        }

        alias = strdup(parser->current_token.data.identifier.name);
        advance_token(parser);
    }

    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        free(import_path);
        free(alias);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_IMPORT_DECL, start_loc);
    if (!node) {
        free(import_path);
        free(alias);
        return NULL;
    }

    node->data.import_decl.path = import_path;
    node->data.import_decl.alias = alias;

    return node;
}

ASTNode *parse_top_level_decl(Parser *parser) {
    if (!parser)
        return NULL;

    // Parse annotations first using the function from grammar_annotations.h
    ASTNodeList *annotations = parse_annotation_list(parser);

    ASTNode *decl = NULL;
    VisibilityType visibility;

    // Check for type aliases early (before visibility check)
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        const char *id = parser->current_token.data.identifier.name;
        if (id && strcmp(id, "type") == 0) {
            report_error(parser, "Type aliases are not yet implemented. Type aliases and generic "
                                 "constraints are planned future features.");
            // Try to recover by skipping the type alias
            advance_token(parser); // skip 'type'
            while (!match_token(parser, TOKEN_SEMICOLON) && !at_end(parser)) {
                advance_token(parser);
            }
            if (match_token(parser, TOKEN_SEMICOLON)) {
                advance_token(parser);
            }
            // Free annotations on error
            if (annotations) {
                ast_node_list_destroy(annotations);
            }
            return NULL;
        }
    }

    // Check for impl blocks early (they don't require visibility modifiers)
    if (match_token(parser, TOKEN_IMPL)) {
        decl = parse_impl_block(parser);
        // Note: impl_block doesn't have visibility field in AST

        if (decl && annotations) {
            // Attach annotations to the impl block
            decl->data.impl_block.annotations = annotations;
        } else if (annotations) {
            // Free annotations on error
            ast_node_list_destroy(annotations);
        }

        return decl;
    }

    // Parse required visibility modifier (v1.25 grammar enforcement)
    if (match_token(parser, TOKEN_PUB)) {
        visibility = VISIBILITY_PUBLIC;
        advance_token(parser);
    } else if (match_token(parser, TOKEN_PRIV)) {
        visibility = VISIBILITY_PRIVATE;
        advance_token(parser);
    } else {
        report_error(parser,
                     "Expected explicit visibility modifier 'pub' or 'priv' (grammar requirement)");

        // Free annotations on error
        if (annotations) {
            ast_node_list_destroy(annotations);
        }

        // Skip the current token first to avoid getting stuck
        // on tokens that synchronize() would stop at immediately
        if (!at_end(parser)) {
            advance_token(parser);
        }

        // Then synchronize to find a good recovery point
        synchronize(parser);

        return NULL;
    }

    if (match_token(parser, TOKEN_FN)) {
        decl = parse_function_decl(parser);
        if (decl) {
            decl->data.function_decl.visibility = visibility; // Set visibility
        }
    } else if (match_token(parser, TOKEN_STRUCT)) {
        decl = parse_struct_decl(parser);
        if (decl) {
            decl->data.struct_decl.visibility = visibility; // Set visibility
        }
    } else if (match_token(parser, TOKEN_ENUM)) {
        decl = parse_enum_decl(parser);
        if (decl) {
            decl->data.enum_decl.visibility = visibility; // Set visibility
        }
    } else if (match_token(parser, TOKEN_EXTERN)) {
        decl = parse_extern_decl(parser);
        // Note: extern_decl doesn't have visibility field in AST
    } else if (match_token(parser, TOKEN_CONST)) {
        decl = parse_const_decl(parser);
        if (decl) {
            decl->data.const_decl.visibility = visibility; // Set visibility
        }
    } else if (match_token(parser, TOKEN_IMPL)) {
        decl = parse_impl_block(parser);
        // Note: impl_block doesn't have visibility field in AST, but we can store it in a comment
        // or extend the AST For now, we'll just parse it successfully - visibility will be handled
        // by methods inside
    } else {
        report_error(parser, "Expected function, struct, enum, extern, const, or impl declaration");

        // Free annotations on error
        if (annotations) {
            ast_node_list_destroy(annotations);
        }

        // Skip the current token first to avoid getting stuck
        if (!at_end(parser)) {
            advance_token(parser);
        }

        // Then synchronize to find a good recovery point
        synchronize(parser);

        return NULL;
    }

    if (decl && annotations) {
        // Attach annotations to the declaration
        switch (decl->type) {
        case AST_FUNCTION_DECL:
            decl->data.function_decl.annotations = annotations;
            break;
        case AST_STRUCT_DECL:
            decl->data.struct_decl.annotations = annotations;
            break;
        case AST_ENUM_DECL:
            decl->data.enum_decl.annotations = annotations;
            break;
        case AST_EXTERN_DECL:
            decl->data.extern_decl.annotations = annotations;
            break;
        case AST_IMPL_BLOCK:
            decl->data.impl_block.annotations = annotations;
            break;
        case AST_CONST_DECL:
            decl->data.const_decl.annotations = annotations;
            break;
        default:
            // Free annotations if we can't attach them
            ast_node_list_destroy(annotations);
            break;
        }
    }

    return decl;
}

// =============================================================================
// MODULE SYSTEM (missing implementations)
// =============================================================================

ASTNode *parse_visibility_modifier(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!match_token(parser, TOKEN_PUB)) {
        return NULL; // No visibility modifier
    }

    advance_token(parser);

    ASTNode *node = ast_create_node(AST_VISIBILITY_MODIFIER, start_loc);
    if (!node) {
        return NULL;
    }

    node->data.visibility_modifier.is_public = true;

    return node;
}
