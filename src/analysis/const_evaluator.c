/**
 * Asthra Programming Language Compiler
 * Const Expression Evaluator - Compile-time Expression Evaluation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 2: Semantic Analysis Implementation
 * Implements compile-time evaluation of constant expressions
 */

#include "const_evaluator.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// =============================================================================
// CONST VALUE REPRESENTATION
// =============================================================================

ConstValue *const_value_create_integer(int64_t value) {
    ConstValue *const_val = malloc(sizeof(ConstValue));
    if (!const_val) return NULL;
    
    const_val->type = CONST_VALUE_INTEGER;
    const_val->data.integer_value = value;
    return const_val;
}

ConstValue *const_value_create_float(double value) {
    ConstValue *const_val = malloc(sizeof(ConstValue));
    if (!const_val) return NULL;
    
    const_val->type = CONST_VALUE_FLOAT;
    const_val->data.float_value = value;
    return const_val;
}

ConstValue *const_value_create_string(const char *value) {
    ConstValue *const_val = malloc(sizeof(ConstValue));
    if (!const_val) return NULL;
    
    const_val->type = CONST_VALUE_STRING;
    const_val->data.string_value = strdup(value);
    if (!const_val->data.string_value) {
        free(const_val);
        return NULL;
    }
    return const_val;
}

ConstValue *const_value_create_boolean(bool value) {
    ConstValue *const_val = malloc(sizeof(ConstValue));
    if (!const_val) return NULL;
    
    const_val->type = CONST_VALUE_BOOLEAN;
    const_val->data.boolean_value = value;
    return const_val;
}

void const_value_destroy(ConstValue *value) {
    if (!value) return;
    
    if (value->type == CONST_VALUE_STRING && value->data.string_value) {
        free(value->data.string_value);
    }
    free(value);
}

// =============================================================================
// CONST EXPRESSION EVALUATION
// =============================================================================

/**
 * Evaluate a literal const expression
 */
static ConstValue *evaluate_const_literal(SemanticAnalyzer *analyzer, ASTNode *literal) {
    if (!analyzer || !literal) return NULL;
    
    switch (literal->type) {
        case AST_INTEGER_LITERAL:
            return const_value_create_integer(literal->data.integer_literal.value);
            
        case AST_FLOAT_LITERAL:
            return const_value_create_float(literal->data.float_literal.value);
            
        case AST_STRING_LITERAL:
            return const_value_create_string(literal->data.string_literal.value);
            
        case AST_BOOL_LITERAL:
            return const_value_create_boolean(literal->data.bool_literal.value);
            
        case AST_CHAR_LITERAL:
            // Treat char literals as integers for const evaluation
            return const_value_create_integer((int64_t)literal->data.char_literal.value);
            
        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 literal->location,
                                 "Unsupported literal type in const expression");
            return NULL;
    }
}

/**
 * Evaluate a const identifier (reference to another constant)
 */
static ConstValue *evaluate_const_identifier(SemanticAnalyzer *analyzer, const char *identifier) {
    if (!analyzer || !identifier) return NULL;
    
    // Look up the identifier in the symbol table
    SymbolEntry *symbol = semantic_resolve_identifier(analyzer, identifier);
    if (!symbol) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_UNDEFINED_SYMBOL,
                             (SourceLocation){NULL, 0, 0, 0},
                             "Undefined constant identifier '%s'", identifier);
        return NULL;
    }
    
    // Ensure it's a const symbol
    if (symbol->kind != SYMBOL_CONST) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             (SourceLocation){NULL, 0, 0, 0},
                             "Identifier '%s' is not a constant", identifier);
        return NULL;
    }
    
    // Return the stored const value
    if (!symbol->const_value) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                             (SourceLocation){NULL, 0, 0, 0},
                             "Constant '%s' has no evaluated value", identifier);
        return NULL;
    }
    
    // Create a copy of the const value
    switch (symbol->const_value->type) {
        case CONST_VALUE_INTEGER:
            return const_value_create_integer(symbol->const_value->data.integer_value);
        case CONST_VALUE_FLOAT:
            return const_value_create_float(symbol->const_value->data.float_value);
        case CONST_VALUE_STRING:
            return const_value_create_string(symbol->const_value->data.string_value);
        case CONST_VALUE_BOOLEAN:
            return const_value_create_boolean(symbol->const_value->data.boolean_value);
        default:
            return NULL;
    }
}

