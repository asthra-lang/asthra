/**
 * Asthra Programming Language Compiler
 * Code Generator Program and Function Generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Program-level and function code generation functionality
 */

#include "code_generator_common.h"
#include "../analysis/semantic_analyzer.h"
#include "../analysis/semantic_symbols.h"

// =============================================================================
// MAIN CODE GENERATION FUNCTIONS
// =============================================================================

bool code_generate_program(CodeGenerator *generator, ASTNode *program) {
    if (!generator || !program || program->type != AST_PROGRAM) {
        return false;
    }
    
    // Generate assembly header
    if (generator->config.emit_comments) {
        // Add header comment (implementation would add to instruction buffer)
    }
    
    // Process all declarations
    ASTNodeList *declarations = program->data.program.declarations;
    if (declarations) {
        for (size_t i = 0; i < declarations->count; i++) {
            ASTNode *decl = declarations->nodes[i];
            switch (decl->type) {
                case AST_FUNCTION_DECL:
                    if (!code_generate_function(generator, decl)) {
                        return false;
                    }
                    break;
                    
                case AST_ENUM_DECL:
                    if (!code_generate_enum_declaration(generator, decl)) {
                        return false;
                    }
                    break;
                    
                case AST_STRUCT_DECL:
                    // Struct declarations are handled during semantic analysis
                    // No code generation needed for the declaration itself
                    break;
                    
                case AST_EXTERN_DECL:
                    // Extern declarations are handled by the FFI system
                    // No code generation needed for the declaration itself
                    break;
                    
                case AST_IMPL_BLOCK:
                    if (!code_generate_impl_block(generator, decl)) {
                        return false;
                    }
                    break;
                    
                default:
                    // Skip other declaration types
                    break;
            }
        }
    }
    
    atomic_fetch_add(&generator->stats.basic_blocks_generated, 1);
    return true;
}

bool code_generate_function(CodeGenerator *generator, ASTNode *function_decl) {
    if (!generator || !function_decl || function_decl->type != AST_FUNCTION_DECL) {
        return false;
    }
    
    const char *func_name = function_decl->data.function_decl.name;
    if (!func_name) return false;
    
    // Set current function context
    free(generator->current_function_name);
    generator->current_function_name = strdup(func_name);
    
    // Create new symbol table for this function
    LocalSymbolTable *prev_table = generator->symbol_table;
    generator->symbol_table = local_symbol_table_create(16);
    if (!generator->symbol_table) {
        generator->symbol_table = prev_table;
        return false;
    }
    
    // Create function label
    char *func_label = label_manager_create_label(generator->label_manager, 
                                                 LABEL_FUNCTION, func_name);
    if (!func_label) {
        local_symbol_table_destroy(generator->symbol_table);
        generator->symbol_table = prev_table;
        return false;
    }
    
    // Emit export directive for public functions
    if (function_decl->data.function_decl.visibility == VISIBILITY_PUBLIC) {
        printf("DEBUG: Function %s is PUBLIC, emitting .global directive\n", func_name);
        // Create a comment instruction to represent the export directive
        AssemblyInstruction *export_inst = create_instruction(INST_MOV, 0); // Use MOV as placeholder
        if (export_inst) {
            char export_directive[256];
            snprintf(export_directive, sizeof(export_directive), ".global %s", func_name);
            export_inst->comment = strdup(export_directive);
            instruction_buffer_add(generator->instruction_buffer, export_inst);
        }
    } else {
        printf("DEBUG: Function %s is PRIVATE (visibility=%d)\n", func_name, function_decl->data.function_decl.visibility);
    }
    
    // Generate function prologue
    Register saved_regs[] = {REG_RBP, REG_RBX, REG_R12, REG_R13, REG_R14, REG_R15};
    size_t saved_count = sizeof(saved_regs) / sizeof(Register);
    
    if (!generate_function_prologue(generator, DEFAULT_STACK_FRAME_SIZE, saved_regs, saved_count)) {
        local_symbol_table_destroy(generator->symbol_table);
        generator->symbol_table = prev_table;
        free(func_label);
        return false;
    }
    
    // Add function parameters to local symbol table
    ASTNodeList *params = function_decl->data.function_decl.params;
    if (params) {
        Register param_regs[] = {REG_RDI, REG_RSI, REG_RDX, REG_RCX, REG_R8, REG_R9};
        size_t param_count = ast_node_list_size(params);
        
        // Set the parameter size based on parameter count
        generator->current_function_param_size = param_count * 8; // Assume 8 bytes per parameter
        
        for (size_t i = 0; i < param_count && i < 6; i++) { // First 6 params in registers
            ASTNode *param = ast_node_list_get(params, i);
            if (param && param->type == AST_PARAM_DECL) {
                const char *param_name = param->data.param_decl.name;
                if (param_name) {
                    // Get parameter type from semantic analyzer
                    TypeDescriptor *param_type = NULL;
                    if (generator->semantic_analyzer) {
                        // Look up the function symbol to get parameter types
                        SymbolEntry *func_sym = symbol_table_lookup_safe(
                            generator->semantic_analyzer->global_scope, func_name);
                        if (func_sym && func_sym->type && 
                            func_sym->type->category == TYPE_FUNCTION &&
                            i < func_sym->type->data.function.param_count) {
                            param_type = func_sym->type->data.function.param_types[i];
                        }
                    }
                    
                    // Add parameter as local variable
                    if (!add_local_variable(generator, param_name, param_type, 8)) {
                        code_generator_report_error(generator, CODEGEN_ERROR_NONE,
                            "Failed to add parameter '%s' to symbol table", param_name);
                    }
                    
                    // Store parameter from register to stack
                    LocalVariable *var = find_local_variable(generator, param_name);
                    if (var) {
                        var->is_parameter = true;
                        // Generate store instruction to save parameter to local variable slot
                        AssemblyInstruction *store = create_store_local(param_regs[i], var->offset);
                        if (store) {
                            instruction_buffer_add(generator->instruction_buffer, store);
                        }
                    }
                }
            }
        }
        
        // TODO: Handle parameters passed on stack (beyond the first 6)
    }
    
    // Generate function body
    ASTNode *body = function_decl->data.function_decl.body;
    if (body && !code_generate_statement(generator, body)) {
        local_symbol_table_destroy(generator->symbol_table);
        generator->symbol_table = prev_table;
        free(func_label);
        return false;
    }
    
    // Generate function epilogue
    if (!generate_function_epilogue(generator, DEFAULT_STACK_FRAME_SIZE, saved_regs, saved_count)) {
        local_symbol_table_destroy(generator->symbol_table);
        generator->symbol_table = prev_table;
        free(func_label);
        return false;
    }
    
    // Restore previous symbol table
    local_symbol_table_destroy(generator->symbol_table);
    generator->symbol_table = prev_table;
    
    atomic_fetch_add(&generator->stats.functions_generated, 1);
    free(func_label);
    return true;
} 
