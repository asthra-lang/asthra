/**
 * Asthra Programming Language Compiler
 * Code Generator Statement Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "code_generator.h"
#include "../analysis/type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

// Helper functions
static const char* get_branch_suffix(int branch_num);
static bool emit_jump(CodeGenerator* generator, const char* label);
static bool emit_label(CodeGenerator* generator, const char* label);
static bool emit_jump_conditional(CodeGenerator* generator, const char* label, Register condition_reg);
static char* create_branch_label(const char* label, int branch_num);

// Forward declarations are in code_generator_core.h

// Main statement generation function
bool code_generate_statement(CodeGenerator *generator, ASTNode *stmt) {
    if (!generator || !stmt) {
        return false;
    }
    
    switch (stmt->type) {
        case AST_LET_STMT: {
            const char *var_name = stmt->data.let_stmt.name;
            ASTNode *init_expr = stmt->data.let_stmt.initializer;
            
            // Generate initializer expression - use ASTHRA_REG_NONE for target
            Register value_reg = code_generate_expression(generator, init_expr, ASTHRA_REG_NONE);
            if (value_reg == ASTHRA_REG_NONE) {
                return false;
            }
            
            // Create local variable
            LocalVariable *var = create_local_variable(generator, var_name, value_reg);
            if (!var) {
                register_free(generator->register_allocator, value_reg);
                return false;
            }
            
            // Store value to variable location
            AssemblyInstruction *store_inst = create_store_local(value_reg, var->offset);
            if (!store_inst || !instruction_buffer_add(generator->instruction_buffer, store_inst)) {
                register_free(generator->register_allocator, value_reg);
                return false;
            }
            
            register_free(generator->register_allocator, value_reg);
            return true;
        }
        
        case AST_IF_STMT: {
            ASTNode *condition = stmt->data.if_stmt.condition;
            ASTNode *then_block = stmt->data.if_stmt.then_block;
            ASTNode *else_block = stmt->data.if_stmt.else_block;
            
            // Allocate a register for the condition
            Register cond_reg = register_allocate(generator->register_allocator, true);
            if (cond_reg == ASTHRA_REG_NONE) {
                return false;
            }
            
            // Generate condition
            bool cond_success = code_generate_expression(generator, condition, cond_reg);
            if (!cond_success) {
                register_free(generator->register_allocator, cond_reg);
                return false;
            }
            
            // Create labels
            char *else_label = label_manager_create_label(generator->label_manager, LABEL_BRANCH_TARGET, ".L_else");
            char *end_label = label_manager_create_label(generator->label_manager, LABEL_BRANCH_TARGET, ".L_end");
            if (!else_label || !end_label) {
                register_free(generator->register_allocator, cond_reg);
                return false;
            }
            
            // Jump to else if condition is false
            if (!emit_jump_conditional(generator, else_label, cond_reg)) {
                register_free(generator->register_allocator, cond_reg);
                return false;
            }
            
            register_free(generator->register_allocator, cond_reg);
            
            // Generate then block
            if (!code_generate_statement(generator, then_block)) {
                return false;
            }
            
            // Jump to end
            if (!emit_jump(generator, end_label)) {
                return false;
            }
            
            // Else label
            if (!emit_label(generator, else_label)) {
                return false;
            }
            
            // Generate else block if present
            if (else_block) {
                if (!code_generate_statement(generator, else_block)) {
                    return false;
                }
            }
            
            // End label
            return emit_label(generator, end_label);
        }
        
        case AST_IF_LET_STMT: {
            return code_generate_if_let_statement(generator, stmt);
        }
        
        case AST_FOR_STMT: {
            const char *iterator_var = stmt->data.for_stmt.variable;
            ASTNode *iterable = stmt->data.for_stmt.iterable;
            ASTNode *body = stmt->data.for_stmt.body;
            
            // Create loop labels
            char *loop_start = label_manager_create_label(generator->label_manager, LABEL_LOOP_START, ".L_for_start");
            char *loop_continue = label_manager_create_label(generator->label_manager, LABEL_LOOP_START, ".L_for_continue");
            char *loop_exit = label_manager_create_label(generator->label_manager, LABEL_LOOP_END, ".L_for_exit");
            
            if (!loop_start || !loop_continue || !loop_exit) {
                return false;
            }
            
            // Save current loop context
            char *saved_exit = generator->loop_context.exit_label;
            char *saved_continue = generator->loop_context.continue_label;
            size_t saved_depth = generator->loop_context.depth;
            
            // Set new loop context
            generator->loop_context.exit_label = loop_exit;
            generator->loop_context.continue_label = loop_continue;
            generator->loop_context.depth++;
            
            // For now, implement a simple counting loop for range(n)
            // TODO: Full iterable support
            
            // Allocate registers for loop counter and limit
            Register counter_reg = register_allocate(generator->register_allocator, true);
            Register limit_reg = register_allocate(generator->register_allocator, true);
            
            if (counter_reg == ASTHRA_REG_NONE || limit_reg == ASTHRA_REG_NONE) {
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Initialize counter to 0
            AssemblyInstruction *init_counter = create_mov_immediate(counter_reg, 0);
            if (!init_counter || !instruction_buffer_add(generator->instruction_buffer, init_counter)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Generate the iterable expression (for range(n), this evaluates to n)
            // For simplicity, assume it's a call to range(n) that returns the limit
            if (iterable->type == AST_CALL_EXPR && 
                iterable->data.call_expr.function->type == AST_IDENTIFIER &&
                strcmp(iterable->data.call_expr.function->data.identifier.name, "range") == 0) {
                
                // Get the argument to range()
                ASTNodeList *args = iterable->data.call_expr.args;
                if (args && args->count > 0) {
                    ASTNode *limit_expr = args->nodes[0];
                    if (!code_generate_expression(generator, limit_expr, limit_reg)) {
                        register_free(generator->register_allocator, counter_reg);
                        register_free(generator->register_allocator, limit_reg);
                        // Restore loop context
                        generator->loop_context.exit_label = saved_exit;
                        generator->loop_context.continue_label = saved_continue;
                        generator->loop_context.depth = saved_depth;
                        return false;
                    }
                }
            } else {
                // For now, only support range() iterables
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Loop start label
            if (!emit_label(generator, loop_start)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Compare counter with limit
            AssemblyInstruction *cmp = create_cmp_instruction(counter_reg, limit_reg);
            if (!cmp || !instruction_buffer_add(generator->instruction_buffer, cmp)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Jump to exit if counter >= limit (use JGE for signed comparison)
            AssemblyInstruction *jge = create_instruction(INST_JGE, 1,
                &(AssemblyOperand){.type = OPERAND_LABEL, .data.label = loop_exit});
            if (!jge || !instruction_buffer_add(generator->instruction_buffer, jge)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Create local variable for the iterator
            LocalVariable *iter_var = create_local_variable(generator, iterator_var, counter_reg);
            if (!iter_var) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Store counter value to iterator variable
            AssemblyInstruction *store_iter = create_store_local(counter_reg, iter_var->offset);
            if (!store_iter || !instruction_buffer_add(generator->instruction_buffer, store_iter)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Generate loop body
            if (!code_generate_statement(generator, body)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Continue label (for continue statements)
            if (!emit_label(generator, loop_continue)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Increment counter
            AssemblyInstruction *inc = create_inc_instruction(counter_reg);
            if (!inc || !instruction_buffer_add(generator->instruction_buffer, inc)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Jump back to loop start
            if (!emit_jump(generator, loop_start)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Exit label
            if (!emit_label(generator, loop_exit)) {
                register_free(generator->register_allocator, counter_reg);
                register_free(generator->register_allocator, limit_reg);
                // Restore loop context
                generator->loop_context.exit_label = saved_exit;
                generator->loop_context.continue_label = saved_continue;
                generator->loop_context.depth = saved_depth;
                return false;
            }
            
            // Free registers
            register_free(generator->register_allocator, counter_reg);
            register_free(generator->register_allocator, limit_reg);
            
            // Restore loop context
            generator->loop_context.exit_label = saved_exit;
            generator->loop_context.continue_label = saved_continue;
            generator->loop_context.depth = saved_depth;
            
            return true;
        }
        
        case AST_RETURN_STMT: {
            ASTNode *return_value = stmt->data.return_stmt.expression;
            
            if (return_value) {
                // Generate return value - use ASTHRA_REG_NONE for target
                Register value_reg = code_generate_expression(generator, return_value, ASTHRA_REG_NONE);
                if (value_reg == ASTHRA_REG_NONE) {
                    return false;
                }
                
                // Move to return register if needed
                if (value_reg != ASTHRA_REG_RAX) {
                    AssemblyInstruction *mov = create_mov_instruction(ASTHRA_REG_RAX, value_reg);
                    if (!mov || !instruction_buffer_add(generator->instruction_buffer, mov)) {
                        register_free(generator->register_allocator, value_reg);
                        return false;
                    }
                }
                
                register_free(generator->register_allocator, value_reg);
            }
            
            // Generate function epilogue and return
            // For now, pass 0 for stack size and NULL for saved regs - proper values should come from function context
            if (!generate_function_epilogue(generator, 0, NULL, 0)) {
                return false;
            }
            
            AssemblyInstruction *ret = create_ret_instruction();
            return ret && instruction_buffer_add(generator->instruction_buffer, ret);
        }
        
        case AST_EXPR_STMT: {
            ASTNode *expr = stmt->data.expr_stmt.expression;
            
            // Allocate a register for the result
            Register result = register_allocate(generator->register_allocator, true);
            if (result == ASTHRA_REG_NONE) {
                return false;
            }
            
            // Generate expression
            bool success = code_generate_expression(generator, expr, result);
            if (!success) {
                register_free(generator->register_allocator, result);
                return false;
            }
            
            register_free(generator->register_allocator, result);
            return true;
        }
        
        case AST_BLOCK: {
            // Generate each statement in the block
            ASTNodeList *statements = stmt->data.block.statements;
            if (statements) {
                for (size_t i = 0; i < statements->count; i++) {
                    ASTNode *node = statements->nodes[i];
                    if (!code_generate_statement(generator, node)) {
                        return false;
                    }
                }
            }
            return true;
        }
        
        case AST_ASSIGNMENT: {
            // Assignment as a statement (not wrapped in expression statement)
            // Allocate a register for the result
            Register result = register_allocate(generator->register_allocator, true);
            if (result == ASTHRA_REG_NONE) {
                return false;
            }
            
            // Generate assignment expression
            bool success = code_generate_expression(generator, stmt, result);
            if (!success) {
                register_free(generator->register_allocator, result);
                return false;
            }
            
            register_free(generator->register_allocator, result);
            return true;
        }
        
        case AST_BREAK_STMT: {
            // Check if we're in a loop
            if (generator->loop_context.depth == 0 || !generator->loop_context.exit_label) {
                // Error: break statement outside of loop
                code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION,
                                          "break statement outside of loop");
                return false;
            }
            
            // Emit jump to the loop's exit label
            return emit_jump(generator, generator->loop_context.exit_label);
        }
        
        case AST_CONTINUE_STMT: {
            // Check if we're in a loop
            if (generator->loop_context.depth == 0 || !generator->loop_context.continue_label) {
                // Error: continue statement outside of loop
                code_generator_report_error(generator, CODEGEN_ERROR_UNSUPPORTED_OPERATION,
                                          "continue statement outside of loop");
                return false;
            }
            
            // Emit jump to the loop's continue label
            return emit_jump(generator, generator->loop_context.continue_label);
        }
        
        case AST_MATCH_STMT: {
            return code_generate_match_statement(generator, stmt);
        }
        
        default:
            // Unknown statement type
            return false;
    }
}

// Helper function implementations
static const char* get_branch_suffix(int branch_num) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "_branch_%d", branch_num);
    return buffer;
}

static bool emit_jump(CodeGenerator* generator, const char* label) {
    AssemblyInstruction* jmp = create_jmp_instruction(label);
    return jmp && instruction_buffer_add(generator->instruction_buffer, jmp);
}

static bool emit_label(CodeGenerator* generator, const char* label) {
    // Labels are handled differently - they're not instructions but markers
    // For now, just define the label in the label manager
    return label_manager_define_label(generator->label_manager, label, 
                                    generator->instruction_buffer->count);
}

static bool emit_jump_conditional(CodeGenerator* generator, const char* label, Register condition_reg) {
    // Test condition (test reg, reg sets ZF if reg is zero)
    AssemblyInstruction* test = create_instruction(INST_TEST, 2,
        &(AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = condition_reg},
        &(AssemblyOperand){.type = OPERAND_REGISTER, .data.reg = condition_reg});
    if (!test || !instruction_buffer_add(generator->instruction_buffer, test)) {
        return false;
    }
    
    // Jump if zero (false) - JE jumps if ZF=1
    AssemblyInstruction* je = create_je_instruction(label);
    return je && instruction_buffer_add(generator->instruction_buffer, je);
}

static char* create_branch_label(const char* label, int branch_num) {
    size_t len = strlen(label) + 32;
    char* new_label = malloc(len);
    if (!new_label) return NULL;
    
    snprintf(new_label, len, "%s%s", label, get_branch_suffix(branch_num));
    return new_label;
}

/**
 * Generate if-let statement with pattern matching
 */
