/**
 * Asthra Programming Language Compiler
 * FFI Assembly Const Declarations - Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3: Code Generation Implementation
 * Generates C code for const declarations with proper visibility and types
 */

#include "ffi_assembly_const.h"
#include "ffi_assembly_internal.h"
#include "ffi_generator_core.h"
#include "global_symbols.h"
#include "../analysis/const_evaluator.h"
#include "../analysis/semantic_const_declarations.h"
#include "../parser/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// CONST DECLARATION CODE GENERATION
// =============================================================================

/**
 * Generate C code for a const declaration
 * Maps Asthra const declarations to appropriate C equivalents
 */
bool ffi_generate_const_decl(FFIAssemblyGenerator* generator, ASTNode* const_decl_node) {
    if (!generator || !const_decl_node) {
        return false;
    }
    
    if (const_decl_node->type != AST_CONST_DECL) {
        return false;
    }

    // Extract const declaration components
    const char* const_name = const_decl_node->data.const_decl.name;
    ASTNode* type_node = const_decl_node->data.const_decl.type;
    ASTNode* value_node = const_decl_node->data.const_decl.value;
    bool is_public = (const_decl_node->data.const_decl.visibility == VISIBILITY_PUBLIC);

    if (!const_name || !type_node || !value_node) {
        return false;
    }

    // Determine C type string from Asthra type
    char* c_type = ffi_get_c_type_from_ast(type_node);
    if (!c_type) {
        return false;
    }

    // Generate C constant value string
    char* c_value = ffi_generate_const_value(generator, value_node);
    if (!c_value) {
        free(c_type);
        return false;
    }

    // Generate appropriate C code based on type and visibility
    bool success = false;
    
    // For simple integer/float constants, prefer #define for performance
    if (ffi_is_simple_numeric_const(type_node, value_node)) {
        success = ffi_generate_const_define(generator, const_name, c_value, is_public);
    } else {
        // For complex types (strings, computed values), use static const
        success = ffi_generate_const_declaration(generator, const_name, c_type, c_value, is_public);
    }

    // Cleanup
    free(c_type);
    free(c_value);

    return success;
}

/**
 * Generate #define macro for simple numeric constants
 */
bool ffi_generate_const_define(FFIAssemblyGenerator* generator, const char* const_name, 
                              const char* const_value, bool is_public) {
    if (!generator || !const_name || !const_value) {
        return false;
    }

    // For now, just emit a comment since we don't have FILE* output in the generator
    // In a real implementation, this would write to appropriate output streams
    emit_comment(generator, "Const #define would be generated here");
    
    char comment_buffer[512];
    snprintf(comment_buffer, sizeof(comment_buffer), 
             "%s #define %s %s", 
             is_public ? "Public" : "Private", const_name, const_value);
    emit_comment(generator, comment_buffer);

    // Add to symbol table for tracking
    return ffi_add_global_const_symbol(generator, const_name, "define", is_public);
}

/**
 * Generate static const declaration for complex constants
 */
bool ffi_generate_const_declaration(FFIAssemblyGenerator* generator, const char* const_name,
                                   const char* c_type, const char* const_value, bool is_public) {
    if (!generator || !const_name || !c_type || !const_value) {
        return false;
    }

    // For now, just emit a comment since we don't have FILE* output in the generator
    // In a real implementation, this would write to appropriate output streams
    emit_comment(generator, "Const declaration would be generated here");
    
    char comment_buffer[512];
    const char* visibility = is_public ? "extern" : "static";
    snprintf(comment_buffer, sizeof(comment_buffer), 
             "%s const %s %s = %s;", 
             visibility, c_type, const_name, const_value);
    emit_comment(generator, comment_buffer);

    // Add to symbol table for tracking
    return ffi_add_global_const_symbol(generator, const_name, c_type, is_public);
}

// =============================================================================
// CONST VALUE GENERATION
// =============================================================================

/**
 * Generate C representation of const value from AST node
 */
char* ffi_generate_const_value(FFIAssemblyGenerator* generator, ASTNode* value_node) {
    if (!generator || !value_node) {
        return NULL;
    }

    switch (value_node->type) {
        case AST_INTEGER_LITERAL:
            return ffi_generate_integer_const(value_node->data.integer_literal.value);
            
        case AST_FLOAT_LITERAL:
            return ffi_generate_float_const(value_node->data.float_literal.value);
            
        case AST_STRING_LITERAL:
            return ffi_generate_string_const(value_node->data.string_literal.value);
            
        case AST_BOOL_LITERAL:
        case AST_BOOLEAN_LITERAL:
            return ffi_generate_boolean_const(value_node->data.bool_literal.value);
            
        case AST_CONST_EXPR:
            return ffi_generate_const_expression(generator, value_node);
            
        default:
            return NULL;
    }
}

/**
 * Generate integer constant value
 */
