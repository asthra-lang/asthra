/**
 * Asthra Programming Language LLVM Statement Generation
 * Implementation of statement code generation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_stmt_gen.h"
#include "llvm_expr_gen.h"
#include "llvm_types.h"
#include "llvm_debug.h"
#include "llvm_locals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Generate code for statements
void generate_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data) {
        return;
    }
    
    if (!node) {
        return;
    }
    
    
    // Set debug location for this statement
    set_debug_location(data, node);
    
    switch (node->type) {
        case AST_RETURN_STMT:
            {
                LLVMValueRef ret_val = NULL;
                if (node->data.return_stmt.expression) {
                    // Check if the expression is a unit literal for void return
                    if (node->data.return_stmt.expression->type == AST_UNIT_LITERAL) {
                        // For unit literals in void functions, use RetVoid
                        // In LLVM 15+, use LLVMGlobalGetValueType to get the function type
                        LLVMTypeRef fn_type = LLVMGlobalGetValueType(data->current_function);
                        LLVMTypeRef fn_ret_type = LLVMGetReturnType(fn_type);
                        if (fn_ret_type == data->void_type) {
                            LLVMBuildRetVoid(data->builder);
                            break;
                        }
                    }
                    ret_val = generate_expression(data, node->data.return_stmt.expression);
                }
                
                if (ret_val) {
                    LLVMBuildRet(data->builder, ret_val);
                } else {
                    LLVMBuildRetVoid(data->builder);
                }
            }
            break;
            
        case AST_LET_STMT:  // Variable declaration
            {
                const char *var_name = node->data.let_stmt.name;
                LLVMTypeRef var_type = data->i32_type; // Default type
                
                // Get type from type_info if available
                if (node->type_info) {
                    var_type = asthra_type_to_llvm(data, node->type_info);
                } else if (node->data.let_stmt.type && node->data.let_stmt.type->type_info) {
                    var_type = asthra_type_to_llvm(data, node->data.let_stmt.type->type_info);
                }
                
                // Allocate space on stack
                LLVMValueRef alloca = LLVMBuildAlloca(data->builder, var_type, var_name);
                
                // Initialize if there's an initializer
                if (node->data.let_stmt.initializer) {
                    LLVMValueRef init_val = generate_expression(data, node->data.let_stmt.initializer);
                    if (init_val) {
                        LLVMBuildStore(data->builder, init_val, alloca);
                    }
                }
                
                // Generate debug info for local variable
                if (data->di_builder && data->current_debug_scope && node->location.line > 0) {
                    LLVMMetadataRef di_var_type = asthra_type_to_debug_type(data, 
                        node->type_info ? node->type_info : 
                        (node->data.let_stmt.type ? node->data.let_stmt.type->type_info : NULL));
                    
                    if (di_var_type) {
                        LLVMMetadataRef di_var = LLVMDIBuilderCreateAutoVariable(
                            data->di_builder,
                            data->current_debug_scope,
                            var_name,
                            strlen(var_name),
                            data->di_file,
                            node->location.line,
                            di_var_type,
                            true,  // preserve through opts
                            LLVMDIFlagZero,
                            0  // align
                        );
                        
                        // Create debug declare
                        LLVMMetadataRef expr = LLVMDIBuilderCreateExpression(data->di_builder, NULL, 0);
                        LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(
                            data->context,
                            node->location.line,
                            node->location.column,
                            data->current_debug_scope,
                            NULL
                        );
                        
                        // Insert declare instruction for debug info
                        // TODO: Update to use LLVMDIBuilderInsertDeclareRecordAtEnd for LLVM 20+
                        // LLVMValueRef dbg_declare = LLVMDIBuilderInsertDeclareAtEnd(
                        //     data->di_builder,
                        //     alloca,
                        //     di_var,
                        //     expr,
                        //     loc,
                        //     LLVMGetInsertBlock(data->builder)
                        // );
                    }
                }
                
                // Register the local variable for later lookup
                register_local_var(data, var_name, alloca, var_type);
            }
            break;
            
        case AST_ASSIGNMENT:
            {
                // Generate lvalue address
                LLVMValueRef target = NULL;
                
                if (node->data.assignment.target->type == AST_IDENTIFIER) {
                    // Simple variable assignment
                    const char *var_name = node->data.assignment.target->data.identifier.name;
                    // Look up local variable first
                    target = lookup_local_var(data, var_name);
                    if (!target) {
                        // Try global variable
                        target = LLVMGetNamedGlobal(data->module, var_name);
                    }
                } else if (node->data.assignment.target->type == AST_FIELD_ACCESS) {
                    // Field assignment
                    target = generate_expression(data, node->data.assignment.target);
                } else if (node->data.assignment.target->type == AST_INDEX_ACCESS) {
                    // Array element assignment
                    LLVMValueRef array = generate_expression(data, 
                        node->data.assignment.target->data.index_access.array);
                    LLVMValueRef index = generate_expression(data, 
                        node->data.assignment.target->data.index_access.index);
                    
                    if (array && index) {
                        LLVMValueRef indices[2] = {
                            LLVMConstInt(data->i64_type, 0, false),
                            index
                        };
                        LLVMTypeRef array_type = LLVMTypeOf(array);
                        target = LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "elemptr");
                    }
                }
                
                // Generate rvalue
                if (target) {
                    LLVMValueRef value = generate_expression(data, node->data.assignment.value);
                    if (value) {
                        LLVMBuildStore(data->builder, value, target);
                    }
                }
            }
            break;
            
        case AST_IF_STMT:
            {
                // Generate condition
                LLVMValueRef cond = generate_expression(data, node->data.if_stmt.condition);
                if (!cond) break;
                
                // Convert to boolean if needed
                if (LLVMGetTypeKind(LLVMTypeOf(cond)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(cond)) != 1) {
                    cond = LLVMBuildICmp(data->builder, LLVMIntNE, cond,
                                         LLVMConstNull(LLVMTypeOf(cond)), "ifcond");
                }
                
                // Create basic blocks
                LLVMValueRef function = data->current_function;
                LLVMBasicBlockRef then_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "then");
                LLVMBasicBlockRef else_bb = NULL;
                LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "ifcont");
                
                if (node->data.if_stmt.else_block) {
                    else_bb = LLVMAppendBasicBlockInContext(
                        data->context, function, "else");
                    LLVMBuildCondBr(data->builder, cond, then_bb, else_bb);
                } else {
                    LLVMBuildCondBr(data->builder, cond, then_bb, merge_bb);
                }
                
                // Generate then block
                LLVMPositionBuilderAtEnd(data->builder, then_bb);
                generate_statement(data, node->data.if_stmt.then_block);
                if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
                    LLVMBuildBr(data->builder, merge_bb);
                }
                
                // Generate else block if present
                if (else_bb) {
                    LLVMPositionBuilderAtEnd(data->builder, else_bb);
                    generate_statement(data, node->data.if_stmt.else_block);
                    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
                        LLVMBuildBr(data->builder, merge_bb);
                    }
                }
                
                // Continue with merge block
                LLVMPositionBuilderAtEnd(data->builder, merge_bb);
            }
            break;
            
        case AST_FOR_STMT:
            {
                // Asthra for loops iterate over collections/ranges
                // for i in range(10) { ... }
                
                // Create basic blocks
                LLVMValueRef function = data->current_function;
                LLVMBasicBlockRef loop_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "loop");
                LLVMBasicBlockRef loop_body_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "loop_body");
                LLVMBasicBlockRef loop_end_bb = LLVMAppendBasicBlockInContext(
                    data->context, function, "loop_end");
                
                // TODO: Handle iterator initialization based on iterable type
                // For now, assume numeric range starting at 0
                
                // Create loop variable
                LLVMTypeRef loop_var_type = data->i32_type;
                LLVMValueRef loop_var = LLVMBuildAlloca(data->builder, loop_var_type, 
                                                        node->data.for_stmt.variable);
                LLVMBuildStore(data->builder, LLVMConstInt(loop_var_type, 0, false), loop_var);
                
                // Jump to loop header
                LLVMBuildBr(data->builder, loop_bb);
                
                // Loop header - check condition
                LLVMPositionBuilderAtEnd(data->builder, loop_bb);
                LLVMValueRef current_val = LLVMBuildLoad2(data->builder, loop_var_type, loop_var, "loop_var");
                
                // TODO: Get actual loop bound from iterable
                LLVMValueRef loop_bound = LLVMConstInt(loop_var_type, 10, false);
                LLVMValueRef cond = LLVMBuildICmp(data->builder, LLVMIntSLT, 
                                                   current_val, loop_bound, "loopcond");
                LLVMBuildCondBr(data->builder, cond, loop_body_bb, loop_end_bb);
                
                // Loop body
                LLVMPositionBuilderAtEnd(data->builder, loop_body_bb);
                generate_statement(data, node->data.for_stmt.body);
                
                // Increment loop variable
                LLVMValueRef inc_val = LLVMBuildAdd(data->builder, current_val,
                                                     LLVMConstInt(loop_var_type, 1, false), "inc");
                LLVMBuildStore(data->builder, inc_val, loop_var);
                LLVMBuildBr(data->builder, loop_bb);
                
                // Continue after loop
                LLVMPositionBuilderAtEnd(data->builder, loop_end_bb);
            }
            break;
            
        case AST_BLOCK:
            {
                // Create lexical scope for debug info
                LLVMMetadataRef old_scope = data->current_debug_scope;
                if (data->di_builder && data->current_debug_scope && node->location.line > 0) {
                    data->current_debug_scope = LLVMDIBuilderCreateLexicalBlock(
                        data->di_builder,
                        data->current_debug_scope,
                        data->di_file,
                        node->location.line,
                        node->location.column
                    );
                }
                
                // Generate all statements in the block
                if (node->data.block.statements) {
                    for (size_t i = 0; i < node->data.block.statements->count; i++) {
                        generate_statement(data, node->data.block.statements->nodes[i]);
                    }
                }
                
                // Restore previous scope
                data->current_debug_scope = old_scope;
            }
            break;
            
        case AST_EXPR_STMT:
            {
                // Expression statement - evaluate for side effects
                generate_expression(data, node->data.expr_stmt.expression);
            }
            break;
            
        case AST_BREAK_STMT:
            // TODO: Implement break statement (need loop context)
            llvm_backend_report_error(data, node, "Break statement not yet implemented");
            break;
            
        case AST_CONTINUE_STMT:
            // TODO: Implement continue statement (need loop context)
            llvm_backend_report_error(data, node, "Continue statement not yet implemented");
            break;
            
        case AST_MATCH_STMT:
            // TODO: Implement pattern matching
            llvm_backend_report_error(data, node, "Match statement not yet implemented");
            break;
            
        default:
            llvm_backend_report_error_printf(data, node, "Unknown statement type: %d", node->type);
            break;
    }
}