#include "../parser/ast.h"
#include "type_info.h"

// =============================================================================
// AST INTEGRATION
// =============================================================================

void ast_node_set_type_info(ASTNode *node, TypeInfo *type_info) {
    if (!node)
        return;

    if (node->type_info) {
        type_info_release(node->type_info);
    }

    node->type_info = type_info;
    if (type_info) {
        type_info_retain(type_info);
    }
}

TypeInfo *ast_node_get_type_info(const ASTNode *node) {
    return node ? node->type_info : NULL;
}

bool type_info_propagate_ast(ASTNode *root) {
    if (!root)
        return true;

    // This is a placeholder for AST type propagation
    // In a full implementation, this would traverse the AST
    // and propagate type information from parent to child nodes

    // For now, just return true to indicate success
    return true;
}
