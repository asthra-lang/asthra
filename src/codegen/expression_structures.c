/**
 * Asthra Programming Language Compiler
 * Code Generator - Structure and Array Expressions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Handles code generation for struct literals, field access, and array operations
 */

#include "expression_structures.h"
#include "code_generator.h"
#include "../analysis/type_info.h"
#include "../analysis/semantic_core.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// STRUCT EXPRESSION GENERATION
// =============================================================================

bool generate_struct_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_STRUCT_LITERAL) {
        return false;
    }
    
    // Struct literal: StructName { field1: value1, field2: value2, ... }
    const char *struct_name = expr->data.struct_literal.struct_name;
    ASTNodeList *fields = expr->data.struct_literal.field_inits;
    
    // Get struct type information from the expression
    TypeDescriptor *struct_type = semantic_get_expression_type(generator->semantic_analyzer, expr);
    if (!struct_type || struct_type->category != TYPE_STRUCT) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Missing or invalid struct type information for %s", 
                                   struct_name ? struct_name : "unknown");
        return false;
    }
    
    size_t struct_size = struct_type->size;
    
    // Allocate space on stack for struct
    AssemblyInstruction *sub_inst = create_instruction_empty(INST_SUB, 2);
    if (!sub_inst) {
        type_descriptor_release(struct_type);
        return false;
    }
    
    sub_inst->operands[0] = create_register_operand(REG_RSP);
    sub_inst->operands[1] = create_immediate_operand(struct_size);
    
    bool success = instruction_buffer_add(generator->instruction_buffer, sub_inst);
    
    // Store struct address in target register
    if (success) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(target_reg);
            mov_inst->operands[1] = create_register_operand(REG_RSP);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        }
    }
    
    // Initialize fields
    if (success && fields) {
        Register field_reg = register_allocate(generator->register_allocator, true);
        if (field_reg != REG_NONE) {
            for (size_t i = 0; i < ast_node_list_size(fields) && success; i++) {
                ASTNode *field_init = ast_node_list_get(fields, i);
                if (field_init && field_init->type == AST_ASSIGNMENT) {
                    // Field initialization is stored as assignment: field_name: value
                    const char *field_name = NULL;
                    if (field_init->data.assignment.target && 
                        field_init->data.assignment.target->type == AST_IDENTIFIER) {
                        field_name = field_init->data.assignment.target->data.identifier.name;
                    }
                    ASTNode *field_value = field_init->data.assignment.value;
                    
                    // Get field offset from struct type
                    // In a complete implementation, this would use the symbol table
                    size_t field_offset = i * 8; // Simplified: assume 8 bytes per field
                    
                    // Generate field value
                    success = code_generate_expression(generator, field_value, field_reg);
                    if (success) {
                        // Store field: mov [rsp + offset], field_reg
                        AssemblyInstruction *store_inst = create_instruction_empty(INST_MOV, 2);
                        if (store_inst) {
                            store_inst->operands[0] = create_memory_operand(REG_RSP, field_offset, REG_NONE, 1);
                            store_inst->operands[1] = create_register_operand(field_reg);
                            success = instruction_buffer_add(generator->instruction_buffer, store_inst);
                        }
                    }
                }
            }
            register_free(generator->register_allocator, field_reg);
        }
    }
    
    type_descriptor_release(struct_type);
    return success;
}

bool generate_field_access(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_FIELD_ACCESS) {
        return false;
    }
    
    // Field access: object.field
    ASTNode *object = expr->data.field_access.object;
    const char *field_name = expr->data.field_access.field_name;
    
    if (!object || !field_name) {
        return false;
    }
    
    // Generate the object address
    Register obj_reg = register_allocate(generator->register_allocator, true);
    if (obj_reg == REG_NONE) {
        return false;
    }
    
    bool success = code_generate_expression(generator, object, obj_reg);
    
    if (success) {
        // Get object type to find field offset
        TypeDescriptor *obj_type = semantic_get_expression_type(generator->semantic_analyzer, object);
        if (obj_type && obj_type->category == TYPE_STRUCT) {
            // In a complete implementation, this would look up the field in the struct's symbol table
            // For now, use a simplified offset calculation
            size_t field_offset = 0; // Would be calculated from field position
            
            // Load field value: mov target_reg, [obj_reg + offset]
            AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
            if (mov_inst) {
                mov_inst->operands[0] = create_register_operand(target_reg);
                mov_inst->operands[1] = create_memory_operand(obj_reg, field_offset, REG_NONE, 1);
                success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
            }
            
            type_descriptor_release(obj_type);
        } else {
            code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                       "Field access on non-struct type");
            success = false;
        }
    }
    
    register_free(generator->register_allocator, obj_reg);
    return success;
}

