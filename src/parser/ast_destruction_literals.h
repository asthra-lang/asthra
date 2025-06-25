#ifndef AST_DESTRUCTION_LITERALS_H
#define AST_DESTRUCTION_LITERALS_H

#include "ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void ast_free_literal_nodes(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // AST_DESTRUCTION_LITERALS_H
