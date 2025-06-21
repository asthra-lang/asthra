#ifndef EXPRESSION_CALLS_H
#define EXPRESSION_CALLS_H

#include "code_generator.h"
#include "../parser/ast_types.h"
#include <stdbool.h>

// Function call generation functions
bool code_generate_associated_function_call(CodeGenerator *generator, ASTNode *call_expr, Register target_reg);
bool code_generate_function_call(CodeGenerator *generator, ASTNode *call_expr, Register target_reg);
bool code_generate_enum_variant_construction(CodeGenerator *generator, ASTNode *expr, Register target_reg);

#endif // EXPRESSION_CALLS_H