// =============================================================================
// ARRAY EXPRESSION GENERATION
// =============================================================================

bool generate_array_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_ARRAY_LITERAL) {
        return false;
    }
    
    // Array literal: [elem1, elem2, ...] or [value; count]
    ASTNodeList *elements = expr->data.array_literal.elements;
    
    if (!elements) {
        return false;
    }
    
    size_t elem_count = ast_node_list_size(elements);
    
    // Check for repeated array syntax [value; count]
    if (elem_count >= 3) {
        ASTNode *first = ast_node_list_get(elements, 0);
        if (first && first->type == AST_IDENTIFIER && 
            first->data.identifier.name &&
            strcmp(first->data.identifier.name, "__repeated_array__") == 0) {
            
            // This is a repeated array literal
            if (elem_count != 3) {
                code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                           "Invalid repeated array syntax");
                return false;
            }
            
            // Get value and count expressions
            ASTNode *value_expr = ast_node_list_get(elements, 1);
            ASTNode *count_expr = ast_node_list_get(elements, 2);
            
            // Get the array type to determine size
            TypeDescriptor *array_type = semantic_get_expression_type(generator->semantic_analyzer, expr);
            if (!array_type || array_type->category != TYPE_ARRAY) {
                code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                           "Missing or invalid array type information");
                return false;
            }
            
            size_t array_size = array_type->data.array.size;
            size_t element_size = array_type->data.array.element_type->size;
            size_t total_size = array_size * element_size;
            
            // Allocate space on stack for array
            AssemblyInstruction *sub_inst = create_instruction_empty(INST_SUB, 2);
            if (!sub_inst) {
                type_descriptor_release(array_type);
                return false;
            }
            
            sub_inst->operands[0] = create_register_operand(REG_RSP);
            sub_inst->operands[1] = create_immediate_operand(total_size);
            
            bool success = instruction_buffer_add(generator->instruction_buffer, sub_inst);
            
            // Store array address in target register
            if (success) {
                AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
                if (mov_inst) {
                    mov_inst->operands[0] = create_register_operand(target_reg);
                    mov_inst->operands[1] = create_register_operand(REG_RSP);
                    success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
                }
            }
            
            // Generate the value expression once
            Register value_reg = register_allocate(generator->register_allocator, true);
            if (value_reg != REG_NONE && success) {
                success = code_generate_expression(generator, value_expr, value_reg);
                
                // Initialize all elements with the same value
                if (success) {
                    for (size_t i = 0; i < array_size && success; i++) {
                        // Store element: mov [rsp + i*element_size], value_reg
                        AssemblyInstruction *store_inst = create_instruction_empty(INST_MOV, 2);
                        if (store_inst) {
                            store_inst->operands[0] = create_memory_operand(REG_RSP, i * element_size, REG_NONE, 1);
                            store_inst->operands[1] = create_register_operand(value_reg);
                            success = instruction_buffer_add(generator->instruction_buffer, store_inst);
                        } else {
                            success = false;
                        }
                    }
                }
                
                register_free(generator->register_allocator, value_reg);
            }
            
            type_descriptor_release(array_type);
            return success;
        }
    }
    
    // Regular array literal [elem1, elem2, ...]
    
    // Allocate space on stack for array (simplified)
    // Real implementation would use proper memory allocation
    AssemblyInstruction *sub_inst = create_instruction_empty(INST_SUB, 2);
    if (!sub_inst) {
        return false;
    }
    
    sub_inst->operands[0] = create_register_operand(REG_RSP);
    sub_inst->operands[1] = create_immediate_operand(elem_count * 8); // 8 bytes per element
    
    bool success = instruction_buffer_add(generator->instruction_buffer, sub_inst);
    
    // Store array address in target register
    if (success) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(target_reg);
            mov_inst->operands[1] = create_register_operand(REG_RSP);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        }
    }
    
    // Initialize elements (simplified)
    if (success && elements) {
        Register elem_reg = register_allocate(generator->register_allocator, true);
        if (elem_reg != REG_NONE) {
            for (size_t i = 0; i < elem_count && success; i++) {
                ASTNode *elem = ast_node_list_get(elements, i);
                if (elem) {
                    success = code_generate_expression(generator, elem, elem_reg);
                    if (success) {
                        // Store element: mov [rsp + i*8], elem_reg
                        AssemblyInstruction *store_inst = create_instruction_empty(INST_MOV, 2);
                        if (store_inst) {
                            store_inst->operands[0] = create_memory_operand(REG_RSP, i * 8, REG_NONE, 1);
                            store_inst->operands[1] = create_register_operand(elem_reg);
                            success = instruction_buffer_add(generator->instruction_buffer, store_inst);
                        }
                    }
                }
            }
            register_free(generator->register_allocator, elem_reg);
        }
    }
    
    return success;
}

