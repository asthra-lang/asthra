/**
 * Asthra Programming Language Compiler - Security Annotation Parsing
 * Parsing security and human review annotations for safe systems code
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SECURITY ANNOTATION PARSING
// =============================================================================

ASTNode *parse_security_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect "security" identifier
    if (!match_token(parser, TOKEN_IDENTIFIER) ||
        strcmp(parser->current_token.data.identifier.name, "security") != 0) {
        report_error(parser, "Expected 'security' annotation");
        return NULL;
    }
    advance_token(parser);

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected security type");
        return NULL;
    }

    SecurityType security_type;
    const char *security_name = parser->current_token.data.identifier.name;

    if (strcmp(security_name, "constant_time") == 0) {
        security_type = SECURITY_CONSTANT_TIME;
    } else if (strcmp(security_name, "volatile_memory") == 0) {
        security_type = SECURITY_VOLATILE_MEMORY;
    } else {
        report_error(parser, "Unknown security type");
        return NULL;
    }

    advance_token(parser);

    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_SECURITY_TAG, start_loc);
    if (!node)
        return NULL;

    node->data.security_tag.security_type = security_type;

    return node;
}

ASTNode *parse_human_review_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect "review" identifier
    if (!match_token(parser, TOKEN_IDENTIFIER) ||
        strcmp(parser->current_token.data.identifier.name, "review") != 0) {
        report_error(parser, "Expected 'review' annotation");
        return NULL;
    }
    advance_token(parser);

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected review priority");
        return NULL;
    }

    ReviewPriority priority;
    const char *priority_name = parser->current_token.data.identifier.name;

    if (strcmp(priority_name, "low") == 0) {
        priority = REVIEW_LOW;
    } else if (strcmp(priority_name, "medium") == 0) {
        priority = REVIEW_MEDIUM;
    } else if (strcmp(priority_name, "high") == 0) {
        priority = REVIEW_HIGH;
    } else {
        report_error(parser, "Unknown review priority");
        return NULL;
    }

    advance_token(parser);

    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_HUMAN_REVIEW_TAG, start_loc);
    if (!node)
        return NULL;

    node->data.human_review_tag.priority = priority;

    return node;
}