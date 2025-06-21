/**
 * Asthra Programming Language Compiler
 * FFI Assembly Const Declarations - Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3: Code Generation Implementation
 * Interface for generating C code from Asthra const declarations
 */

#ifndef ASTHRA_FFI_ASSEMBLY_CONST_H
#define ASTHRA_FFI_ASSEMBLY_CONST_H

#include "ffi_generator_core.h"
#include "../parser/ast.h"
#include "../parser/ast_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONST DECLARATION CODE GENERATION
// =============================================================================

/**
 * Generate C code for a const declaration
 * Maps Asthra const declarations to appropriate C equivalents
 * @param generator The FFI assembly generator instance
 * @param const_decl_node The AST node for the const declaration
 * @return true if generation successful, false otherwise
 */
bool ffi_generate_const_decl(FFIAssemblyGenerator* generator, ASTNode* const_decl_node);

/**
 * Generate #define macro for simple numeric constants
 * @param generator The FFI assembly generator instance
 * @param const_name The name of the constant
 * @param const_value The C representation of the constant value
 * @param is_public Whether the constant should be publicly visible
 * @return true if generation successful, false otherwise
 */
bool ffi_generate_const_define(FFIAssemblyGenerator* generator, const char* const_name, 
                              const char* const_value, bool is_public);

/**
 * Generate static const declaration for complex constants
 * @param generator The FFI assembly generator instance
 * @param const_name The name of the constant
 * @param c_type The C type string for the constant
 * @param const_value The C representation of the constant value
 * @param is_public Whether the constant should be publicly visible
 * @return true if generation successful, false otherwise
 */
bool ffi_generate_const_declaration(FFIAssemblyGenerator* generator, const char* const_name,
                                   const char* c_type, const char* const_value, bool is_public);

// =============================================================================
// CONST VALUE GENERATION
// =============================================================================

/**
 * Generate C representation of const value from AST node
 * @param generator The FFI assembly generator instance
 * @param value_node The AST node containing the const value
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_const_value(FFIAssemblyGenerator* generator, ASTNode* value_node);

/**
 * Generate integer constant value
 * @param value The integer value
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_integer_const(int64_t value);

/**
 * Generate float constant value
 * @param value The float value
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_float_const(double value);

/**
 * Generate string constant value with proper C escaping
 * @param value The string value
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_string_const(const char* value);

/**
 * Generate boolean constant value
 * @param value The boolean value
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_boolean_const(bool value);

/**
 * Generate const expression (arithmetic, references, etc.)
 * @param generator The FFI assembly generator instance
 * @param expr_node The AST node for the const expression
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_const_expression(FFIAssemblyGenerator* generator, ASTNode* expr_node);

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Determine if a const declaration should use #define vs static const
 * @param type_node The AST node for the const type
 * @param value_node The AST node for the const value
 * @return true if should use #define, false if should use static const
 */
bool ffi_is_simple_numeric_const(ASTNode* type_node, ASTNode* value_node);

/**
 * Get C type string from Asthra type AST node
 * @param type_node The AST node for the type
 * @return Allocated string with C type, caller must free
 */
char* ffi_get_c_type_from_ast(ASTNode* type_node);

/**
 * Generate const identifier reference
 * @param identifier The identifier name
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_const_identifier_ref(const char* identifier);

/**
 * Generate const binary expression
 * @param generator The FFI assembly generator instance
 * @param left Left operand AST node
 * @param op Binary operator
 * @param right Right operand AST node
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_const_binary_expr(FFIAssemblyGenerator* generator, 
                                     ASTNode* left, BinaryOperator op, ASTNode* right);

/**
 * Generate const unary expression
 * @param generator The FFI assembly generator instance
 * @param op Unary operator
 * @param operand Operand AST node
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_const_unary_expr(FFIAssemblyGenerator* generator,
                                    UnaryOperator op, ASTNode* operand);

/**
 * Generate const sizeof expression
 * @param generator The FFI assembly generator instance
 * @param type_node The AST node for the type in sizeof
 * @return Allocated string with C representation, caller must free
 */
char* ffi_generate_const_sizeof_expr(FFIAssemblyGenerator* generator, ASTNode* type_node);

/**
 * Get C binary operator string
 * @param op The binary operator
 * @return C operator string, or NULL if unsupported
 */
const char* ffi_get_c_binary_operator(BinaryOperator op);

/**
 * Get C unary operator string
 * @param op The unary operator
 * @return C operator string, or NULL if unsupported
 */
const char* ffi_get_c_unary_operator(UnaryOperator op);

// =============================================================================
// SYMBOL MANAGEMENT
// =============================================================================

/**
 * Add const symbol to the global symbol table
 * @param generator The FFI assembly generator instance
 * @param symbol_name The name of the constant symbol
 * @param symbol_type The type of the constant symbol
 * @param is_public Whether the symbol is publicly visible
 * @return true if symbol added successfully, false otherwise
 */
bool ffi_add_const_symbol(FFIAssemblyGenerator* generator, const char* symbol_name, 
                         const char* symbol_type, bool is_public);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_ASSEMBLY_CONST_H 
