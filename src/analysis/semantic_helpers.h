#ifndef SEMANTIC_HELPERS_H
#define SEMANTIC_HELPERS_H

#include "semantic_symbols.h"
#include "semantic_types.h"
#include "../parser/ast_types.h"

// Type promotion and helper functions
TypeDescriptor *semantic_promote_arithmetic_types(SemanticAnalyzer *analyzer, 
                                                 TypeDescriptor *left_type, 
                                                 TypeDescriptor *right_type);

bool semantic_is_bool_type(TypeDescriptor *type);

void semantic_set_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr, TypeDescriptor *type);

bool semantic_types_equal(TypeDescriptor *type1, TypeDescriptor *type2);

// Generic implementation functions
bool semantic_analyze_node_impl(SemanticAnalyzer *analyzer, ASTNode *node);

bool semantic_analyze_node_const_impl(SemanticAnalyzer *analyzer, const ASTNode *node);

// Function context utilities
SymbolEntry *semantic_get_current_function(SemanticAnalyzer *analyzer);

#endif // SEMANTIC_HELPERS_H