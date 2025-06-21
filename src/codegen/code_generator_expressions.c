/**
 * Asthra Programming Language Compiler
 * Code Generator Expression Implementation - Include Aggregator
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file now serves as an include aggregator for expression code generation.
 * The actual implementation is split across specialized modules.
 */

#include "code_generator.h"
#include "expression_literals.h"
#include "expression_operations.h"
#include "expression_calls.h"
#include "expression_structures.h"
#include "../analysis/type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <execinfo.h>

// =============================================================================
// MAIN EXPRESSION CODE GENERATION DISPATCHER
// =============================================================================

bool code_generate_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    // ARCHITECTURAL SAFEGUARD: Prevent inappropriate calls during parsing
    if (!generator || !expr || target_reg == ASTHRA_REG_NONE) return false;
    
    // Note: Safeguards implemented below to prevent architectural violations
    
    // Safeguard: Check if generator is in a valid state for code generation
    if (!generator->semantic_analyzer) {
        // This indicates the generator is being called before semantic analysis
        // which violates the separation of concerns principle
        fprintf(stderr, "SAFEGUARD TRIGGERED: No semantic analyzer!\n");
        fflush(stderr);
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "ARCHITECTURAL VIOLATION: Code generation called before semantic analysis. "
                                   "This violates compiler architecture separation of concerns.");
        return false;
    }
    
    // Additional safeguard: Verify the AST has been semantically analyzed
    if (expr->type == AST_IDENTIFIER && !expr->type_info) {
        // Identifiers should have type information attached by semantic analysis
        fprintf(stderr, "SAFEGUARD TRIGGERED: Identifier '%s' without type info at %p!\n", 
                expr->data.identifier.name ? expr->data.identifier.name : "unknown", (void*)expr);
        fflush(stderr);
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION,
                                   "ARCHITECTURAL VIOLATION: Identifier '%s' lacks type information from semantic analysis. "
                                   "Code generation must only occur after semantic analysis.",
                                   expr->data.identifier.name ? expr->data.identifier.name : "unknown");
        return false;
    }
    
    // Dispatch to specialized generation functions based on expression type
    switch (expr->type) {
        // Literal expressions
        case AST_INTEGER_LITERAL:
            return generate_integer_literal(generator, expr, target_reg);
        
        case AST_CHAR_LITERAL:
            return generate_char_literal(generator, expr, target_reg);
        
        case AST_STRING_LITERAL:
            return generate_string_literal(generator, expr, target_reg);
        
        case AST_FLOAT_LITERAL:
            return generate_float_literal(generator, expr, target_reg);
        
        case AST_BOOL_LITERAL:
        case AST_BOOLEAN_LITERAL:
            return generate_bool_literal(generator, expr, target_reg);
        
        case AST_UNIT_LITERAL:
            return generate_unit_literal(generator, expr, target_reg);
        
        // Operations and identifiers
        case AST_IDENTIFIER:
            return generate_identifier_expression(generator, expr, target_reg);
        
        case AST_BINARY_EXPR:
            return generate_binary_expression(generator, expr, target_reg);
        
        case AST_ASSIGNMENT:
            return generate_assignment_expression(generator, expr, target_reg);
        
        // Function calls
        case AST_ASSOCIATED_FUNC_CALL:
            return code_generate_associated_function_call(generator, expr, target_reg);
        
        case AST_CALL_EXPR:
            return code_generate_function_call(generator, expr, target_reg);
        
        case AST_ENUM_VARIANT:
            return code_generate_enum_variant_construction(generator, expr, target_reg);
        
        // Structure expressions
        case AST_STRUCT_LITERAL:
            return generate_struct_literal(generator, expr, target_reg);
        
        case AST_FIELD_ACCESS:
            return generate_field_access(generator, expr, target_reg);
        
        case AST_ARRAY_LITERAL:
            return generate_array_literal(generator, expr, target_reg);
        
        case AST_TUPLE_LITERAL:
            return generate_tuple_literal(generator, expr, target_reg);
        
        case AST_INDEX_ACCESS:
            return generate_index_access(generator, expr, target_reg);
        
        case AST_SLICE_EXPR:
            return generate_slice_expr(generator, expr, target_reg);
        
        // Handle other expression types as needed
        default:
            code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, "Unsupported expression type: %d", expr->type);
            return false;
    }
}

// This file now serves as an include aggregator for expression code generation.
// The actual functions are implemented in the specialized modules:
//
// - expression_literals.h: generate_integer_literal, generate_char_literal, generate_string_literal,
//                          generate_float_literal, generate_bool_literal, generate_unit_literal,
//                          escape_string_for_c_generation, is_multiline_string_content, create_string_comment
// - expression_operations.h: generate_binary_expression, generate_assignment_expression, 
//                            generate_identifier_expression, create_mangled_function_name
// - expression_calls.h: code_generate_associated_function_call, code_generate_function_call,
//                       code_generate_enum_variant_construction
// - expression_structures.h: generate_struct_literal, generate_field_access, generate_array_literal
//
// All functionality is now available through their respective headers.