bool code_generate_if_let_statement(CodeGenerator *generator, ASTNode *stmt) {
    if (!generator || !stmt || stmt->type != AST_IF_LET_STMT) {
        return false;
    }
    
    ASTNode *pattern = stmt->data.if_let_stmt.pattern;
    ASTNode *expr = stmt->data.if_let_stmt.expression;
    ASTNode *then_block = stmt->data.if_let_stmt.then_block;
    ASTNode *else_block = stmt->data.if_let_stmt.else_block;
    
    // Generate the expression to match against
    Register value_reg = code_generate_expression(generator, expr, ASTHRA_REG_NONE);
    if (value_reg == ASTHRA_REG_NONE) {
        return false;
    }
    
    // Create labels
    char *else_label = label_manager_create_label(generator->label_manager, LABEL_BRANCH_TARGET, ".L_if_let_else");
    char *end_label = label_manager_create_label(generator->label_manager, LABEL_BRANCH_TARGET, ".L_if_let_end");
    if (!else_label || !end_label) {
        register_free(generator->register_allocator, value_reg);
        return false;
    }
    
    // Test pattern - will jump to else_label if pattern doesn't match
    // Pass NULL for match_label since we continue execution on match
    if (!code_generate_pattern_test(generator, pattern, value_reg, NULL, else_label)) {
        register_free(generator->register_allocator, value_reg);
        return false;
    }
    
    // Pattern matched - generate bindings
    if (!code_generate_pattern_bindings(generator, pattern, value_reg)) {
        register_free(generator->register_allocator, value_reg);
        return false;
    }
    
    register_free(generator->register_allocator, value_reg);
    
    // Generate then block
    if (!code_generate_statement(generator, then_block)) {
        return false;
    }
    
    // Jump to end
    if (!emit_jump(generator, end_label)) {
        return false;
    }
    
    // Else label
    if (!emit_label(generator, else_label)) {
        return false;
    }
    
    // Generate else block if present
    if (else_block) {
        if (!code_generate_statement(generator, else_block)) {
            return false;
        }
    }
    
    // End label
    return emit_label(generator, end_label);
}