/**
 * Evaluate a binary const expression
 */
static ConstValue *evaluate_const_binary_op(SemanticAnalyzer *analyzer, 
                                           ConstValue *left, 
                                           BinaryOperator op, 
                                           ConstValue *right) {
    if (!analyzer || !left || !right) return NULL;
    
    // Integer operations
    if (left->type == CONST_VALUE_INTEGER && right->type == CONST_VALUE_INTEGER) {
        int64_t l = left->data.integer_value;
        int64_t r = right->data.integer_value;
        
        switch (op) {
            case BINOP_ADD:
                return const_value_create_integer(l + r);
            case BINOP_SUB:
                return const_value_create_integer(l - r);
            case BINOP_MUL:
                return const_value_create_integer(l * r);
            case BINOP_DIV:
                if (r == 0) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                         (SourceLocation){NULL, 0, 0, 0},
                                         "Division by zero in const expression");
                    return NULL;
                }
                return const_value_create_integer(l / r);
            case BINOP_MOD:
                if (r == 0) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                         (SourceLocation){NULL, 0, 0, 0},
                                         "Modulo by zero in const expression");
                    return NULL;
                }
                return const_value_create_integer(l % r);
            case BINOP_EQ:
                return const_value_create_boolean(l == r);
            case BINOP_NE:
                return const_value_create_boolean(l != r);
            case BINOP_LT:
                return const_value_create_boolean(l < r);
            case BINOP_LE:
                return const_value_create_boolean(l <= r);
            case BINOP_GT:
                return const_value_create_boolean(l > r);
            case BINOP_GE:
                return const_value_create_boolean(l >= r);
            case BINOP_BITWISE_AND:
                return const_value_create_integer(l & r);
            case BINOP_BITWISE_OR:
                return const_value_create_integer(l | r);
            case BINOP_BITWISE_XOR:
                return const_value_create_integer(l ^ r);
            case BINOP_LSHIFT:
                return const_value_create_integer(l << r);
            case BINOP_RSHIFT:
                return const_value_create_integer(l >> r);
            default:
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     (SourceLocation){NULL, 0, 0, 0},
                                     "Unsupported binary operator in const expression");
                return NULL;
        }
    }
    
    // Float operations
    if ((left->type == CONST_VALUE_FLOAT || left->type == CONST_VALUE_INTEGER) &&
        (right->type == CONST_VALUE_FLOAT || right->type == CONST_VALUE_INTEGER)) {
        
        double l = (left->type == CONST_VALUE_FLOAT) ? 
                   left->data.float_value : (double)left->data.integer_value;
        double r = (right->type == CONST_VALUE_FLOAT) ? 
                   right->data.float_value : (double)right->data.integer_value;
        
        switch (op) {
            case BINOP_ADD:
                return const_value_create_float(l + r);
            case BINOP_SUB:
                return const_value_create_float(l - r);
            case BINOP_MUL:
                return const_value_create_float(l * r);
            case BINOP_DIV:
                if (r == 0.0) {
                    semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                         (SourceLocation){NULL, 0, 0, 0},
                                         "Division by zero in const expression");
                    return NULL;
                }
                return const_value_create_float(l / r);
            case BINOP_EQ:
                return const_value_create_boolean(fabs(l - r) < 1e-10);
            case BINOP_NE:
                return const_value_create_boolean(fabs(l - r) >= 1e-10);
            case BINOP_LT:
                return const_value_create_boolean(l < r);
            case BINOP_LE:
                return const_value_create_boolean(l <= r);
            case BINOP_GT:
                return const_value_create_boolean(l > r);
            case BINOP_GE:
                return const_value_create_boolean(l >= r);
            default:
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     (SourceLocation){NULL, 0, 0, 0},
                                     "Unsupported binary operator for float values");
                return NULL;
        }
    }
    
    // Boolean operations
    if (left->type == CONST_VALUE_BOOLEAN && right->type == CONST_VALUE_BOOLEAN) {
        bool l = left->data.boolean_value;
        bool r = right->data.boolean_value;
        
        switch (op) {
            case BINOP_AND:
                return const_value_create_boolean(l && r);
            case BINOP_OR:
                return const_value_create_boolean(l || r);
            case BINOP_EQ:
                return const_value_create_boolean(l == r);
            case BINOP_NE:
                return const_value_create_boolean(l != r);
            default:
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     (SourceLocation){NULL, 0, 0, 0},
                                     "Unsupported binary operator for boolean values");
                return NULL;
        }
    }
    
    // String operations (limited)
    if (left->type == CONST_VALUE_STRING && right->type == CONST_VALUE_STRING) {
        const char *l = left->data.string_value;
        const char *r = right->data.string_value;
        
        switch (op) {
            case BINOP_EQ:
                return const_value_create_boolean(strcmp(l, r) == 0);
            case BINOP_NE:
                return const_value_create_boolean(strcmp(l, r) != 0);
            default:
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                     (SourceLocation){NULL, 0, 0, 0},
                                     "Unsupported binary operator for string values");
                return NULL;
        }
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                         (SourceLocation){NULL, 0, 0, 0},
                         "Type mismatch in binary const expression");
    return NULL;
}

