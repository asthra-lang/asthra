#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include "../parser/ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int generate_c_code(FILE *output, ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // CODE_GENERATION_H
