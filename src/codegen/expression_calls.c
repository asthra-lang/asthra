/**
 * Asthra Programming Language Compiler
 * Expression Function Calls Code Generation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Code generation for function calls, method calls, and enum variant construction
 */

#include "expression_calls.h"
#include "expression_operations.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// ASSOCIATED FUNCTION CALL CODE GENERATION
// =============================================================================

bool code_generate_associated_function_call(CodeGenerator *generator, ASTNode *call_expr, Register target_reg) {
    if (!generator || !call_expr || call_expr->type != AST_ASSOCIATED_FUNC_CALL) {
        return false;
    }
    
    const char *struct_name = call_expr->data.associated_func_call.struct_name;
    const char *function_name = call_expr->data.associated_func_call.function_name;
    ASTNodeList *args = call_expr->data.associated_func_call.args;
    
    if (!struct_name || !function_name) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Associated function call missing struct or function name");
        return false;
    }
    
    // Create mangled function name (associated functions are static methods)
    char *mangled_name = create_mangled_function_name(struct_name, function_name, false /* is_instance_method */);
    if (!mangled_name) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Failed to create mangled function name");
        return false;
    }
    
    // Generate arguments
    Register *arg_regs = NULL;
    size_t arg_count = 0;
    
    if (args) {
        arg_count = ast_node_list_size(args);
        arg_regs = malloc(arg_count * sizeof(Register));
        if (!arg_regs) {
            free(mangled_name);
            return false;
        }
        
        // Generate each argument
        for (size_t i = 0; i < arg_count; i++) {
            ASTNode *arg = ast_node_list_get(args, i);
            arg_regs[i] = register_allocate(generator->register_allocator, true);
            
            if (arg_regs[i] == ASTHRA_REG_NONE || !code_generate_expression(generator, arg, arg_regs[i])) {
                // Free allocated registers on error
                for (size_t j = 0; j < i; j++) {
                    register_free(generator->register_allocator, arg_regs[j]);
                }
                free(arg_regs);
                free(mangled_name);
                return false;
            }
        }
    }
    
    // Generate function call
    AssemblyInstruction *call_inst = create_call_instruction(mangled_name);
    bool success = call_inst && instruction_buffer_add(generator->instruction_buffer, call_inst);
    
    // Free argument registers
    if (arg_regs) {
        for (size_t i = 0; i < arg_count; i++) {
            register_free(generator->register_allocator, arg_regs[i]);
        }
        free(arg_regs);
    }
    
    free(mangled_name);
    return success;
}

// =============================================================================
// REGULAR FUNCTION CALL CODE GENERATION
// =============================================================================