/**
 * Evaluate a unary const expression
 */
static ConstValue *evaluate_const_unary_op(SemanticAnalyzer *analyzer, 
                                          UnaryOperator op, 
                                          ConstValue *operand) {
    if (!analyzer || !operand) return NULL;
    
    switch (op) {
        case UNOP_MINUS:
            if (operand->type == CONST_VALUE_INTEGER) {
                return const_value_create_integer(-operand->data.integer_value);
            } else if (operand->type == CONST_VALUE_FLOAT) {
                return const_value_create_float(-operand->data.float_value);
            }
            break;
            
        case UNOP_NOT:
            if (operand->type == CONST_VALUE_BOOLEAN) {
                return const_value_create_boolean(!operand->data.boolean_value);
            }
            break;
            
        case UNOP_BITWISE_NOT:
            if (operand->type == CONST_VALUE_INTEGER) {
                return const_value_create_integer(~operand->data.integer_value);
            }
            break;
            
        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 (SourceLocation){NULL, 0, 0, 0},
                                 "Unsupported unary operator in const expression");
            return NULL;
    }
    
    semantic_report_error(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH,
                         (SourceLocation){NULL, 0, 0, 0},
                         "Type mismatch in unary const expression");
    return NULL;
}

/**
 * Evaluate a sizeof const expression
 */
static ConstValue *evaluate_const_sizeof(SemanticAnalyzer *analyzer, ASTNode *type_node) {
    if (!analyzer || !type_node) return NULL;
    
    // Get the type descriptor from the type node
    TypeDescriptor *type = analyze_type_node(analyzer, type_node);
    if (!type) {
        semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                             type_node->location,
                             "Invalid type in sizeof expression");
        return NULL;
    }
    
    // Calculate size based on type
    size_t size = 0;
    switch (type->category) {
        case TYPE_PRIMITIVE:
            switch (type->data.primitive.primitive_kind) {
                case PRIMITIVE_I8:
                case PRIMITIVE_U8:
                    size = 1;
                    break;
                case PRIMITIVE_I16:
                case PRIMITIVE_U16:
                    size = 2;
                    break;
                case PRIMITIVE_I32:
                case PRIMITIVE_U32:
                case PRIMITIVE_F32:
                    size = 4;
                    break;
                case PRIMITIVE_I64:
                case PRIMITIVE_U64:
                case PRIMITIVE_F64:
                    size = 8;
                    break;
                case PRIMITIVE_BOOL:
                    size = 1;
                    break;
                case PRIMITIVE_STRING:
                    size = 8; // Pointer size
                    break;
                case PRIMITIVE_USIZE:
                case PRIMITIVE_ISIZE:
                    size = 8; // Platform-dependent, 8 bytes on 64-bit systems
                    break;
                default:
                    size = 8; // Default pointer size
                    break;
            }
            break;
            
        case TYPE_POINTER:
            size = 8; // Pointer size on 64-bit systems
            break;
            
        case TYPE_SLICE:
            size = 16; // Pointer + length
            break;
            
        default:
            // For complex types, use a default size for now
            // In a full implementation, we'd calculate actual struct/enum sizes
            size = 8;
            break;
    }
    
    type_descriptor_release(type);
    return const_value_create_integer((int64_t)size);
}

