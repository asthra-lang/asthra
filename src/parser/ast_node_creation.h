#ifndef AST_NODE_CREATION_H
#define AST_NODE_CREATION_H

#include "ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
ASTNode *ast_clone_node(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // AST_NODE_CREATION_H
