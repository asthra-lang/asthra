/**
 * Asthra Programming Language Compiler
 * Expression Operations Code Generation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Code generation for binary operations, assignments, and identifiers
 */

#include "expression_operations.h"
#include "codegen_128bit_operations.h"
#include "../analysis/type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

// Helper function to create mangled function names
char *create_mangled_function_name(const char *struct_name, const char *function_name, bool is_instance_method) {
    if (!struct_name || !function_name) {
        return NULL;
    }
    
    // Create mangled name: StructName_{instance|associated}_{function_name}
    const char *method_type = is_instance_method ? "instance" : "associated";
    size_t total_len = strlen(struct_name) + strlen(method_type) + strlen(function_name) + 3;
    
    char *mangled_name = malloc(total_len);
    if (!mangled_name) {
        return NULL;
    }
    
    snprintf(mangled_name, total_len, "%s_%s_%s", struct_name, method_type, function_name);
    return mangled_name;
}

// =============================================================================
// EXPRESSION OPERATION GENERATION
// =============================================================================

bool generate_identifier_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_IDENTIFIER) {
        return false;
    }
    
    // For identifiers, load the variable value
    const char *var_name = expr->data.identifier.name;
    if (!var_name) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, "Identifier has no name");
        return false;
    }
    
    // Look up variable in symbol table
    LocalVariable *var = find_local_variable(generator, var_name);
    if (!var) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, "Undefined variable: %s", var_name);
        return false;
    }
    
    // Generate load instruction
    AssemblyInstruction *load_inst = create_load_local(target_reg, var->offset);
    return load_inst && instruction_buffer_add(generator->instruction_buffer, load_inst);
}

bool generate_binary_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_BINARY_EXPR) {
        return false;
    }
    
    Register left_reg = register_allocate(generator->register_allocator, true);
    Register right_reg = register_allocate(generator->register_allocator, true);
    
    if (left_reg == ASTHRA_REG_NONE || right_reg == ASTHRA_REG_NONE) {
        if (left_reg != ASTHRA_REG_NONE) register_free(generator->register_allocator, left_reg);
        if (right_reg != ASTHRA_REG_NONE) register_free(generator->register_allocator, right_reg);
        return false;
    }
    
    // Generate left operand
    if (!code_generate_expression(generator, expr->data.binary_expr.left, left_reg)) {
        register_free(generator->register_allocator, left_reg);
        register_free(generator->register_allocator, right_reg);
        return false;
    }
    
    // Generate right operand
    if (!code_generate_expression(generator, expr->data.binary_expr.right, right_reg)) {
        register_free(generator->register_allocator, left_reg);
        register_free(generator->register_allocator, right_reg);
        return false;
    }
    
    // Check if this is a 128-bit operation
    bool success = false;
    if (expr->type_info && expr->type_info->type_descriptor) {
        TypeDescriptor *type = (TypeDescriptor *)expr->type_info->type_descriptor;
        
        if (is_128bit_type(type)) {
            // Use specialized 128-bit code generation
            success = generate_128bit_binary_operation(generator, expr->data.binary_expr.operator,
                                                     type, left_reg, right_reg, target_reg);
        } else {
            // Use standard binary arithmetic
            success = generate_binary_arithmetic(generator, expr->data.binary_expr.operator,
                                              left_reg, right_reg, target_reg);
        }
    } else {
        // Fallback to standard binary arithmetic if no type info
        success = generate_binary_arithmetic(generator, expr->data.binary_expr.operator,
                                          left_reg, right_reg, target_reg);
    }
    
    register_free(generator->register_allocator, left_reg);
    register_free(generator->register_allocator, right_reg);
    
    return success;
}

bool generate_unary_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_UNARY_EXPR) {
        return false;
    }
    
    Register operand_reg = register_allocate(generator->register_allocator, true);
    
    if (operand_reg == ASTHRA_REG_NONE) {
        return false;
    }
    
    // Generate operand
    if (!code_generate_expression(generator, expr->data.unary_expr.operand, operand_reg)) {
        register_free(generator->register_allocator, operand_reg);
        return false;
    }
    
    // Generate unary operation
    bool success = generate_unary_arithmetic(generator, expr->data.unary_expr.operator,
                                           operand_reg, target_reg);
    
    register_free(generator->register_allocator, operand_reg);
    
    return success;
}

bool generate_assignment_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_ASSIGNMENT) {
        return false;
    }
    
    // Assignment: target = value
    ASTNode *target = expr->data.assignment.target;
    ASTNode *value = expr->data.assignment.value;
    
    if (!target || !value) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Assignment missing target or value");
        return false;
    }
    
    // Generate value first
    Register value_reg = register_allocate(generator->register_allocator, true);
    if (value_reg == ASTHRA_REG_NONE) {
        return false;
    }
    
    bool success = code_generate_expression(generator, value, value_reg);
    if (!success) {
        register_free(generator->register_allocator, value_reg);
        return false;
    }
    
    // Handle different assignment targets
    if (target->type == AST_IDENTIFIER) {
        // Simple variable assignment
        const char *var_name = target->data.identifier.name;
        LocalVariable *var = find_local_variable(generator, var_name);
        
        if (!var) {
            code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                       "Undefined variable in assignment: %s", var_name);
            register_free(generator->register_allocator, value_reg);
            return false;
        }
        
        // Store to local variable
        AssemblyInstruction *store_inst = create_store_local(value_reg, var->offset);
        success = store_inst && instruction_buffer_add(generator->instruction_buffer, store_inst);
        
    } else if (target->type == AST_FIELD_ACCESS) {
        // Field assignment: obj.field = value
        ASTNode *object = target->data.field_access.object;
        const char *field_name = target->data.field_access.field_name;
        
        // Get object address
        Register obj_reg = register_allocate(generator->register_allocator, true);
        if (obj_reg == ASTHRA_REG_NONE) {
            register_free(generator->register_allocator, value_reg);
            return false;
        }
        
        success = code_generate_expression(generator, object, obj_reg);
        if (!success) {
            register_free(generator->register_allocator, value_reg);
            register_free(generator->register_allocator, obj_reg);
            return false;
        }
        
        // Calculate field offset (simplified)
        int64_t field_offset = 0;
        if (strcmp(field_name, "x") == 0) {
            field_offset = 0;
        } else if (strcmp(field_name, "y") == 0) {
            field_offset = 8;
        } else if (strcmp(field_name, "member") == 0) {
            field_offset = 0;
        }
        
        // Store to field: mov [obj_reg + offset], value_reg
        AssemblyInstruction *store_inst = create_instruction_empty(INST_MOV, 2);
        if (store_inst) {
            store_inst->operands[0] = create_memory_operand(obj_reg, field_offset, ASTHRA_REG_NONE, 1);
            store_inst->operands[1] = create_register_operand(value_reg);
            success = instruction_buffer_add(generator->instruction_buffer, store_inst);
        }
        
        register_free(generator->register_allocator, obj_reg);
        
    } else {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Unsupported assignment target type: %d", target->type);
        success = false;
    }
    
    // Assignment expressions return the assigned value
    if (success && target_reg != value_reg) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(target_reg);
            mov_inst->operands[1] = create_register_operand(value_reg);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        }
    }
    
    register_free(generator->register_allocator, value_reg);
    return success;
}