/**
 * Main const expression evaluation function
 */
ConstValue *evaluate_const_expression(SemanticAnalyzer *analyzer, ASTNode *const_expr) {
    if (!analyzer || !const_expr || const_expr->type != AST_CONST_EXPR) {
        return NULL;
    }
    
    switch (const_expr->data.const_expr.expr_type) {
        case CONST_EXPR_LITERAL:
            return evaluate_const_literal(analyzer, const_expr->data.const_expr.data.literal);
            
        case CONST_EXPR_IDENTIFIER:
            return evaluate_const_identifier(analyzer, const_expr->data.const_expr.data.identifier);
            
        case CONST_EXPR_BINARY_OP: {
            // Evaluate left and right operands
            ConstValue *left = evaluate_const_expression(analyzer, const_expr->data.const_expr.data.binary.left);
            if (!left) return NULL;
            
            ConstValue *right = evaluate_const_expression(analyzer, const_expr->data.const_expr.data.binary.right);
            if (!right) {
                const_value_destroy(left);
                return NULL;
            }
            
            ConstValue *result = evaluate_const_binary_op(analyzer, left, 
                                                        const_expr->data.const_expr.data.binary.op, 
                                                        right);
            const_value_destroy(left);
            const_value_destroy(right);
            return result;
        }
        
        case CONST_EXPR_UNARY_OP: {
            // Evaluate operand
            ConstValue *operand = evaluate_const_expression(analyzer, const_expr->data.const_expr.data.unary.operand);
            if (!operand) return NULL;
            
            ConstValue *result = evaluate_const_unary_op(analyzer, 
                                                       const_expr->data.const_expr.data.unary.op, 
                                                       operand);
            const_value_destroy(operand);
            return result;
        }
        
        case CONST_EXPR_SIZEOF:
            return evaluate_const_sizeof(analyzer, const_expr->data.const_expr.data.sizeof_expr.type);
            
        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 const_expr->location,
                                 "Unsupported const expression type");
            return NULL;
    }
}

// =============================================================================
// DEPENDENCY CYCLE DETECTION
// =============================================================================

/**
 * Check for dependency cycles in const declarations
 */
bool check_const_dependency_cycle(SemanticAnalyzer *analyzer, const char *const_name, ASTNode *const_expr) {
    if (!analyzer || !const_name || !const_expr) return false;
    
    // Simple cycle detection: track visited constants
    // In a full implementation, we'd use a more sophisticated graph-based approach
    
    // For now, we'll just check direct self-reference
    if (const_expr->type == AST_CONST_EXPR && 
        const_expr->data.const_expr.expr_type == CONST_EXPR_IDENTIFIER) {
        const char *ref_name = const_expr->data.const_expr.data.identifier;
        if (ref_name && strcmp(ref_name, const_name) == 0) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_OPERATION,
                                 const_expr->location,
                                 "Const declaration '%s' cannot reference itself", const_name);
            return false;
        }
    }
    
    // For binary and unary expressions, recursively check operands
    if (const_expr->type == AST_CONST_EXPR) {
        switch (const_expr->data.const_expr.expr_type) {
            case CONST_EXPR_BINARY_OP:
                if (!check_const_dependency_cycle(analyzer, const_name, const_expr->data.const_expr.data.binary.left) ||
                    !check_const_dependency_cycle(analyzer, const_name, const_expr->data.const_expr.data.binary.right)) {
                    return false;
                }
                break;
                
            case CONST_EXPR_UNARY_OP:
                if (!check_const_dependency_cycle(analyzer, const_name, const_expr->data.const_expr.data.unary.operand)) {
                    return false;
                }
                break;
                
            default:
                break;
        }
    }
    
    return true;
}

// =============================================================================
// PUBLIC LITERAL EVALUATION
// =============================================================================

/**
 * Evaluate a literal node as a constant value
 * This is a public wrapper around evaluate_const_literal for direct literal evaluation
 */
ConstValue *evaluate_literal_as_const(SemanticAnalyzer *analyzer, ASTNode *literal) {
    return evaluate_const_literal(analyzer, literal);
}
