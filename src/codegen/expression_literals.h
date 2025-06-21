#ifndef EXPRESSION_LITERALS_H
#define EXPRESSION_LITERALS_H

#include "code_generator.h"
#include "../parser/ast_types.h"
#include <stdbool.h>

// Literal expression generation functions
bool generate_integer_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_char_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_string_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_float_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_bool_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);
bool generate_unit_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg);

// Multi-line string utilities
char *escape_string_for_c_generation(const char *input);
bool is_multiline_string_content(const char *str_value);
char *create_string_comment(const char *str_value);

#endif // EXPRESSION_LITERALS_H