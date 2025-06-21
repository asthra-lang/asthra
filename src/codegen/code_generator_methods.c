/**
 * Asthra Programming Language Compiler
 * Code Generator Method and Impl Block Generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Method generation and impl block handling functionality
 */

#include "code_generator_common.h"

// =============================================================================
// IMPL BLOCK AND METHOD CODE GENERATION
// =============================================================================

// Helper function to create mangled method names
static char *create_mangled_method_name(const char *struct_name, const char *method_name, bool is_instance_method) {
    if (!struct_name || !method_name) {
        return NULL;
    }
    
    // Create mangled name: StructName_{instance|associated}_{method_name}
    const char *method_type = is_instance_method ? "instance" : "associated";
    size_t total_len = strlen(struct_name) + strlen(method_type) + strlen(method_name) + 3; // for underscores and null terminator
    
    char *mangled_name = malloc(total_len);
    if (!mangled_name) {
        return NULL;
    }
    
    snprintf(mangled_name, total_len, "%s_%s_%s", struct_name, method_type, method_name);
    return mangled_name;
}

bool code_generate_impl_block(CodeGenerator *generator, ASTNode *impl_block) {
    if (!generator || !impl_block || impl_block->type != AST_IMPL_BLOCK) {
        return false;
    }
    
    const char *struct_name = impl_block->data.impl_block.struct_name;
    ASTNodeList *methods = impl_block->data.impl_block.methods;
    
    if (!struct_name) {
        return false;
    }
    
    // Generate each method in the impl block
    if (methods) {
        size_t method_count = ast_node_list_size(methods);
        for (size_t i = 0; i < method_count; i++) {
            ASTNode *method = ast_node_list_get(methods, i);
            if (method && method->type == AST_METHOD_DECL) {
                if (!code_generate_method(generator, method, struct_name)) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool code_generate_method(CodeGenerator *generator, ASTNode *method_decl, const char *struct_name) {
    if (!generator || !method_decl || method_decl->type != AST_METHOD_DECL || !struct_name) {
        return false;
    }
    
    const char *method_name = method_decl->data.method_decl.name;
    if (!method_name) {
        return false;
    }
    
    // Create mangled method name
    char *mangled_name = create_mangled_method_name(struct_name, method_name, 
                                                   method_decl->data.method_decl.is_instance_method);
    if (!mangled_name) {
        return false;
    }
    
    // Set current function context
    free(generator->current_function_name);
    generator->current_function_name = strdup(mangled_name);
    
    // Create function label for the method
    char *func_label = label_manager_create_label(generator->label_manager, 
                                                 LABEL_FUNCTION, mangled_name);
    if (!func_label) {
        free(mangled_name);
        return false;
    }
    
    // Emit export directive for public methods
    if (method_decl->data.method_decl.visibility == VISIBILITY_PUBLIC) {
        printf("DEBUG: Method %s is PUBLIC, emitting .global directive\n", mangled_name);
        // Create a comment instruction to represent the export directive
        AssemblyInstruction *export_inst = create_instruction(INST_MOV, 0); // Use MOV as placeholder
        if (export_inst) {
            char export_directive[256];
            snprintf(export_directive, sizeof(export_directive), ".global %s", mangled_name);
            export_inst->comment = strdup(export_directive);
            instruction_buffer_add(generator->instruction_buffer, export_inst);
        }
    } else {
        printf("DEBUG: Method %s is PRIVATE (visibility=%d)\n", mangled_name, method_decl->data.method_decl.visibility);
    }
    
    // Generate function prologue
    Register saved_regs[] = {REG_RBP, REG_RBX, REG_R12, REG_R13, REG_R14, REG_R15};
    size_t saved_count = sizeof(saved_regs) / sizeof(Register);
    
    if (!generate_function_prologue(generator, DEFAULT_STACK_FRAME_SIZE, saved_regs, saved_count)) {
        free(mangled_name);
        free(func_label);
        return false;
    }
    
    // Add method parameters to local variable table
    ASTNodeList *params = method_decl->data.method_decl.params;
    if (params) {
        size_t param_count = ast_node_list_size(params);
        for (size_t i = 0; i < param_count; i++) {
            ASTNode *param = ast_node_list_get(params, i);
            if (param && param->type == AST_PARAM_DECL) {
                const char *param_name = param->data.param_decl.name;
                if (param_name) {
                    // Parameters are typically passed in registers or on stack
                    // For simplicity, assign stack offset based on parameter index
                    int32_t param_offset = -((int32_t)(i + 1) * 8); // 8 bytes per parameter
                    
                    // Use default type and size for now (would need semantic analyzer integration for proper types)
                    if (!add_function_parameter(generator, param_name, NULL, 8, param_offset)) {
                        // Continue with other parameters even if one fails
                        continue;
                    }
                }
            }
        }
    }
    
    // Generate method body
    ASTNode *body = method_decl->data.method_decl.body;
    if (body && !code_generate_statement(generator, body)) {
        free(mangled_name);
        free(func_label);
        return false;
    }
    
    // Generate function epilogue
    if (!generate_function_epilogue(generator, DEFAULT_STACK_FRAME_SIZE, saved_regs, saved_count)) {
        free(mangled_name);
        free(func_label);
        return false;
    }
    
    atomic_fetch_add(&generator->stats.functions_generated, 1);
    free(mangled_name);
    free(func_label);
    return true;
} 
