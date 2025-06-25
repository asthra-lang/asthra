/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node List Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains the implementation of AST node list operations
 * Split from the original ast.c for better maintainability
 */

#include "ast.h"
#include "ast_node_cloning.h"
#include "ast_node_creation.h"
#include "ast_node_refcount.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// AST NODE LIST IMPLEMENTATION
// =============================================================================

ASTNodeList *ast_node_list_create(size_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 4; // Default minimum capacity
    }

    ASTNodeList *list = malloc(sizeof(ASTNodeList) + initial_capacity * sizeof(ASTNode *));
    if (!list) {
        return NULL;
    }

    list->count = 0;
    list->capacity = initial_capacity;
    return list;
}

void ast_node_list_destroy(ASTNodeList *list) {
    if (!list)
        return;

    // Free all nodes in the list
    for (size_t i = 0; i < list->count; i++) {
        if (list->nodes[i]) {
            ast_release_node(list->nodes[i]); // Use reference counting
        }
    }

    free(list);
}

bool ast_node_list_add(ASTNodeList **list, ASTNode *node) {
    if (!list || !node)
        return false;

    if (!*list) {
        *list = ast_node_list_create(4);
        if (!*list)
            return false;
    }

    // Resize if needed
    if ((*list)->count >= (*list)->capacity) {
        size_t new_capacity = (*list)->capacity * 2;
        ASTNodeList *new_list =
            realloc(*list, sizeof(ASTNodeList) + new_capacity * sizeof(ASTNode *));
        if (!new_list)
            return false;

        new_list->capacity = new_capacity;
        *list = new_list;
    }

    (*list)->nodes[(*list)->count] = ast_retain_node(node); // Reference counting
    (*list)->count++;
    return true;
}

bool ast_node_list_insert(ASTNodeList **list, size_t index, ASTNode *node) {
    if (!list || !node)
        return false;

    if (!*list) {
        *list = ast_node_list_create(4);
        if (!*list)
            return false;
    }

    if (index > (*list)->count)
        return false;

    // Resize if needed
    if ((*list)->count >= (*list)->capacity) {
        size_t new_capacity = (*list)->capacity * 2;
        ASTNodeList *new_list =
            realloc(*list, sizeof(ASTNodeList) + new_capacity * sizeof(ASTNode *));
        if (!new_list)
            return false;

        new_list->capacity = new_capacity;
        *list = new_list;
    }

    // Shift elements to make room
    for (size_t i = (*list)->count; i > index; i--) {
        (*list)->nodes[i] = (*list)->nodes[i - 1];
    }

    (*list)->nodes[index] = ast_retain_node(node);
    (*list)->count++;
    return true;
}

bool ast_node_list_remove(ASTNodeList **list, size_t index) {
    if (!list || !*list || index >= (*list)->count)
        return false;

    // Release the node being removed
    ast_release_node((*list)->nodes[index]);

    // Shift elements to fill the gap
    for (size_t i = index; i < (*list)->count - 1; i++) {
        (*list)->nodes[i] = (*list)->nodes[i + 1];
    }

    (*list)->count--;
    return true;
}

ASTNode *ast_node_list_get(const ASTNodeList *list, size_t index) {
    if (!list || index >= list->count)
        return NULL;
    return list->nodes[index];
}

size_t ast_node_list_size(const ASTNodeList *list) {
    return list ? list->count : 0;
}

void ast_node_list_clear(ASTNodeList *list) {
    if (!list)
        return;

    for (size_t i = 0; i < list->count; i++) {
        ast_release_node(list->nodes[i]);
    }
    list->count = 0;
}

ASTNodeList *ast_node_list_clone(const ASTNodeList *list) {
    if (!list)
        return NULL;

    ASTNodeList *new_list = ast_node_list_create(list->capacity);
    if (!new_list)
        return NULL;

    for (size_t i = 0; i < list->count; i++) {
        ast_node_list_add(&new_list, list->nodes[i]);
    }

    return new_list;
}

ASTNodeList *ast_node_list_clone_deep(const ASTNodeList *list) {
    if (!list)
        return NULL;

    ASTNodeList *new_list = ast_node_list_create(list->capacity);
    if (!new_list)
        return NULL;

    for (size_t i = 0; i < list->count; i++) {
        ASTNode *cloned_node = ast_clone_node(list->nodes[i]);
        if (!cloned_node) {
            // Clean up on failure
            ast_node_list_destroy(new_list);
            return NULL;
        }
        // ast_node_list_add will retain the node, but ast_clone_node already set ref_count to 1
        // So we need to add without retaining
        if (new_list->count >= new_list->capacity) {
            size_t new_capacity = new_list->capacity * 2;
            ASTNodeList *resized =
                realloc(new_list, sizeof(ASTNodeList) + new_capacity * sizeof(ASTNode *));
            if (!resized) {
                ast_release_node(cloned_node);
                ast_node_list_destroy(new_list);
                return NULL;
            }
            resized->capacity = new_capacity;
            new_list = resized;
        }
        new_list->nodes[new_list->count] = cloned_node;
        new_list->count++;
    }

    return new_list;
}
