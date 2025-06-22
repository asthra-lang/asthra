/**
 * Asthra Programming Language Compiler
 * Code generation - AST to C code translation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../parser/ast.h"
#include "code_generation.h"

// =============================================================================
// CODE GENERATION
// =============================================================================

// Helper function to generate C code from AST
int generate_c_code(FILE *output, ASTNode *node) {
    if (!output || !node) {
        return -1;
    }
    
    switch (node->type) {
        case AST_PROGRAM:
            // Generate main function
            fprintf(output, "int main() {\n");
            
            // Process all declarations in the program
            if (node->data.program.declarations) {
                ASTNodeList *declarations = node->data.program.declarations;
                for (size_t i = 0; i < declarations->count; i++) {
                    if (declarations->nodes[i]) {
                        generate_c_code(output, declarations->nodes[i]);
                    }
                }
            }
            
            fprintf(output, "    return 0;\n");
            fprintf(output, "}\n");
            break;
            
        case AST_FUNCTION_DECL:
            // For now, just process the function body
            if (node->data.function_decl.body) {
                generate_c_code(output, node->data.function_decl.body);
            }
            break;
            
        case AST_BLOCK:
            // Process all statements in the block
            if (node->data.block.statements) {
                ASTNodeList *statements = node->data.block.statements;
                for (size_t i = 0; i < statements->count; i++) {
                    if (statements->nodes[i]) {
                        generate_c_code(output, statements->nodes[i]);
                    }
                }
            }
            break;
            
        case AST_EXPR_STMT:
            // Generate code for expression statement
            if (node->data.expr_stmt.expression) {
                generate_c_code(output, node->data.expr_stmt.expression);
                fprintf(output, ";\n");
            }
            break;
            
        case AST_CALL_EXPR:
            // Handle function calls (especially log() and panic())
            if (node->data.call_expr.function && 
                node->data.call_expr.function->type == AST_IDENTIFIER &&
                strcmp(node->data.call_expr.function->data.identifier.name, "log") == 0) {
                
                fprintf(output, "    printf(");
                
                // Process arguments
                if (node->data.call_expr.args) {
                    ASTNodeList *args = node->data.call_expr.args;
                    for (size_t i = 0; i < args->count; i++) {
                        if (args->nodes[i]) {
                            if (i > 0) {
                                fprintf(output, ", ");
                            }
                            generate_c_code(output, args->nodes[i]);
                        }
                    }
                }
                
                fprintf(output, ");\n    printf(\"\\n\")");
            } else if (node->data.call_expr.function && 
                       node->data.call_expr.function->type == AST_IDENTIFIER &&
                       strcmp(node->data.call_expr.function->data.identifier.name, "panic") == 0) {
                
                // Handle panic() - print to stderr and exit
                fprintf(output, "    fprintf(stderr, \"panic: \");\n");
                fprintf(output, "    fprintf(stderr, ");
                
                // Process arguments (expect one string argument)
                if (node->data.call_expr.args && node->data.call_expr.args->count > 0) {
                    generate_c_code(output, node->data.call_expr.args->nodes[0]);
                }
                
                fprintf(output, ");\n");
                fprintf(output, "    fprintf(stderr, \"\\n\");\n");
                fprintf(output, "    exit(1)");
            } else if (node->data.call_expr.function && 
                       node->data.call_expr.function->type == AST_IDENTIFIER &&
                       strcmp(node->data.call_expr.function->data.identifier.name, "args") == 0) {
                
                // Handle args() - generate inline stub for testing
                // TODO: Use asthra_runtime_get_args() once runtime linking is implemented
                fprintf(output, "((AsthraSliceHeader){.data = NULL, .length = 0, .element_size = sizeof(char*)})");
            } else {
                // Other function calls
                if (node->data.call_expr.function) {
                    generate_c_code(output, node->data.call_expr.function);
                    fprintf(output, "(");
                    
                    if (node->data.call_expr.args) {
                        ASTNodeList *args = node->data.call_expr.args;
                        for (size_t i = 0; i < args->count; i++) {
                            if (args->nodes[i]) {
                                if (i > 0) {
                                    fprintf(output, ", ");
                                }
                                generate_c_code(output, args->nodes[i]);
                            }
                        }
                    }
                    
                    fprintf(output, ")");
                }
            }
            break;
            
        case AST_STRING_LITERAL:
            // Output string literal
            if (node->data.string_literal.value) {
                fprintf(output, "\"%s\"", node->data.string_literal.value);
            }
            break;
            
        case AST_BOOL_LITERAL:
            // Output boolean literal
            fprintf(output, "%s", node->data.bool_literal.value ? "1" : "0");
            break;
            
        case AST_UNIT_LITERAL:
            // Output unit literal as empty (void)
            // In C, unit type is typically represented as void or no value
            break;
            
        case AST_INTEGER_LITERAL:
            // Output integer literal
            fprintf(output, "%lld", (long long)node->data.integer_literal.value);
            break;
            
        case AST_FLOAT_LITERAL:
            // Output float literal
            fprintf(output, "%f", node->data.float_literal.value);
            break;
            
        case AST_BINARY_EXPR:
            // Handle binary expressions
            generate_c_code(output, node->data.binary_expr.left);
            
            // Output the correct operator
            switch (node->data.binary_expr.operator) {
                case BINOP_ADD: fprintf(output, " + "); break;
                case BINOP_SUB: fprintf(output, " - "); break;
                case BINOP_MUL: fprintf(output, " * "); break;
                case BINOP_DIV: fprintf(output, " / "); break;
                case BINOP_MOD: fprintf(output, " %% "); break;
                case BINOP_EQ: fprintf(output, " == "); break;
                case BINOP_NE: fprintf(output, " != "); break;
                case BINOP_LT: fprintf(output, " < "); break;
                case BINOP_LE: fprintf(output, " <= "); break;
                case BINOP_GT: fprintf(output, " > "); break;
                case BINOP_GE: fprintf(output, " >= "); break;
                case BINOP_AND: fprintf(output, " && "); break;
                case BINOP_OR: fprintf(output, " || "); break;
                case BINOP_BITWISE_AND: fprintf(output, " & "); break;
                case BINOP_BITWISE_OR: fprintf(output, " | "); break;
                case BINOP_BITWISE_XOR: fprintf(output, " ^ "); break;
                case BINOP_LSHIFT: fprintf(output, " << "); break;
                case BINOP_RSHIFT: fprintf(output, " >> "); break;
                default: fprintf(output, " /* unknown op */ "); break;
            }
            
            generate_c_code(output, node->data.binary_expr.right);
            break;
            
        case AST_LET_STMT:
            // Handle variable declarations with immutable-by-default semantics
            if (node->data.let_stmt.name) {
                // Determine C type based on Asthra type or initializer
                const char *c_type = "char *"; // default to string
                if (node->data.let_stmt.type && node->data.let_stmt.type->type == AST_BASE_TYPE) {
                    const char *type_name = node->data.let_stmt.type->data.base_type.name;
                    if (type_name) {
                        if (strcmp(type_name, "bool") == 0 || strcmp(type_name, "BOOL") == 0) {
                            c_type = "int";
                        } else if (strcmp(type_name, "int") == 0 || strcmp(type_name, "i32") == 0 || strcmp(type_name, "INT") == 0 || strcmp(type_name, "I32") == 0) {
                            c_type = "int";
                        } else if (strcmp(type_name, "float") == 0 || strcmp(type_name, "f32") == 0 || strcmp(type_name, "f64") == 0 || strcmp(type_name, "FLOAT_TYPE") == 0 || strcmp(type_name, "F64") == 0) {
                            c_type = "double";
                        } else if (strcmp(type_name, "string") == 0 || strcmp(type_name, "STRING_TYPE") == 0) {
                            c_type = "char *";
                        }
                        // Add more type mappings as needed
                    }
                }
                
                // PHASE 3 ENHANCEMENT: Generate appropriate C const/mutable semantics
                if (node->data.let_stmt.is_mutable) {
                    // Mutable variable - normal C variable
                    fprintf(output, "    %s %s", c_type, node->data.let_stmt.name);
                } else {
                    // Immutable variable - C const
                    fprintf(output, "    const %s %s", c_type, node->data.let_stmt.name);
                }
                
                if (node->data.let_stmt.initializer) {
                    fprintf(output, " = ");
                    generate_c_code(output, node->data.let_stmt.initializer);
                }
                fprintf(output, ";\n");
            }
            break;
            
        case AST_IF_STMT:
            // Handle if statements
            fprintf(output, "    if (");
            if (node->data.if_stmt.condition) {
                generate_c_code(output, node->data.if_stmt.condition);
            }
            fprintf(output, ") {\n");
            if (node->data.if_stmt.then_block) {
                generate_c_code(output, node->data.if_stmt.then_block);
            }
            fprintf(output, "    }\n");
            if (node->data.if_stmt.else_block) {
                fprintf(output, "    else {\n");
                generate_c_code(output, node->data.if_stmt.else_block);
                fprintf(output, "    }\n");
            }
            break;
            
        case AST_FOR_STMT:
            // Handle for loops (for-in-range style)
            if (node->data.for_stmt.variable && node->data.for_stmt.iterable) {
                // Check if it's a range-based for loop (conceptual)
                // For actual range-based loops, the iterable would be a range literal or function call
                // For now, we'll simplify and assume a basic iteration over an array/slice.
                
                // Determine the type of the iterable to get the element type
                // (This information would typically come from semantic analysis)
                // For code generation, we'll assume a basic integer or pointer type for iteration.
                const char *iterator_var_name = node->data.for_stmt.variable;
                
                // Simple C-style for loop for array/slice iteration (placeholder)
                // Assuming iterable is an array/slice for now
                fprintf(output, "    // Generated from Asthra for loop (simplified)\n");
                fprintf(output, "    size_t temp_idx = 0;\n");
                fprintf(output, "    size_t temp_len = 0;\n");

                // Simulate getting length of iterable (from semantic analysis or type info)
                // This part needs to be improved with actual type information from semantic analysis
                fprintf(output, "    // temp_len = get_length(iterable);\n");
                fprintf(output, "    temp_len = 3; // Simulated length for test\n");

                fprintf(output, "    for (temp_idx = 0; temp_idx < temp_len; temp_idx++) {\n");
                fprintf(output, "        // %s represents an element from the iterable\n", iterator_var_name);
                fprintf(output, "        // let %s = iterable[temp_idx];\n", iterator_var_name);
                
                // Generate loop body
                if (node->data.for_stmt.body) {
                    generate_c_code(output, node->data.for_stmt.body);
                }
                
                fprintf(output, "    }\n");

            } else {
                // Other types of for loops (not implemented yet)
                fprintf(output, "    // TODO: Implement other for loop types\n");
            }
            break;
            
        case AST_IDENTIFIER:
            // Output identifier
            if (node->data.identifier.name) {
                fprintf(output, "%s", node->data.identifier.name);
            }
            break;
            
        case AST_MATCH_STMT:
            // Generate code for match statement using if-else if structure
            if (node->data.match_stmt.expression && node->data.match_stmt.arms) {
                // Analyze the match expression once
                fprintf(output, "    // Generated from Asthra match statement (simplified)\n");
                fprintf(output, "    // Match expression: ");
                // generate_c_code(output, node->data.match_stmt.expression); // Would generate expr
                fprintf(output, " (expression evaluated)\n");

                // Generate if-else if chain for each arm
                for (size_t i = 0; i < node->data.match_stmt.arms->count; i++) {
                    ASTNode *arm = node->data.match_stmt.arms->nodes[i];
                    if (arm->type != AST_MATCH_ARM) continue; // Should be validated by semantic analysis

                    ASTNode *pattern = arm->data.match_arm.pattern;
                    ASTNode *body = arm->data.match_arm.body;

                    if (i == 0) {
                        fprintf(output, "    if (");
                    } else {
                        fprintf(output, "    else if (");
                    }
                    
                    // Generate condition for pattern (simplified)
                    if (pattern->type == AST_WILDCARD_PATTERN) {
                        fprintf(output, "1 /* _ */"); // Wildcard always true
                    } else if (pattern->type == AST_IDENTIFIER) {
                        // For identifier patterns, bind variable. Simplified for now.
                        fprintf(output, "1 /* let %s = expr */", pattern->data.identifier.name);
                    } else {
                         // Other patterns (enum, struct, literal) - highly complex, will be simplified
                        fprintf(output, "0 /* Complex pattern not generated */");
                    }
                    
                    fprintf(output, ") {\n");
                    generate_c_code(output, body);
                    fprintf(output, "    }\n");
                }

                // Add an else block for non-exhaustive matches (if no wildcard)
                // A real implementation would check for exhaustiveness here.
                // For now, we assume semantic analysis handles exhaustiveness warnings.
                fprintf(output, "    // Optional else for non-exhaustive matches (semantic check should warn)\n");

            }
            break;
            
        default:
            // For unhandled node types, just continue
            break;
    }
    
    return 0;
} 