char* ffi_generate_integer_const(int64_t value) {
    char* result = malloc(32); // Sufficient for 64-bit integer
    if (!result) {
        return NULL;
    }
    
    if (snprintf(result, 32, "%lld", (long long)value) < 0) {
        free(result);
        return NULL;
    }
    
    return result;
}

/**
 * Generate float constant value
 */
char* ffi_generate_float_const(double value) {
    char* result = malloc(64); // Sufficient for double precision
    if (!result) {
        return NULL;
    }
    
    if (snprintf(result, 64, "%.17g", value) < 0) {
        free(result);
        return NULL;
    }
    
    return result;
}

/**
 * Generate string constant value with proper C escaping
 */
char* ffi_generate_string_const(const char* value) {
    if (!value) {
        return NULL;
    }
    
    size_t len = strlen(value);
    char* result = malloc(len * 2 + 3); // Worst case: every char escaped + quotes + null
    if (!result) {
        return NULL;
    }
    
    char* dst = result;
    *dst++ = '"';
    
    for (const char* src = value; *src; src++) {
        switch (*src) {
            case '"':  *dst++ = '\\'; *dst++ = '"'; break;
            case '\\': *dst++ = '\\'; *dst++ = '\\'; break;
            case '\n': *dst++ = '\\'; *dst++ = 'n'; break;
            case '\r': *dst++ = '\\'; *dst++ = 'r'; break;
            case '\t': *dst++ = '\\'; *dst++ = 't'; break;
            default:   *dst++ = *src; break;
        }
    }
    
    *dst++ = '"';
    *dst = '\0';
    
    return result;
}

/**
 * Generate boolean constant value
 */
char* ffi_generate_boolean_const(bool value) {
    char* result = malloc(6); // "true" or "false" + null
    if (!result) {
        return NULL;
    }
    
    strcpy(result, value ? "true" : "false");
    return result;
}

/**
 * Generate const expression (arithmetic, references, etc.)
 */