/**
 * Simple string hash function for variant tags
 * TODO: Replace with proper type system integration
 */
uint32_t simple_string_hash(const char *str) {
    if (!str) return 0;
    
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (uint32_t)c;
    }
    
    return hash;
}

/**
 * Generate pattern test
 * Tests if a value matches a pattern and jumps to appropriate label
 */
bool code_generate_pattern_test(CodeGenerator *generator, ASTNode *pattern, Register value_reg, 
                               char *match_label, char *no_match_label) {
    // For now, use no_match_label as the fail label
    const char *fail_label = no_match_label;
    if (!generator || !pattern || value_reg == ASTHRA_REG_NONE || !fail_label) {
        return false;
    }
    
    switch (pattern->type) {
        case AST_WILDCARD_PATTERN:
            // Wildcard always matches
            return true;
            
        // For now, literal patterns are not directly supported - use identifiers
        // TODO: Add literal pattern support when AST types are updated
        
        case AST_IDENTIFIER:
            // Variable pattern always matches (binding handled separately)
            return true;
            
        case AST_ENUM_PATTERN: {
            // Load variant tag from the value
            Register tag_reg = register_allocate(generator->register_allocator, true);
            if (tag_reg == ASTHRA_REG_NONE) {
                return false;
            }
            
            // Load tag (first 4 bytes of enum value)
            AssemblyInstruction *load_tag = create_load_instruction(tag_reg, value_reg, 0);
            if (!load_tag || !instruction_buffer_add(generator->instruction_buffer, load_tag)) {
                register_free(generator->register_allocator, tag_reg);
                return false;
            }
            
            // Compute expected tag based on enum type
            const char *enum_name = pattern->data.enum_pattern.enum_name;
            const char *variant_name = pattern->data.enum_pattern.variant_name;
            uint32_t expected_tag;
            
            // Special handling for Option type
            if (enum_name && strcmp(enum_name, "Option") == 0) {
                if (strcmp(variant_name, "Some") == 0) {
                    expected_tag = 0; // ASTHRA_OPTION_TAG_SOME
                } else if (strcmp(variant_name, "None") == 0) {
                    expected_tag = 1; // ASTHRA_OPTION_TAG_NONE
                } else {
                    // Invalid Option variant
                    register_free(generator->register_allocator, tag_reg);
                    return false;
                }
            }
            // Special handling for Result type
            else if (enum_name && strcmp(enum_name, "Result") == 0) {
                if (strcmp(variant_name, "Ok") == 0) {
                    expected_tag = 0; // ASTHRA_RESULT_TAG_OK
                } else if (strcmp(variant_name, "Err") == 0) {
                    expected_tag = 1; // ASTHRA_RESULT_TAG_ERR
                } else {
                    // Invalid Result variant
                    register_free(generator->register_allocator, tag_reg);
                    return false;
                }
            }
            // For user-defined enums, use hash
            else {
                expected_tag = simple_string_hash(variant_name);
            }
            
            // Load expected tag into register
            Register expected_reg = register_allocate(generator->register_allocator, true);
            if (expected_reg == ASTHRA_REG_NONE) {
                register_free(generator->register_allocator, tag_reg);
                return false;
            }
            
            AssemblyInstruction *mov_expected = create_mov_immediate(expected_reg, expected_tag);
            if (!mov_expected || !instruction_buffer_add(generator->instruction_buffer, mov_expected)) {
                register_free(generator->register_allocator, tag_reg);
                register_free(generator->register_allocator, expected_reg);
                return false;
            }
            
            // Compare tags
            AssemblyInstruction *cmp = create_cmp_instruction(tag_reg, expected_reg);
            if (!cmp || !instruction_buffer_add(generator->instruction_buffer, cmp)) {
                register_free(generator->register_allocator, tag_reg);
                register_free(generator->register_allocator, expected_reg);
                return false;
            }
            
            register_free(generator->register_allocator, tag_reg);
            register_free(generator->register_allocator, expected_reg);
            
            // Jump if not equal - use create_instruction with INST_JNE
            AssemblyInstruction *jne = create_instruction(INST_JNE, 1,
                &(AssemblyOperand){.type = OPERAND_LABEL, .data.label = (char*)fail_label});
            return jne && instruction_buffer_add(generator->instruction_buffer, jne);
        }
        
        case AST_STRUCT_PATTERN: {
            // TODO: Implement struct pattern matching
            // For now, struct patterns always match
            return true;
        }
        
        default:
            // Unknown pattern type
            return false;
    }
}

