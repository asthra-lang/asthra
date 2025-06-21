#ifndef EXPRESSION_OPERATIONS_H
#define EXPRESSION_OPERATIONS_H

#include "code_generator.h"
#include "../parser/ast_types.h"
#include <stdbool.h>

// Binary and unary operation generation
bool generate_binary_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_assignment_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_identifier_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg);

// Helper functions
char *create_mangled_function_name(const char *struct_name, const char *function_name, bool is_instance_method);

#endif // EXPRESSION_OPERATIONS_H