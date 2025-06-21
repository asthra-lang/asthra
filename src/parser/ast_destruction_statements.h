#ifndef AST_DESTRUCTION_STATEMENTS_H
#define AST_DESTRUCTION_STATEMENTS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

void ast_free_statement_nodes(ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // AST_DESTRUCTION_STATEMENTS_H
