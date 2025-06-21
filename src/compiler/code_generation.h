#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "../parser/ast.h"

#ifdef __cplusplus
extern "C" {
#endif

int generate_c_code(FILE *output, ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // CODE_GENERATION_H