bool generate_tuple_literal(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_TUPLE_LITERAL) {
        return false;
    }
    
    // Tuple literal: (elem1, elem2, ...)
    ASTNodeList *elements = expr->data.tuple_literal.elements;
    
    if (!elements || ast_node_list_size(elements) < 2) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Tuple literals must have at least 2 elements");
        return false;
    }
    
    // Get tuple type information from the expression
    TypeDescriptor *tuple_type = semantic_get_expression_type(generator->semantic_analyzer, expr);
    if (!tuple_type || tuple_type->category != TYPE_TUPLE) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Missing or invalid tuple type information");
        return false;
    }
    
    size_t tuple_size = tuple_type->size;
    size_t elem_count = tuple_type->data.tuple.element_count;
    
    // Allocate space on stack for tuple
    AssemblyInstruction *sub_inst = create_instruction_empty(INST_SUB, 2);
    if (!sub_inst) {
        type_descriptor_release(tuple_type);
        return false;
    }
    
    sub_inst->operands[0] = create_register_operand(REG_RSP);
    sub_inst->operands[1] = create_immediate_operand(tuple_size);
    
    bool success = instruction_buffer_add(generator->instruction_buffer, sub_inst);
    
    // Store tuple address in target register
    if (success) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(target_reg);
            mov_inst->operands[1] = create_register_operand(REG_RSP);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        }
    }
    
    // Initialize elements at their proper offsets
    if (success) {
        Register elem_reg = register_allocate(generator->register_allocator, true);
        if (elem_reg != REG_NONE) {
            for (size_t i = 0; i < elem_count && success; i++) {
                ASTNode *elem = ast_node_list_get(elements, i);
                if (elem) {
                    success = code_generate_expression(generator, elem, elem_reg);
                    if (success) {
                        // Get element offset from tuple type
                        size_t elem_offset = tuple_type->data.tuple.element_offsets[i];
                        TypeDescriptor *elem_type = tuple_type->data.tuple.element_types[i];
                        
                        // Store element at proper offset: mov [rsp + offset], elem_reg
                        AssemblyInstruction *store_inst = create_instruction_empty(INST_MOV, 2);
                        if (store_inst) {
                            store_inst->operands[0] = create_memory_operand(REG_RSP, elem_offset, REG_NONE, 1);
                            store_inst->operands[1] = create_register_operand(elem_reg);
                            
                            // Note: In a complete implementation, we would set operand size
                            // based on element type for correct memory access width
                            
                            success = instruction_buffer_add(generator->instruction_buffer, store_inst);
                        }
                    }
                }
            }
            register_free(generator->register_allocator, elem_reg);
        }
    }
    
    return success;
}

// =============================================================================
// INDEX ACCESS GENERATION
// =============================================================================

bool generate_index_access(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_INDEX_ACCESS) {
        return false;
    }
    
    ASTNode *array = expr->data.index_access.array;
    ASTNode *index = expr->data.index_access.index;
    
    if (!array || !index) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Index access missing array or index");
        return false;
    }
    
    // Get registers for array address and index
    Register array_reg = register_allocate(generator->register_allocator, true);
    Register index_reg = register_allocate(generator->register_allocator, true);
    
    if (array_reg == REG_NONE || index_reg == REG_NONE) {
        if (array_reg != REG_NONE) register_free(generator->register_allocator, array_reg);
        if (index_reg != REG_NONE) register_free(generator->register_allocator, index_reg);
        return false;
    }
    
    // Generate array address
    bool success = code_generate_expression(generator, array, array_reg);
    
    // Generate index value
    if (success) {
        success = code_generate_expression(generator, index, index_reg);
    }
    
    // Calculate element address: array_reg + (index_reg * element_size)
    if (success) {
        // Get element size from type info
        TypeDescriptor *array_type = semantic_get_expression_type(generator->semantic_analyzer, array);
        size_t element_size = 8; // Default to 8 bytes
        
        if (array_type) {
            if (array_type->category == TYPE_SLICE && array_type->data.slice.element_type) {
                element_size = array_type->data.slice.element_type->size;
            } else if (array_type->category == TYPE_ARRAY && array_type->data.array.element_type) {
                element_size = array_type->data.array.element_type->size;
            }
            type_descriptor_release(array_type);
        }
        
        // Scale index by element size if needed
        if (element_size != 1) {
            AssemblyInstruction *imul_inst = create_instruction_empty(INST_IMUL, 3);
            if (imul_inst) {
                imul_inst->operands[0] = create_register_operand(index_reg);
                imul_inst->operands[1] = create_register_operand(index_reg);
                imul_inst->operands[2] = create_immediate_operand(element_size);
                success = instruction_buffer_add(generator->instruction_buffer, imul_inst);
            } else {
                success = false;
            }
        }
        
        // Add base and scaled index: array_reg + index_reg
        if (success) {
            AssemblyInstruction *add_inst = create_instruction_empty(INST_ADD, 2);
            if (add_inst) {
                add_inst->operands[0] = create_register_operand(array_reg);
                add_inst->operands[1] = create_register_operand(index_reg);
                success = instruction_buffer_add(generator->instruction_buffer, add_inst);
            } else {
                success = false;
            }
        }
        
        // Load value from calculated address
        if (success) {
            AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
            if (mov_inst) {
                mov_inst->operands[0] = create_register_operand(target_reg);
                mov_inst->operands[1] = create_memory_operand(array_reg, 0, REG_NONE, 1);
                success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
            } else {
                success = false;
            }
        }
    }
    
    register_free(generator->register_allocator, array_reg);
    register_free(generator->register_allocator, index_reg);
    
    return success;
}

