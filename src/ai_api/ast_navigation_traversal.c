#include "ast_navigation_internal.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// AST TRAVERSAL AND ANALYSIS
// =============================================================================

// Helper function to find parent by traversing AST
static ASTNode *find_parent_by_traversal(ASTNode *root, ASTNode *target, ASTNode *current_parent) {
    if (!root || root == target) {
        return current_parent;
    }

    // Check all possible child nodes based on type
    switch (root->type) {
    case AST_PROGRAM:
        if (root->data.program.package_decl == target)
            return root;
        if (root->data.program.imports) {
            for (size_t i = 0; i < ast_node_list_size(root->data.program.imports); i++) {
                ASTNode *child = ast_node_list_get(root->data.program.imports, i);
                if (child == target)
                    return root;
                ASTNode *result = find_parent_by_traversal(child, target, root);
                if (result)
                    return result;
            }
        }
        if (root->data.program.declarations) {
            for (size_t i = 0; i < ast_node_list_size(root->data.program.declarations); i++) {
                ASTNode *child = ast_node_list_get(root->data.program.declarations, i);
                if (child == target)
                    return root;
                ASTNode *result = find_parent_by_traversal(child, target, root);
                if (result)
                    return result;
            }
        }
        break;

    case AST_FUNCTION_DECL:
        if (root->data.function_decl.return_type == target)
            return root;
        if (root->data.function_decl.body == target)
            return root;
        if (root->data.function_decl.params) {
            for (size_t i = 0; i < ast_node_list_size(root->data.function_decl.params); i++) {
                ASTNode *child = ast_node_list_get(root->data.function_decl.params, i);
                if (child == target)
                    return root;
                ASTNode *result = find_parent_by_traversal(child, target, root);
                if (result)
                    return result;
            }
        }
        if (root->data.function_decl.body) {
            ASTNode *result = find_parent_by_traversal(root->data.function_decl.body, target, root);
            if (result)
                return result;
        }
        break;

    case AST_BLOCK:
        if (root->data.block.statements) {
            for (size_t i = 0; i < ast_node_list_size(root->data.block.statements); i++) {
                ASTNode *child = ast_node_list_get(root->data.block.statements, i);
                if (child == target)
                    return root;
                ASTNode *result = find_parent_by_traversal(child, target, root);
                if (result)
                    return result;
            }
        }
        break;

    case AST_BINARY_EXPR:
        if (root->data.binary_expr.left == target)
            return root;
        if (root->data.binary_expr.right == target)
            return root;
        if (root->data.binary_expr.left) {
            ASTNode *result = find_parent_by_traversal(root->data.binary_expr.left, target, root);
            if (result)
                return result;
        }
        if (root->data.binary_expr.right) {
            ASTNode *result = find_parent_by_traversal(root->data.binary_expr.right, target, root);
            if (result)
                return result;
        }
        break;

    case AST_CALL_EXPR:
        if (root->data.call_expr.function == target)
            return root;
        if (root->data.call_expr.args) {
            for (size_t i = 0; i < ast_node_list_size(root->data.call_expr.args); i++) {
                ASTNode *child = ast_node_list_get(root->data.call_expr.args, i);
                if (child == target)
                    return root;
                ASTNode *result = find_parent_by_traversal(child, target, root);
                if (result)
                    return result;
            }
        }
        break;

    // Add more cases as needed for complete coverage
    default:
        break;
    }

    return NULL;
}

ASTNode *asthra_ai_get_parent_node(AsthraSemanticsAPI *api, ASTNode *node) {
    if (!is_api_valid(api) || !node) {
        return NULL;
    }

    pthread_mutex_lock(&api->api_mutex);

    // TODO: Need access to root AST for parent finding
    // For now, return NULL - parent finding requires AST traversal
    ASTNode *parent = NULL;
    (void)node; // Suppress unused warning

    pthread_mutex_unlock(&api->api_mutex);
    return parent;
}

ASTNode **asthra_ai_get_child_nodes(AsthraSemanticsAPI *api, ASTNode *node, size_t *count) {
    if (!is_api_valid(api) || !node || !count) {
        if (count)
            *count = 0;
        return NULL;
    }

    *count = 0;

    // Extract children based on node type
    // This is a simplified implementation
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.declarations) {
            size_t decl_count = ast_node_list_size(node->data.program.declarations);
            if (decl_count > 0) {
                ASTNode **children = malloc(sizeof(ASTNode *) * decl_count);
                if (children) {
                    for (size_t i = 0; i < decl_count; i++) {
                        children[i] = ast_node_list_get(node->data.program.declarations, i);
                    }
                    *count = decl_count;
                    return children;
                }
            }
        }
        break;

    case AST_FUNCTION_DECL:
        if (node->data.function_decl.body) {
            ASTNode **children = malloc(sizeof(ASTNode *) * 1);
            if (children) {
                children[0] = node->data.function_decl.body;
                *count = 1;
                return children;
            }
        }
        break;

    case AST_BLOCK:
        if (node->data.block.statements) {
            size_t stmt_count = ast_node_list_size(node->data.block.statements);
            if (stmt_count > 0) {
                ASTNode **children = malloc(sizeof(ASTNode *) * stmt_count);
                if (children) {
                    for (size_t i = 0; i < stmt_count; i++) {
                        children[i] = ast_node_list_get(node->data.block.statements, i);
                    }
                    *count = stmt_count;
                    return children;
                }
            }
        }
        break;

    default:
        break;
    }

    return NULL;
}

ASTNode **asthra_ai_find_nodes_by_type(AsthraSemanticsAPI *api, ASTNodeType node_type,
                                       size_t *count) {
    if (!is_api_valid(api) || !count) {
        if (count)
            *count = 0;
        return NULL;
    }

    *count = 0;

    // This would require a full AST traversal
    // For now, return empty result
    return NULL;
}

char *asthra_ai_get_node_scope(AsthraSemanticsAPI *api, ASTNode *node) {
    if (!is_api_valid(api) || !node) {
        return NULL;
    }

    // For now, return a simplified scope name
    return strdup("global");
}