/**
 * Generate pattern variable bindings
 * Extracts values from matched patterns and binds them to variables
 */
bool code_generate_pattern_bindings(CodeGenerator *generator, ASTNode *pattern, Register value_reg) {
    if (!generator || !pattern || value_reg == ASTHRA_REG_NONE) {
        return false;
    }
    
    switch (pattern->type) {
        case AST_IDENTIFIER: {
            // Variable binding - store the value in a local variable
            const char *var_name = pattern->data.identifier.name;
            LocalVariable *var = find_local_variable(generator, var_name);
            if (!var) {
                // Create new local variable if it doesn't exist
                var = create_local_variable(generator, var_name, value_reg);
                if (!var) {
                    return false;
                }
            }
            
            // Store value to variable location
            AssemblyInstruction *store_inst = create_store_local(value_reg, var->offset);
            return store_inst && instruction_buffer_add(generator->instruction_buffer, store_inst);
        }
        
        case AST_ENUM_PATTERN: {
            return code_generate_enum_pattern_bindings(generator, pattern, value_reg);
        }
        
        case AST_STRUCT_PATTERN: {
            return code_generate_struct_pattern_bindings(generator, pattern, value_reg);
        }
        
        case AST_WILDCARD_PATTERN: {
            // Wildcard pattern - no bindings needed
            return true;
        }
        
        default:
            // Other pattern types don't create bindings
            return true;
    }
}

