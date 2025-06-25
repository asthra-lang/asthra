/**
 * Asthra Programming Language Compiler - Ownership Annotation Parsing
 * Parsing ownership memory management annotations (#gc, #c, #pinned)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// OWNERSHIP ANNOTATION PARSING
// =============================================================================

ASTNode *parse_ownership_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected ownership type after '#'");
        return NULL;
    }

    OwnershipType ownership;
    const char *ownership_name = parser->current_token.data.identifier.name;

    if (strcmp(ownership_name, "gc") == 0) {
        ownership = OWNERSHIP_GC;
    } else if (strcmp(ownership_name, "c") == 0) {
        ownership = OWNERSHIP_C;
    } else if (strcmp(ownership_name, "pinned") == 0) {
        ownership = OWNERSHIP_PINNED;
    } else {
        report_error(parser, "Unknown ownership type");
        return NULL;
    }

    advance_token(parser);

    ASTNode *node = ast_create_node(AST_OWNERSHIP_TAG, start_loc);
    if (!node)
        return NULL;

    node->data.ownership_tag.ownership = ownership;

    return node;
}