/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Const declarations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_toplevel.h"
#include "grammar_expressions.h"
#include "grammar_statements.h"
#include "grammar_annotations.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CONST DECLARATION PARSING
// =============================================================================

/**
 * Recursively convert an expression AST to a const expression AST
 */
static ASTNode *convert_expr_to_const_expr(ASTNode *expr) {
    if (!expr) return NULL;
    
    SourceLocation loc = expr->location;
    
    // Create const expression wrapper node
    ASTNode *const_expr = ast_create_node(AST_CONST_EXPR, loc);
    if (!const_expr) {
        return NULL;
    }
    
    // Set const expression type based on parsed expression
    switch (expr->type) {
        case AST_INTEGER_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_STRING_LITERAL:
        case AST_BOOL_LITERAL:
        case AST_CHAR_LITERAL:
            const_expr->data.const_expr.expr_type = CONST_EXPR_LITERAL;
            const_expr->data.const_expr.data.literal = expr;
            break;
            
        case AST_IDENTIFIER:
            const_expr->data.const_expr.expr_type = CONST_EXPR_IDENTIFIER;
            const_expr->data.const_expr.data.identifier = strdup(expr->data.identifier.name);
            ast_free_node(expr);  // Free original, we copied the name
            break;
            
        case AST_BINARY_EXPR: {
            const_expr->data.const_expr.expr_type = CONST_EXPR_BINARY_OP;
            
            // Recursively convert left and right operands
            const_expr->data.const_expr.data.binary.left = convert_expr_to_const_expr(expr->data.binary_expr.left);
            const_expr->data.const_expr.data.binary.op = expr->data.binary_expr.operator;
            const_expr->data.const_expr.data.binary.right = convert_expr_to_const_expr(expr->data.binary_expr.right);
            
            // Check if conversion succeeded
            if (!const_expr->data.const_expr.data.binary.left || !const_expr->data.const_expr.data.binary.right) {
                ast_free_node(const_expr);
                ast_free_node(expr);
                return NULL;
            }
            
            // Free the original binary expression wrapper (children are now owned by const_expr)
            expr->data.binary_expr.left = NULL;
            expr->data.binary_expr.right = NULL;
            ast_free_node(expr);
            break;
        }
            
        case AST_UNARY_EXPR: {
            // Check if this is a sizeof expression
            if (expr->data.unary_expr.operator == UNOP_SIZEOF) {
                const_expr->data.const_expr.expr_type = CONST_EXPR_SIZEOF;
                const_expr->data.const_expr.data.sizeof_expr.type = expr->data.unary_expr.operand;
                
                // Transfer ownership of the type node
                expr->data.unary_expr.operand = NULL;
                ast_free_node(expr);
            } else {
                // Regular unary expression
                const_expr->data.const_expr.expr_type = CONST_EXPR_UNARY_OP;
                const_expr->data.const_expr.data.unary.op = expr->data.unary_expr.operator;
                
                // Recursively convert operand
                const_expr->data.const_expr.data.unary.operand = convert_expr_to_const_expr(expr->data.unary_expr.operand);
                
                // Check if conversion succeeded
                if (!const_expr->data.const_expr.data.unary.operand) {
                    ast_free_node(const_expr);
                    ast_free_node(expr);
                    return NULL;
                }
                
                // Free the original unary expression wrapper (operand is now owned by const_expr)
                expr->data.unary_expr.operand = NULL;
                ast_free_node(expr);
            }
            break;
        }
            
        default:
            // For other expression types, treat as literal for now
            // Semantic analysis will validate if it's actually compile-time evaluable
            const_expr->data.const_expr.expr_type = CONST_EXPR_LITERAL;
            const_expr->data.const_expr.data.literal = expr;
            break;
    }
    
    return const_expr;
}

/**
 * Parse const expression for compile-time evaluation
 * ConstExpr <- Literal / SimpleIdent / BinaryConstExpr / UnaryConstExpr / SizeOf
 */
static ASTNode *parse_const_expr(Parser *parser) {
    if (!parser) return NULL;
    
    // Try to parse different const expression types
    if (match_token(parser, TOKEN_SIZEOF)) {
        return parse_sizeof(parser);
    }
    
    // For now, parse as regular expression and validate later in semantic analysis
    // This allows us to handle all literal types, identifiers, and operations
    ASTNode *expr = parse_expr(parser);
    if (!expr) {
        return NULL;
    }
    
    // Recursively convert to const expression
    return convert_expr_to_const_expr(expr);
}

/**
 * Parse const declaration
 * ConstDecl <- 'const' SimpleIdent ':' Type '=' ConstExpr ';'
 */
ASTNode *parse_const_decl(Parser *parser) {
    if (!parser) return NULL;
    
    SourceLocation start_loc = parser->current_token.location;
    
    // Expect 'const' keyword
    if (!expect_token(parser, TOKEN_CONST)) {
        return NULL;
    }
    
    // Parse constant name
    if (!match_token(parser, TOKEN_IDENTIFIER)) {
        report_error(parser, "Expected constant name after 'const'");
        return NULL;
    }
    
    char *const_name = strdup(parser->current_token.data.identifier.name);
    advance_token(parser);
    
    // Expect colon
    if (!expect_token(parser, TOKEN_COLON)) {
        free(const_name);
        return NULL;
    }
    
    // Parse type annotation (required)
    ASTNode *const_type = parse_type(parser);
    if (!const_type) {
        report_error(parser, "Expected type annotation for constant");
        free(const_name);
        return NULL;
    }
    
    // Expect assignment operator
    if (!expect_token(parser, TOKEN_ASSIGN)) {
        ast_free_node(const_type);
        free(const_name);
        return NULL;
    }
    
    // Parse const expression
    ASTNode *const_value = parse_const_expr(parser);
    if (!const_value) {
        report_error(parser, "Expected constant expression");
        ast_free_node(const_type);
        free(const_name);
        return NULL;
    }
    
    // Expect semicolon
    if (!expect_token(parser, TOKEN_SEMICOLON)) {
        ast_free_node(const_value);
        ast_free_node(const_type);
        free(const_name);
        return NULL;
    }
    
    // Create const declaration node
    ASTNode *const_decl = ast_create_node(AST_CONST_DECL, start_loc);
    if (!const_decl) {
        ast_free_node(const_value);
        ast_free_node(const_type);
        free(const_name);
        return NULL;
    }
    
    // Set const declaration data
    const_decl->data.const_decl.name = const_name;
    const_decl->data.const_decl.type = const_type;
    const_decl->data.const_decl.value = const_value;
    const_decl->data.const_decl.visibility = VISIBILITY_PRIVATE;  // Will be set by caller
    const_decl->data.const_decl.annotations = NULL;              // Will be set by caller
    
    return const_decl;
} 