/**
 * Generate enum pattern bindings
 * Extract associated data from enum variants
 */
bool code_generate_enum_pattern_bindings(CodeGenerator *generator, ASTNode *pattern, Register value_reg) {
    if (!generator || !pattern || pattern->type != AST_ENUM_PATTERN) {
        return false;
    }
    
    // If the enum pattern has a binding variable, extract the associated data
    const char *binding_name = pattern->data.enum_pattern.binding;
    if (binding_name) {
        // Extract data from enum value (skip the tag)
        Register data_reg = register_allocate(generator->register_allocator, true);
        if (data_reg == ASTHRA_REG_NONE) {
            return false;
        }
        
        // Load data from enum value (offset by tag size)
        AssemblyInstruction *load_data = create_load_instruction(data_reg, value_reg, sizeof(uint32_t));
        if (!load_data || !instruction_buffer_add(generator->instruction_buffer, load_data)) {
            register_free(generator->register_allocator, data_reg);
            return false;
        }
        
        // Create local variable for the binding
        LocalVariable *var = create_local_variable(generator, binding_name, data_reg);
        if (!var) {
            register_free(generator->register_allocator, data_reg);
            return false;
        }
        
        // Store data to variable location
        AssemblyInstruction *store_inst = create_store_local(data_reg, var->offset);
        if (!store_inst || !instruction_buffer_add(generator->instruction_buffer, store_inst)) {
            register_free(generator->register_allocator, data_reg);
            return false;
        }
        
        register_free(generator->register_allocator, data_reg);
    }
    
    return true;
}

