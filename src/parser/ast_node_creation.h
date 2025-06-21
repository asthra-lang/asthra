#ifndef AST_NODE_CREATION_H
#define AST_NODE_CREATION_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
ASTNode *ast_clone_node(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // AST_NODE_CREATION_H
