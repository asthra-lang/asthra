/**
 * Asthra Programming Language Compiler
 * Code generation functions - function declaration and program generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdio.h>
#include <string.h>

int c_generate_program(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_PROGRAM) {
        return -1;
    }

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

    return 0;
}

int c_generate_function_decl(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_FUNCTION_DECL) {
        return -1;
    }

    // Generate proper C function definition
    const char *func_name = node->data.function_decl.name;
    const char *return_type = "void"; // Default to void

    if (node->data.function_decl.return_type) {
        return_type = get_c_type_string(node->data.function_decl.return_type);
    }

    // Check if this is the main function
    bool is_main = strcmp(func_name, "main") == 0;
    bool main_returns_int = false;
    bool main_returns_void = false;

    if (is_main) {
        // Check if main returns int/i32 or void
        if (node->data.function_decl.return_type &&
            node->data.function_decl.return_type->type == AST_BASE_TYPE &&
            node->data.function_decl.return_type->data.base_type.name) {
            const char *type_name = node->data.function_decl.return_type->data.base_type.name;
            if (strcmp(type_name, "i32") == 0 || strcmp(type_name, "int") == 0) {
                main_returns_int = true;
            } else if (strcmp(type_name, "void") == 0) {
                main_returns_void = true;
            }
        }
        // Generate C main function
        fprintf(output, "int main() {\n");
    } else {
        // Generate regular function
        fprintf(output, "%s %s() {\n", return_type, func_name);
    }

    // Set context for return statement handling
    const char *old_function_name = current_function_name;
    bool old_function_returns_void = current_function_returns_void;
    current_function_name = func_name;
    current_function_returns_void = (strcmp(return_type, "void") == 0) || main_returns_void;

    // Generate function body
    if (node->data.function_decl.body) {
        generate_c_code(output, node->data.function_decl.body);
    }

    // Add return statement only if main doesn't already end with one
    if (is_main && !ends_with_return(node->data.function_decl.body)) {
        fprintf(output, "    return 0;\n");
    }

    fprintf(output, "}\n\n");

    // Restore context
    current_function_name = old_function_name;
    current_function_returns_void = old_function_returns_void;

    return 0;
}