// =============================================================================
// SLICE EXPRESSION GENERATION
// =============================================================================

bool generate_slice_expr(CodeGenerator *generator, ASTNode *expr, Register target_reg) {
    if (!generator || !expr || expr->type != AST_SLICE_EXPR) {
        return false;
    }
    
    ASTNode *array = expr->data.slice_expr.array;
    ASTNode *start = expr->data.slice_expr.start;
    ASTNode *end = expr->data.slice_expr.end;
    
    if (!array) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Slice expression missing array");
        return false;
    }
    
    // Get array type information
    TypeDescriptor *array_type = semantic_get_expression_type(generator->semantic_analyzer, array);
    if (!array_type) {
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Cannot determine array type for slice operation");
        return false;
    }
    
    size_t element_size = 8; // Default
    size_t array_length = 0;
    
    if (array_type->category == TYPE_SLICE) {
        if (array_type->data.slice.element_type) {
            element_size = array_type->data.slice.element_type->size;
        }
        // For slices, we'll need to load the length from the slice descriptor
    } else if (array_type->category == TYPE_ARRAY) {
        element_size = array_type->data.array.element_type->size;
        array_length = array_type->data.array.size;
    } else {
        type_descriptor_release(array_type);
        code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION, 
                                   "Cannot slice non-array/non-slice type");
        return false;
    }
    
    // Allocate registers
    Register array_reg = register_allocate(generator->register_allocator, true);
    Register start_reg = register_allocate(generator->register_allocator, true);
    Register end_reg = register_allocate(generator->register_allocator, true);
    Register temp_reg = register_allocate(generator->register_allocator, true);
    
    if (array_reg == REG_NONE || start_reg == REG_NONE || end_reg == REG_NONE || temp_reg == REG_NONE) {
        if (array_reg != REG_NONE) register_free(generator->register_allocator, array_reg);
        if (start_reg != REG_NONE) register_free(generator->register_allocator, start_reg);
        if (end_reg != REG_NONE) register_free(generator->register_allocator, end_reg);
        if (temp_reg != REG_NONE) register_free(generator->register_allocator, temp_reg);
        type_descriptor_release(array_type);
        return false;
    }
    
    bool success = true;
    
    // 1. Generate array address
    success = code_generate_expression(generator, array, array_reg);
    
    // 2. Generate or default start index
    if (success) {
        if (start) {
            success = code_generate_expression(generator, start, start_reg);
        } else {
            // Default start to 0
            AssemblyInstruction *xor_inst = create_instruction_empty(INST_XOR, 2);
            if (xor_inst) {
                xor_inst->operands[0] = create_register_operand(start_reg);
                xor_inst->operands[1] = create_register_operand(start_reg);
                success = instruction_buffer_add(generator->instruction_buffer, xor_inst);
            } else {
                success = false;
            }
        }
    }
    
    // 3. Generate or default end index
    if (success) {
        if (end) {
            success = code_generate_expression(generator, end, end_reg);
        } else {
            // Default end to array length
            if (array_type->category == TYPE_ARRAY) {
                // For fixed arrays, we know the length at compile time
                AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
                if (mov_inst) {
                    mov_inst->operands[0] = create_register_operand(end_reg);
                    mov_inst->operands[1] = create_immediate_operand(array_length);
                    success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
                } else {
                    success = false;
                }
            } else if (array_type->category == TYPE_SLICE) {
                // For slices, load the length from offset 8 (after the pointer)
                AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
                if (mov_inst) {
                    mov_inst->operands[0] = create_register_operand(end_reg);
                    mov_inst->operands[1] = create_memory_operand(array_reg, 8, REG_NONE, 1);
                    success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
                } else {
                    success = false;
                }
            }
        }
    }
    
    // 4. Allocate space for slice descriptor (16 bytes: pointer + length)
    if (success) {
        AssemblyInstruction *sub_inst = create_instruction_empty(INST_SUB, 2);
        if (sub_inst) {
            sub_inst->operands[0] = create_register_operand(REG_RSP);
            sub_inst->operands[1] = create_immediate_operand(16);
            success = instruction_buffer_add(generator->instruction_buffer, sub_inst);
        } else {
            success = false;
        }
    }
    
    // 5. Calculate slice pointer: base + (start * element_size)
    if (success && element_size != 1) {
        // Scale start index by element size
        AssemblyInstruction *imul_inst = create_instruction_empty(INST_IMUL, 3);
        if (imul_inst) {
            imul_inst->operands[0] = create_register_operand(temp_reg);
            imul_inst->operands[1] = create_register_operand(start_reg);
            imul_inst->operands[2] = create_immediate_operand(element_size);
            success = instruction_buffer_add(generator->instruction_buffer, imul_inst);
        } else {
            success = false;
        }
        
        if (success) {
            // Use temp_reg as scaled start
            AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
            if (mov_inst) {
                mov_inst->operands[0] = create_register_operand(start_reg);
                mov_inst->operands[1] = create_register_operand(temp_reg);
                success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
            } else {
                success = false;
            }
        }
    }
    
    // Get the base pointer (for arrays, it's the array address; for slices, load from offset 0)
    if (success && array_type->category == TYPE_SLICE) {
        // Load the pointer from the slice descriptor
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(temp_reg);
            mov_inst->operands[1] = create_memory_operand(array_reg, 0, REG_NONE, 1);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        } else {
            success = false;
        }
        
        if (success) {
            // Use temp_reg as base pointer
            AssemblyInstruction *mov_inst2 = create_instruction_empty(INST_MOV, 2);
            if (mov_inst2) {
                mov_inst2->operands[0] = create_register_operand(array_reg);
                mov_inst2->operands[1] = create_register_operand(temp_reg);
                success = instruction_buffer_add(generator->instruction_buffer, mov_inst2);
            } else {
                success = false;
            }
        }
    }
    
    // Add start offset to base pointer
    if (success) {
        AssemblyInstruction *add_inst = create_instruction_empty(INST_ADD, 2);
        if (add_inst) {
            add_inst->operands[0] = create_register_operand(array_reg);
            add_inst->operands[1] = create_register_operand(start_reg);
            success = instruction_buffer_add(generator->instruction_buffer, add_inst);
        } else {
            success = false;
        }
    }
    
    // 6. Store pointer in slice descriptor at [rsp]
    if (success) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_memory_operand(REG_RSP, 0, REG_NONE, 1);
            mov_inst->operands[1] = create_register_operand(array_reg);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        } else {
            success = false;
        }
    }
    
    // 7. Calculate length: end - start
    if (success) {
        AssemblyInstruction *sub_inst = create_instruction_empty(INST_SUB, 2);
        if (sub_inst) {
            sub_inst->operands[0] = create_register_operand(end_reg);
            sub_inst->operands[1] = create_register_operand(start_reg);
            success = instruction_buffer_add(generator->instruction_buffer, sub_inst);
        } else {
            success = false;
        }
    }
    
    // 8. Store length in slice descriptor at [rsp + 8]
    if (success) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_memory_operand(REG_RSP, 8, REG_NONE, 1);
            mov_inst->operands[1] = create_register_operand(end_reg);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        } else {
            success = false;
        }
    }
    
    // 9. Return slice descriptor address in target register
    if (success) {
        AssemblyInstruction *mov_inst = create_instruction_empty(INST_MOV, 2);
        if (mov_inst) {
            mov_inst->operands[0] = create_register_operand(target_reg);
            mov_inst->operands[1] = create_register_operand(REG_RSP);
            success = instruction_buffer_add(generator->instruction_buffer, mov_inst);
        } else {
            success = false;
        }
    }
    
    // Clean up
    register_free(generator->register_allocator, array_reg);
    register_free(generator->register_allocator, start_reg);
    register_free(generator->register_allocator, end_reg);
    register_free(generator->register_allocator, temp_reg);
    type_descriptor_release(array_type);
    
    return success;
}