char* ffi_generate_const_expression(FFIAssemblyGenerator* generator, ASTNode* expr_node) {
    if (!generator || !expr_node || expr_node->type != AST_CONST_EXPR) {
        return NULL;
    }

    switch (expr_node->data.const_expr.expr_type) {
        case CONST_EXPR_LITERAL:
            return ffi_generate_const_value(generator, expr_node->data.const_expr.data.literal);
            
        case CONST_EXPR_IDENTIFIER:
            return ffi_generate_const_identifier_ref(expr_node->data.const_expr.data.identifier);
            
        case CONST_EXPR_BINARY_OP:
            return ffi_generate_const_binary_expr(generator, 
                expr_node->data.const_expr.data.binary.left,
                expr_node->data.const_expr.data.binary.op,
                expr_node->data.const_expr.data.binary.right);
            
        case CONST_EXPR_UNARY_OP:
            return ffi_generate_const_unary_expr(generator,
                expr_node->data.const_expr.data.unary.op,
                expr_node->data.const_expr.data.unary.operand);
            
        case CONST_EXPR_SIZEOF:
            return ffi_generate_const_sizeof_expr(generator, expr_node->data.const_expr.data.sizeof_expr.type);
            
        default:
            return NULL;
    }
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Determine if a const declaration should use #define vs static const
 */
bool ffi_is_simple_numeric_const(ASTNode* type_node, ASTNode* value_node) {
    if (!type_node || !value_node) {
        return false;
    }

    // Check if type is a simple numeric type
    if (type_node->type != AST_BASE_TYPE) {
        return false;
    }

    const char* type_name = type_node->data.base_type.name;
    if (!type_name) {
        return false;
    }

    // Check for numeric types
    if (strcmp(type_name, "i32") == 0 || strcmp(type_name, "i64") == 0 ||
        strcmp(type_name, "u32") == 0 || strcmp(type_name, "u64") == 0 ||
        strcmp(type_name, "f32") == 0 || strcmp(type_name, "f64") == 0) {
        
        // Check if value is a simple literal (not computed)
        return (value_node->type == AST_INTEGER_LITERAL || 
                value_node->type == AST_FLOAT_LITERAL);
    }

    return false;
}

/**
 * Get C type string from Asthra type AST node
 */
char* ffi_get_c_type_from_ast(ASTNode* type_node) {
    if (!type_node || type_node->type != AST_BASE_TYPE) {
        return NULL;
    }

    const char* type_name = type_node->data.base_type.name;
    if (!type_name) {
        return NULL;
    }

    char* result = malloc(32); // Sufficient for basic type names
    if (!result) {
        return NULL;
    }

    // Map Asthra types to C types
    if (strcmp(type_name, "i32") == 0) {
        strcpy(result, "int32_t");
    } else if (strcmp(type_name, "i64") == 0) {
        strcpy(result, "int64_t");
    } else if (strcmp(type_name, "u32") == 0) {
        strcpy(result, "uint32_t");
    } else if (strcmp(type_name, "u64") == 0) {
        strcpy(result, "uint64_t");
    } else if (strcmp(type_name, "f32") == 0) {
        strcpy(result, "float");
    } else if (strcmp(type_name, "f64") == 0) {
        strcpy(result, "double");
    } else if (strcmp(type_name, "bool") == 0) {
        strcpy(result, "bool");
    } else if (strcmp(type_name, "string") == 0) {
        strcpy(result, "const char*");
    } else if (strcmp(type_name, "usize") == 0) {
        strcpy(result, "size_t");
    } else {
        // Unknown type
        free(result);
        return NULL;
    }

    return result;
}

/**
 * Generate const identifier reference
 */
char* ffi_generate_const_identifier_ref(const char* identifier) {
    if (!identifier) {
        return NULL;
    }
    
    char* result = malloc(strlen(identifier) + 1);
    if (!result) {
        return NULL;
    }
    
    strcpy(result, identifier);
    return result;
}

/**
 * Generate const binary expression
 */
char* ffi_generate_const_binary_expr(FFIAssemblyGenerator* generator,
                                     ASTNode* left, BinaryOperator op, ASTNode* right) {
    if (!generator || !left || !right) {
        return NULL;
    }

    char* left_str = ffi_generate_const_value(generator, left);
    char* right_str = ffi_generate_const_value(generator, right);
    const char* op_str = ffi_get_c_binary_operator(op);

    if (!left_str || !right_str || !op_str) {
        free(left_str);
        free(right_str);
        return NULL;
    }

    size_t result_len = strlen(left_str) + strlen(right_str) + strlen(op_str) + 6; // "() op ()"
    char* result = malloc(result_len);
    if (!result) {
        free(left_str);
        free(right_str);
        return NULL;
    }

    snprintf(result, result_len, "(%s %s %s)", left_str, op_str, right_str);

    free(left_str);
    free(right_str);
    return result;
}

/**
 * Generate const unary expression
 */
char* ffi_generate_const_unary_expr(FFIAssemblyGenerator* generator,
                                    UnaryOperator op, ASTNode* operand) {
    if (!generator || !operand) {
        return NULL;
    }

    char* operand_str = ffi_generate_const_value(generator, operand);
    const char* op_str = ffi_get_c_unary_operator(op);

    if (!operand_str || !op_str) {
        free(operand_str);
        return NULL;
    }

    size_t result_len = strlen(operand_str) + strlen(op_str) + 4; // "op()"
    char* result = malloc(result_len);
    if (!result) {
        free(operand_str);
        return NULL;
    }

    snprintf(result, result_len, "%s(%s)", op_str, operand_str);

    free(operand_str);
    return result;
}

/**
 * Generate const sizeof expression
 */
char* ffi_generate_const_sizeof_expr(FFIAssemblyGenerator* generator, ASTNode* type_node) {
    if (!generator || !type_node) {
        return NULL;
    }

    char* type_str = ffi_get_c_type_from_ast(type_node);
    if (!type_str) {
        return NULL;
    }

    size_t result_len = strlen(type_str) + 10; // "sizeof()"
    char* result = malloc(result_len);
    if (!result) {
        free(type_str);
        return NULL;
    }

    snprintf(result, result_len, "sizeof(%s)", type_str);

    free(type_str);
    return result;
}

/**
 * Get C binary operator string
 */
const char* ffi_get_c_binary_operator(BinaryOperator op) {
    switch (op) {
        case BINOP_ADD: return "+";
        case BINOP_SUB: return "-";
        case BINOP_MUL: return "*";
        case BINOP_DIV: return "/";
        case BINOP_MOD: return "%";
        case BINOP_EQ:  return "==";
        case BINOP_NE:  return "!=";
        case BINOP_LT:  return "<";
        case BINOP_LE:  return "<=";
        case BINOP_GT:  return ">";
        case BINOP_GE:  return ">=";
        case BINOP_AND: return "&&";
        case BINOP_OR:  return "||";
        case BINOP_BITWISE_AND: return "&";
        case BINOP_BITWISE_OR:  return "|";
        case BINOP_BITWISE_XOR: return "^";
        case BINOP_LSHIFT: return "<<";
        case BINOP_RSHIFT: return ">>";
        default: return NULL;
    }
}

/**
 * Get C unary operator string
 */
const char* ffi_get_c_unary_operator(UnaryOperator op) {
    switch (op) {
        case UNOP_MINUS: return "-";
        case UNOP_NOT: return "!";
        case UNOP_BITWISE_NOT: return "~";
        case UNOP_SIZEOF: return "sizeof";
        default: return NULL;
    }
}

// =============================================================================
// SYMBOL MANAGEMENT
// =============================================================================

/**
 * Add const symbol to the global symbol table (delegate to global_symbols.c)
 */
bool ffi_add_const_symbol(FFIAssemblyGenerator* generator, const char* symbol_name, 
                         const char* symbol_type, bool is_public) {
    // Delegate to the global symbols implementation
    return ffi_add_global_const_symbol(generator, symbol_name, symbol_type, is_public);
} 
