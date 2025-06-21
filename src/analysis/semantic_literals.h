#ifndef SEMANTIC_LITERALS_H
#define SEMANTIC_LITERALS_H

#include "semantic_analyzer_core.h"
#include "../parser/ast_types.h"
#include "type_info.h"

// Literal expression analysis functions
bool analyze_identifier_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_literal_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool analyze_char_literal(SemanticAnalyzer *analyzer, ASTNode *literal);
bool analyze_string_literal(SemanticAnalyzer *analyzer, ASTNode *literal);

// Helper functions for literal analysis
bool is_valid_character_value(uint32_t value);
bool has_explicit_type_context(SemanticAnalyzer *analyzer, ASTNode *literal);
TypeInfo* get_builtin_char_type(SemanticAnalyzer *analyzer);
TypeInfo* get_builtin_string_type(SemanticAnalyzer *analyzer);

// String literal helpers
bool is_multiline_string_literal(const char *value);
char* normalize_multiline_indentation(const char* content);
size_t detect_common_indentation(const char* content);
char* strip_common_indentation(const char* content, size_t indent_level);

#endif // SEMANTIC_LITERALS_H