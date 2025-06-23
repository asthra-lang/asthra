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

// Forward declaration for type generation
static const char* get_c_type_string(ASTNode *type_node);

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
            // First pass: Generate forward declarations for all functions
            if (node->data.program.declarations) {
                ASTNodeList *declarations = node->data.program.declarations;
                for (size_t i = 0; i < declarations->count; i++) {
                    ASTNode *decl = declarations->nodes[i];
                    if (decl && decl->type == AST_FUNCTION_DECL) {
                        const char *func_name = decl->data.function_decl.name;
                        
                        // Skip forward declaration for main function
                        if (strcmp(func_name, "main") == 0) {
                            continue;
                        }
                        
                        // Generate forward declaration
                        const char *return_type = "void"; // Default to void
                        if (decl->data.function_decl.return_type) {
                            return_type = get_c_type_string(decl->data.function_decl.return_type);
                        }
                        fprintf(output, "%s %s();\n", return_type, func_name);
                    }
                }
                fprintf(output, "\n");
                
                // Second pass: Generate function definitions
                for (size_t i = 0; i < declarations->count; i++) {
                    if (declarations->nodes[i]) {
                        generate_c_code(output, declarations->nodes[i]);
                    }
                }
            }
            break;
            
        case AST_FUNCTION_DECL:
            {
                // Generate proper C function definition
                const char *func_name = node->data.function_decl.name;
                const char *return_type = "void"; // Default to void
                
                if (node->data.function_decl.return_type) {
                    return_type = get_c_type_string(node->data.function_decl.return_type);
                }
                
                // Check if this is the main function
                if (strcmp(func_name, "main") == 0) {
                    // Generate C main function
                    fprintf(output, "int main() {\n");
                } else {
                    // Generate regular function
                    fprintf(output, "%s %s() {\n", return_type, func_name);
                }
                
                // Generate function body
                if (node->data.function_decl.body) {
                    generate_c_code(output, node->data.function_decl.body);
                }
                
                // Add return statement if needed
                if (strcmp(func_name, "main") == 0) {
                    fprintf(output, "    return 0;\n");
                }
                
                fprintf(output, "}\n\n");
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
                fprintf(output, "((AsthraSliceHeader){.ptr = NULL, .len = 0, .cap = 0, .element_size = sizeof(char*), .ownership = ASTHRA_OWNERSHIP_GC, .is_mutable = 0, .type_id = 0})");
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
            
        case AST_UNARY_EXPR:
            // Handle unary expressions
            switch (node->data.unary_expr.operator) {
                case UNOP_MINUS: fprintf(output, "-"); break;
                case UNOP_NOT: fprintf(output, "!"); break;
                case UNOP_BITWISE_NOT: fprintf(output, "~"); break;
                case UNOP_DEREF: fprintf(output, "*"); break;
                case UNOP_ADDRESS_OF: fprintf(output, "&"); break;
                default: fprintf(output, "/* unknown unary op */"); break;
            }
            generate_c_code(output, node->data.unary_expr.operand);
            break;
            
        case AST_LET_STMT:
            // Handle variable declarations with immutable-by-default semantics
            if (node->data.let_stmt.name) {
                // Use our type helper function to get the correct C type
                const char *c_type = get_c_type_string(node->data.let_stmt.type);
                
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
            // Handle for loops (for-in style)
            if (node->data.for_stmt.variable && node->data.for_stmt.iterable) {
                const char *iterator_var_name = node->data.for_stmt.variable;
                
                // Generate proper slice iteration
                fprintf(output, "    // Generated from Asthra for-in loop\n");
                fprintf(output, "    {\n");
                
                // Get the slice to iterate over
                fprintf(output, "        AsthraSliceHeader _slice = ");
                generate_c_code(output, node->data.for_stmt.iterable);
                fprintf(output, ";\n");
                
                // Determine element type from the iterable's type info
                const char *element_type = "int"; // Default
                // TODO: Once TypeInfo is properly exposed in code generation,
                // we can check if the iterable is a slice and get its element type
                
                // Generate the for loop
                fprintf(output, "        for (size_t _idx = 0; _idx < _slice.len; _idx++) {\n");
                fprintf(output, "            %s %s = *((%s*)((char*)_slice.ptr + _idx * _slice.element_size));\n",
                        element_type, iterator_var_name, element_type);
                
                // Generate loop body
                if (node->data.for_stmt.body) {
                    generate_c_code(output, node->data.for_stmt.body);
                }
                
                fprintf(output, "        }\n");
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
            
        case AST_ARRAY_LITERAL:
            // Generate array literal as slice initialization
            if (node->data.array_literal.elements) {
                ASTNodeList *elements = node->data.array_literal.elements;
                
                // For now, we'll generate a static array and create a slice from it
                // This is a simplified approach - a real implementation would use heap allocation
                fprintf(output, "({\n");
                
                // Determine element type from first element or type info
                const char *element_type = "int"; // Default to int
                // TODO: Once TypeInfo is properly exposed in code generation,
                // we can check the actual slice element type
                if (elements->count > 0 && elements->nodes[0]) {
                    // Infer from first element
                    ASTNode *first = elements->nodes[0];
                    if (first->type == AST_STRING_LITERAL) {
                        element_type = "char*";
                    } else if (first->type == AST_FLOAT_LITERAL) {
                        element_type = "double";
                    } else if (first->type == AST_BOOL_LITERAL) {
                        element_type = "int";
                    }
                }
                
                // Generate static array
                fprintf(output, "        static %s _arr[] = {", element_type);
                for (size_t i = 0; i < elements->count; i++) {
                    if (i > 0) fprintf(output, ", ");
                    generate_c_code(output, elements->nodes[i]);
                }
                fprintf(output, "};\n");
                
                // Create slice header - note the expression must evaluate to the struct, not void
                fprintf(output, "        AsthraSliceHeader _slice = {.ptr = _arr, .len = %zu, .cap = %zu, .element_size = sizeof(%s), .ownership = ASTHRA_OWNERSHIP_GC, .is_mutable = 0, .type_id = 0};\n",
                        elements->count, elements->count, element_type);
                fprintf(output, "        _slice;\n");  // Return the slice
                fprintf(output, "    })");
            }
            break;
            
        case AST_SLICE_EXPR:
            // Generate slice expression array[start:end]
            if (node->data.slice_expr.array) {
                fprintf(output, "asthra_slice_subslice(");
                generate_c_code(output, node->data.slice_expr.array);
                fprintf(output, ", ");
                
                // Start index (default to 0 if NULL)
                if (node->data.slice_expr.start) {
                    generate_c_code(output, node->data.slice_expr.start);
                } else {
                    fprintf(output, "0");
                }
                fprintf(output, ", ");
                
                // End index (default to length if NULL)
                if (node->data.slice_expr.end) {
                    generate_c_code(output, node->data.slice_expr.end);
                } else {
                    // Need to get the length of the array
                    fprintf(output, "asthra_slice_get_len(");
                    generate_c_code(output, node->data.slice_expr.array);
                    fprintf(output, ")");
                }
                fprintf(output, ")");
            }
            break;
            
        case AST_INDEX_ACCESS:
            // Generate array index access array[index]
            if (node->data.index_access.array && node->data.index_access.index) {
                // We need to dereference the element pointer and cast to the correct type
                const char *element_type = "int"; // Default to int
                // For now, we'll use a simple heuristic based on the expected result type
                // A proper implementation would use type inference from semantic analysis
                
                fprintf(output, "(*(%s*)asthra_slice_get_element(", element_type);
                generate_c_code(output, node->data.index_access.array);
                fprintf(output, ", ");
                generate_c_code(output, node->data.index_access.index);
                fprintf(output, "))");
            }
            break;
            
        case AST_SLICE_LENGTH_ACCESS:
            // Generate slice.len access
            if (node->data.slice_length_access.slice) {
                fprintf(output, "asthra_slice_get_len(");
                generate_c_code(output, node->data.slice_length_access.slice);
                fprintf(output, ")");
            }
            break;
            
        case AST_SLICE_TYPE:
            // This shouldn't appear in expression context, but handle it for completeness
            fprintf(output, "/* slice type */");
            break;
            
        default:
            // For unhandled node types, just continue
            break;
    }
    
    return 0;
}

// Helper function to convert AST type nodes to C type strings
static const char* get_c_type_string(ASTNode *type_node) {
    if (!type_node) return "void";
    
    switch (type_node->type) {
        case AST_BASE_TYPE:
            if (type_node->data.base_type.name) {
                const char *name = type_node->data.base_type.name;
                if (strcmp(name, "bool") == 0) return "int";
                if (strcmp(name, "i32") == 0 || strcmp(name, "int") == 0) return "int";
                if (strcmp(name, "i64") == 0) return "long long";
                if (strcmp(name, "f32") == 0 || strcmp(name, "float") == 0) return "float";
                if (strcmp(name, "f64") == 0) return "double";
                if (strcmp(name, "string") == 0) return "char*";
                if (strcmp(name, "void") == 0) return "void";
                if (strcmp(name, "usize") == 0) return "size_t";
            }
            break;
        case AST_SLICE_TYPE:
            return "AsthraSliceHeader";
        case AST_PTR_TYPE:
            // Simplified pointer type handling
            return "void*";
        default:
            break;
    }
    
    return "void";
} 
