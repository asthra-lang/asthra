/**
 * Asthra Programming Language Compiler - FFI Annotation Parsing
 * Parsing FFI transfer and lifetime annotations for safe C interop
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// FFI ANNOTATION PARSING
// =============================================================================

ASTNode *parse_ffi_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // Expect "ffi" identifier
    if (!match_token(parser, TOKEN_IDENTIFIER) ||
        strcmp(parser->current_token.data.identifier.name, "ffi") != 0) {
        report_error(parser, "Expected 'ffi' annotation");
        return NULL;
    }
    advance_token(parser);

    if (!expect_token(parser, TOKEN_LEFT_PAREN)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected FFI transfer type");
        return NULL;
    }

    FFITransferType transfer_type;
    const char *transfer_name = parser->current_token.data.identifier.name;

    if (strcmp(transfer_name, "full") == 0) {
        transfer_type = FFI_TRANSFER_FULL;
    } else if (strcmp(transfer_name, "none") == 0) {
        transfer_type = FFI_TRANSFER_NONE;
    } else if (strcmp(transfer_name, "borrowed") == 0) {
        transfer_type = FFI_BORROWED;
    } else {
        report_error(parser, "Unknown FFI transfer type");
        return NULL;
    }

    advance_token(parser);

    if (!expect_token(parser, TOKEN_RIGHT_PAREN)) {
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_FFI_ANNOTATION, start_loc);
    if (!node)
        return NULL;

    node->data.ffi_annotation.transfer_type = transfer_type;

    return node;
}

// NEW: SafeFFIAnnotation parser implementing the fixed grammar
ASTNode *parse_safe_ffi_annotation(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    // SafeFFIAnnotation <- FFITransferTag / LifetimeTag
    // FFITransferTag <- '#[' ('transfer_full' | 'transfer_none') ']'
    // LifetimeTag    <- '#[borrowed]'

    if (!expect_token(parser, TOKEN_HASH)) {
        return NULL;
    }

    if (!expect_token(parser, TOKEN_LEFT_BRACKET)) {
        return NULL;
    }

    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected FFI annotation identifier");
        return NULL;
    }

    FFITransferType transfer_type;
    const char *annotation_name = parser->current_token.data.identifier.name;

    if (strcmp(annotation_name, "transfer_full") == 0) {
        transfer_type = FFI_TRANSFER_FULL;
    } else if (strcmp(annotation_name, "transfer_none") == 0) {
        transfer_type = FFI_TRANSFER_NONE;
    } else if (strcmp(annotation_name, "borrowed") == 0) {
        transfer_type = FFI_BORROWED;
    } else {
        report_error(
            parser,
            "Unknown FFI annotation. Expected 'transfer_full', 'transfer_none', or 'borrowed'");
        return NULL;
    }

    advance_token(parser);

    if (!expect_token(parser, TOKEN_RIGHT_BRACKET)) {
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_FFI_ANNOTATION, start_loc);
    if (!node)
        return NULL;

    node->data.ffi_annotation.transfer_type = transfer_type;

    return node;
}