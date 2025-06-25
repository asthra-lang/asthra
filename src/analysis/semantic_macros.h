/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - C17 Generic Macros
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Type-safe semantic operations using C17 _Generic
 */

#ifndef ASTHRA_SEMANTIC_MACROS_H
#define ASTHRA_SEMANTIC_MACROS_H

#include "../parser/ast.h"
#include "semantic_symbols_defs.h"
#include "semantic_types_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration for SemanticAnalyzer
typedef struct SemanticAnalyzer SemanticAnalyzer;

// =============================================================================
// C17 GENERIC TYPE-SAFE SEMANTIC OPERATIONS
// =============================================================================

// Type-safe semantic analysis dispatch using C17 _Generic
#define semantic_analyze_node(analyzer, node)                                                      \
    _Generic((node),                                                                               \
        ASTNode *: semantic_analyze_node_impl,                                                     \
        const ASTNode *: semantic_analyze_node_const_impl)(analyzer, node)

#define semantic_check_type(analyzer, node, expected_type)                                         \
    _Generic((node),                                                                               \
        ASTNode *: semantic_check_type_impl,                                                       \
        const ASTNode *: semantic_check_type_const_impl)(analyzer, node, expected_type)

#define semantic_resolve_symbol(analyzer, name)                                                    \
    _Generic((name),                                                                               \
        char *: semantic_resolve_symbol_impl,                                                      \
        const char *: semantic_resolve_symbol_impl)(analyzer, name)

// Type descriptor operations with _Generic
#define type_descriptor_create(category, ...)                                                      \
    _Generic((category), TypeCategory: type_descriptor_create_impl)(category, __VA_ARGS__)

#define type_descriptor_compare(type1, type2)                                                      \
    _Generic((type1),                                                                              \
        TypeDescriptor *: type_descriptor_compare_impl,                                            \
        const TypeDescriptor *: type_descriptor_compare_const_impl)(type1, type2)

// Symbol table operations with thread safety
#define symbol_table_insert(table, name, symbol)                                                   \
    _Generic((table), SymbolTable *: symbol_table_insert_impl)(table, name, symbol)

#define symbol_table_lookup(table, name)                                                           \
    _Generic((table),                                                                              \
        SymbolTable *: symbol_table_lookup_impl,                                                   \
        const SymbolTable *: symbol_table_lookup_const_impl)(table, name)

// =============================================================================
// IMPLEMENTATION FUNCTION DECLARATIONS
// =============================================================================

// Implementation functions (not for direct use)
bool semantic_analyze_node_impl(SemanticAnalyzer *analyzer, ASTNode *node);
bool semantic_analyze_node_const_impl(SemanticAnalyzer *analyzer, const ASTNode *node);

bool semantic_check_type_impl(SemanticAnalyzer *analyzer, ASTNode *node, TypeDescriptor *expected);
bool semantic_check_type_const_impl(SemanticAnalyzer *analyzer, const ASTNode *node,
                                    TypeDescriptor *expected);

SymbolEntry *semantic_resolve_symbol_impl(SemanticAnalyzer *analyzer, const char *name);

TypeDescriptor *type_descriptor_create_impl(TypeCategory category, ...);
bool type_descriptor_compare_impl(TypeDescriptor *type1, TypeDescriptor *type2);
bool type_descriptor_compare_const_impl(const TypeDescriptor *type1, const TypeDescriptor *type2);

bool symbol_table_insert_impl(SymbolTable *table, const char *name, SymbolEntry *entry);
SymbolEntry *symbol_table_lookup_impl(SymbolTable *table, const char *name);
SymbolEntry *symbol_table_lookup_const_impl(const SymbolTable *table, const char *name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_MACROS_H
