#ifndef SEMANTIC_PATTERNS_H
#define SEMANTIC_PATTERNS_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"

// Unsafe block and pattern validation functions
bool analyze_unsafe_block_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool semantic_validate_pattern_types(SemanticAnalyzer *analyzer, ASTNode *pattern, TypeDescriptor *expected);

#endif // SEMANTIC_PATTERNS_H