/**
 * Generate struct pattern bindings
 * Extract fields from struct values
 */
bool code_generate_struct_pattern_bindings(CodeGenerator *generator, ASTNode *pattern, Register value_reg) {
    if (!generator || !pattern || pattern->type != AST_STRUCT_PATTERN) {
        return false;
    }
    
    // TODO: Implement struct field extraction
    // This would iterate through the pattern's field list and extract each field
    // For now, return true as a placeholder
    
    (void)generator;
    (void)value_reg;
    return true;
}

/**
 * Generate code for match statement
 */
bool code_generate_match_statement(CodeGenerator *generator, ASTNode *stmt) {
    if (!generator || !stmt || stmt->type != AST_MATCH_STMT) {
        return false;
    }
    
    ASTNode *expr = stmt->data.match_stmt.expression;
    ASTNodeList *arms = stmt->data.match_stmt.arms;
    
    if (!expr || !arms) {
        return false;
    }
    
    // Generate the match expression
    Register value_reg = code_generate_expression(generator, expr, ASTHRA_REG_NONE);
    if (value_reg == ASTHRA_REG_NONE) {
        return false;
    }
    
    // Create match end label
    char *match_end = label_manager_create_label(generator->label_manager, LABEL_BRANCH_TARGET, ".L_match_end");
    if (!match_end) {
        register_free(generator->register_allocator, value_reg);
        return false;
    }
    
    // Generate code for each match arm
    size_t arm_count = ast_node_list_size(arms);
    for (size_t i = 0; i < arm_count; i++) {
        ASTNode *arm = ast_node_list_get(arms, i);
        if (!arm || arm->type != AST_MATCH_ARM) {
            continue;
        }
        
        ASTNode *pattern = arm->data.match_arm.pattern;
        ASTNode *body = arm->data.match_arm.body;
        
        if (!pattern || !body) {
            continue;
        }
        
        // Create labels for this arm
        char *next_arm_label = NULL;
        if (i < arm_count - 1) {
            next_arm_label = label_manager_create_label(generator->label_manager, LABEL_BRANCH_TARGET, ".L_match_next");
            if (!next_arm_label) {
                register_free(generator->register_allocator, value_reg);
                return false;
            }
        } else {
            // For the last arm, we need a label even if we don't use it
            // This is because code_generate_pattern_test requires a non-NULL fail label
            next_arm_label = match_end;  // Use match_end as the fail label for exhaustiveness
        }
        
        // Test pattern - will jump to next_arm_label if pattern doesn't match
        if (!code_generate_pattern_test(generator, pattern, value_reg, NULL, next_arm_label)) {
            register_free(generator->register_allocator, value_reg);
            return false;
        }
        
        // Pattern matched - generate bindings
        if (!code_generate_pattern_bindings(generator, pattern, value_reg)) {
            register_free(generator->register_allocator, value_reg);
            return false;
        }
        
        // Generate arm body (can be either a block statement or an expression)
        if (body->type == AST_BLOCK) {
            // Body is a block statement
            if (!code_generate_statement(generator, body)) {
                register_free(generator->register_allocator, value_reg);
                return false;
            }
        } else {
            // Body is an expression - evaluate it
            Register expr_reg = code_generate_expression(generator, body, ASTHRA_REG_NONE);
            if (expr_reg == ASTHRA_REG_NONE) {
                register_free(generator->register_allocator, value_reg);
                return false;
            }
            register_free(generator->register_allocator, expr_reg);
        }
        
        // Jump to end of match
        if (!emit_jump(generator, match_end)) {
            register_free(generator->register_allocator, value_reg);
            return false;
        }
        
        // Emit next arm label if not the last arm
        if (i < arm_count - 1 && next_arm_label && !emit_label(generator, next_arm_label)) {
            register_free(generator->register_allocator, value_reg);
            return false;
        }
    }
    
    // Emit match end label
    if (!emit_label(generator, match_end)) {
        register_free(generator->register_allocator, value_reg);
        return false;
    }
    
    register_free(generator->register_allocator, value_reg);
    return true;
}