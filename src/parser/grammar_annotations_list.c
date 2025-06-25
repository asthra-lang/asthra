/**
 * Asthra Programming Language Compiler - Annotation List Parsing
 * Parsing lists of annotations and helper functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// ANNOTATION LIST PARSING
// =============================================================================

ASTNodeList *parse_annotation_list(Parser *parser) {
    if (!parser)
        return NULL;

    ASTNode **annotations = NULL;
    size_t annotation_count = 0;
    size_t annotation_capacity = 4;

    // Check if we actually have annotations before allocating
    if (!is_annotation_start(parser)) {
        return NULL;
    }

    annotations = malloc(annotation_capacity * sizeof(ASTNode *));
    if (!annotations)
        return NULL;

    while (is_annotation_start(parser)) {
        ASTNode *annotation = parse_annotation(parser);
        if (!annotation) {
            // If annotation parsing fails, clean up and return what we have
            break;
        }

        if (annotation_count >= annotation_capacity) {
            annotation_capacity *= 2;
            annotations = realloc(annotations, annotation_capacity * sizeof(ASTNode *));
            if (!annotations) {
                ast_free_node(annotation);
                return NULL;
            }
        }

        annotations[annotation_count++] = annotation;
    }

    if (annotation_count == 0) {
        free(annotations);
        return NULL;
    }

    // Convert annotations array to ASTNodeList
    ASTNodeList *list = ast_node_list_create(annotation_count);
    if (list) {
        for (size_t i = 0; i < annotation_count; i++) {
            ast_node_list_add(&list, annotations[i]);
        }
    }
    free(annotations);

    return list;
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

bool is_annotation_start(Parser *parser) {
    if (!parser)
        return false;

    // Only #[...] annotations are supported now
    // Use check_token to avoid advancing the token
    if (check_token(parser, TOKEN_HASH)) {
        return true;
    }

    return false;
}