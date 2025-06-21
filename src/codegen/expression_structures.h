#ifndef EXPRESSION_STRUCTURES_H
#define EXPRESSION_STRUCTURES_H

#include "code_generator.h"
#include "../parser/ast_types.h"
#include <stdbool.h>

// Structure and array expression generation
bool generate_struct_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_field_access(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_array_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_tuple_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_index_access(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_slice_expr(CodeGenerator *generator, ASTNode *expr, Register target_reg);

#endif // EXPRESSION_STRUCTURES_H