bool code_generate_function_call(CodeGenerator *generator, ASTNode *call_expr, Register target_reg) {
    if (!generator || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }
    
    ASTNode *function_expr = call_expr->data.call_expr.function;
    ASTNodeList *args = call_expr->data.call_expr.args;
    
    if (!function_expr) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Function call missing function expression");
        return false;
    }
    
    // Handle different types of function expressions
    const char *function_name = NULL;
    bool is_method_call = false;
    char *instance_reg_name = NULL;
    
    if (function_expr->type == AST_IDENTIFIER) {
        // Regular function call: func_name(args)
        function_name = function_expr->data.identifier.name;
    } else if (function_expr->type == AST_FIELD_ACCESS) {
        // Instance method call: obj.method(args)
        is_method_call = true;
        
        // Get the object and method name
        ASTNode *object_expr = function_expr->data.field_access.object;
        const char *method_name = function_expr->data.field_access.field_name;
        
        if (!object_expr || !method_name) {
            code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                       "Invalid method call expression");
            return false;
        }
        
        // For now, assume the object type can be determined from semantic analysis
        // This would need to be enhanced with proper type information
        function_name = method_name;
        
        // Generate code to load the instance into a register
        // This is a simplified implementation
        Register instance_reg = register_allocate(generator->register_allocator, true);
        if (instance_reg == ASTHRA_REG_NONE || !code_generate_expression(generator, object_expr, instance_reg)) {
            code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                       "Failed to generate instance expression");
            return false;
        }
        
        // Store instance register for later use
        instance_reg_name = malloc(16);
        if (instance_reg_name) {
            snprintf(instance_reg_name, 16, "r%d", (int)instance_reg);
        }
    } else {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Unsupported function expression type: %d", function_expr->type);
        return false;
    }
    
    if (!function_name) {
        free(instance_reg_name);
        return false;
    }
    
    // Map predeclared functions to their runtime names
    const char* runtime_function_name = NULL;
    if (strcmp(function_name, "log") == 0) {
        runtime_function_name = "asthra_simple_log";
    } else if (strcmp(function_name, "panic") == 0) {
        runtime_function_name = "asthra_panic";
    } else if (strcmp(function_name, "args") == 0) {
        runtime_function_name = "asthra_runtime_get_args";
    } else if (strcmp(function_name, "infinite") == 0) {
        runtime_function_name = "asthra_infinite_iterator";
    }
    
    // Use runtime name if it's a predeclared function
    if (runtime_function_name) {
        function_name = runtime_function_name;
    }
    
    // Generate arguments
    Register *arg_regs = NULL;
    size_t arg_count = 0;
    size_t total_args = 0;
    
    if (args) {
        arg_count = ast_node_list_size(args);
    }
    
    // For instance methods, add one extra argument for 'self'
    total_args = arg_count + (is_method_call ? 1 : 0);
    
    if (total_args > 0) {
        arg_regs = malloc(total_args * sizeof(Register));
        if (!arg_regs) {
            free(instance_reg_name);
            return false;
        }
        
        size_t arg_index = 0;
        
        // For instance methods, add self as first argument
        if (is_method_call && instance_reg_name) {
            // The instance register is already allocated and contains the self object
            // We'll need to pass it as the first argument
            arg_regs[arg_index++] = register_allocate(generator->register_allocator, true);
            // Copy instance to argument register would go here
        }
        
        // Generate remaining arguments
        for (size_t i = 0; i < arg_count; i++) {
            ASTNode *arg = ast_node_list_get(args, i);
            arg_regs[arg_index] = register_allocate(generator->register_allocator, true);
            
            if (arg_regs[arg_index] == ASTHRA_REG_NONE || !code_generate_expression(generator, arg, arg_regs[arg_index])) {
                // Free allocated registers on error
                for (size_t j = 0; j < arg_index; j++) {
                    register_free(generator->register_allocator, arg_regs[j]);
                }
                free(arg_regs);
                free(instance_reg_name);
                return false;
            }
            arg_index++;
        }
    }
    
    // Generate function call
    AssemblyInstruction *call_inst = create_call_instruction(function_name);
    bool success = call_inst && instruction_buffer_add(generator->instruction_buffer, call_inst);
    
    // Free argument registers
    if (arg_regs) {
        for (size_t i = 0; i < total_args; i++) {
            register_free(generator->register_allocator, arg_regs[i]);
        }
        free(arg_regs);
    }
    
    free(instance_reg_name);
    return success;
}

// =============================================================================
// ENUM VARIANT CONSTRUCTION CODE GENERATION
// =============================================================================

bool code_generate_enum_variant_construction(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_ENUM_VARIANT || target_reg == ASTHRA_REG_NONE) {
        return false;
    }
    
    const char *enum_name = expr->data.enum_variant.enum_name;
    const char *variant_name = expr->data.enum_variant.variant_name;
    ASTNode *value = expr->data.enum_variant.value;
    
    if (!enum_name || !variant_name) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Enum variant missing enum or variant name");
        return false;
    }
    
    // Generate constructor function name: EnumName_VariantName_new
    size_t name_len = strlen(enum_name) + strlen(variant_name) + 10; // "_" + "_new" + null
    char *constructor_name = malloc(name_len);
    if (!constructor_name) {
        return false;
    }
    snprintf(constructor_name, name_len, "%s_%s_new", enum_name, variant_name);
    
    // Handle optional constructor value
    Register *arg_regs = NULL;
    size_t arg_count = 0;
    
    if (value) {
        arg_count = 1;
        arg_regs = malloc(sizeof(Register));
        if (!arg_regs) {
            free(constructor_name);
            return false;
        }
        
        arg_regs[0] = register_allocate(generator->register_allocator, true);
        if (arg_regs[0] == ASTHRA_REG_NONE) {
            free(arg_regs);
            free(constructor_name);
            return false;
        }
        
        // Generate argument value
        if (!code_generate_expression(generator, value, arg_regs[0])) {
            register_free(generator->register_allocator, arg_regs[0]);
            free(arg_regs);
            free(constructor_name);
            return false;
        }
    }
    
    // Generate constructor function call
    AssemblyInstruction *call_inst = create_call_instruction(constructor_name);
    bool success = call_inst && instruction_buffer_add(generator->instruction_buffer, call_inst);
    
    // Move result to target register if needed
    if (success && target_reg != ASTHRA_REG_RAX) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(target_reg);
            mov_inst->operands[1] = create_register_operand(ASTHRA_REG_RAX);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        }
    }
    
    // Cleanup
    if (arg_regs) {
        for (size_t i = 0; i < arg_count; i++) {
            register_free(generator->register_allocator, arg_regs[i]);
        }
        free(arg_regs);
    }
    free(constructor_name);
    
    return success;
}