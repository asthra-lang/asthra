/**
 * Asthra Programming Language LLVM Pattern Matching
 * Implementation of pattern matching for match statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_pattern_matching.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_locals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for internal helper functions
static bool generate_literal_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                           LLVMValueRef value, LLVMBasicBlockRef match_block,
                                           LLVMBasicBlockRef next_block);
static bool generate_enum_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                        LLVMValueRef value, LLVMBasicBlockRef match_block,
                                        LLVMBasicBlockRef next_block);
static bool generate_wildcard_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                            LLVMValueRef value, LLVMBasicBlockRef match_block,
                                            LLVMBasicBlockRef next_block);

// Generate pattern check - returns true if pattern check was generated successfully
bool generate_pattern_check(LLVMBackendData *data, const ASTNode *pattern, LLVMValueRef value,
                            LLVMBasicBlockRef match_block, LLVMBasicBlockRef next_block) {
    if (!data || !pattern || !value || !match_block || !next_block) {
        return false;
    }

    switch (pattern->type) {
    case AST_INTEGER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_BOOL_LITERAL:
    case AST_CHAR_LITERAL:
        return generate_literal_pattern_check(data, pattern, value, match_block, next_block);

    case AST_ENUM_PATTERN:
        return generate_enum_pattern_check(data, pattern, value, match_block, next_block);

    case AST_WILDCARD_PATTERN:
        return generate_wildcard_pattern_check(data, pattern, value, match_block, next_block);

    case AST_IDENTIFIER:
        // Identifier patterns always match (binding pattern)
        LLVMBuildBr(data->builder, match_block);
        return true;

    case AST_STRUCT_PATTERN:
        // TODO: Implement struct pattern matching
        llvm_backend_report_error(data, pattern, "Struct pattern matching not yet implemented");
        return false;

    case AST_TUPLE_PATTERN:
        // TODO: Implement tuple pattern matching
        llvm_backend_report_error(data, pattern, "Tuple pattern matching not yet implemented");
        return false;

    default:
        llvm_backend_report_error_printf(data, pattern, "Unsupported pattern type: %d",
                                         pattern->type);
        return false;
    }
}

// Generate literal pattern check
static bool generate_literal_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                           LLVMValueRef value, LLVMBasicBlockRef match_block,
                                           LLVMBasicBlockRef next_block) {
    if (!data || !pattern || !value || !match_block || !next_block) {
        return false;
    }

    LLVMValueRef pattern_value = generate_expression(data, pattern);
    if (!pattern_value) {
        return false;
    }

    // Generate comparison
    LLVMValueRef cmp_result = NULL;
    LLVMTypeKind value_kind = LLVMGetTypeKind(LLVMTypeOf(value));

    if (value_kind == LLVMFloatTypeKind || value_kind == LLVMDoubleTypeKind) {
        // Floating point comparison
        cmp_result = LLVMBuildFCmp(data->builder, LLVMRealOEQ, value, pattern_value, "pattern_cmp");
    } else {
        // Integer comparison (includes booleans and characters)
        cmp_result = LLVMBuildICmp(data->builder, LLVMIntEQ, value, pattern_value, "pattern_cmp");
    }

    if (!cmp_result) {
        return false;
    }

    // Generate conditional branch
    LLVMBuildCondBr(data->builder, cmp_result, match_block, next_block);
    return true;
}

// Generate enum pattern check
static bool generate_enum_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                        LLVMValueRef value, LLVMBasicBlockRef match_block,
                                        LLVMBasicBlockRef next_block) {
    if (!data || !pattern || !value || !match_block || !next_block) {
        return false;
    }

    // For now, implement a simple enum variant check
    // TODO: This needs to be enhanced to handle actual enum types and extract variant data

    // Assume enum is represented as an integer tag
    // Extract the tag from the enum value (assume first field)
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i32_type, 0, false), // struct index
        LLVMConstInt(data->i32_type, 0, false)  // tag field index
    };

    LLVMTypeRef enum_type = LLVMTypeOf(value);
    LLVMValueRef tag_ptr =
        LLVMBuildGEP2(data->builder, enum_type, value, indices, 2, "enum_tag_ptr");
    LLVMValueRef tag_value = LLVMBuildLoad2(data->builder, data->i32_type, tag_ptr, "enum_tag");

    // For now, assume the variant name maps to an integer constant
    // TODO: This should use actual enum variant information from type system
    LLVMValueRef expected_tag = LLVMConstInt(data->i32_type, 0, false); // Placeholder

    LLVMValueRef cmp_result =
        LLVMBuildICmp(data->builder, LLVMIntEQ, tag_value, expected_tag, "enum_pattern_cmp");
    LLVMBuildCondBr(data->builder, cmp_result, match_block, next_block);

    return true;
}

// Generate wildcard pattern check (always matches)
static bool generate_wildcard_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                            LLVMValueRef value, LLVMBasicBlockRef match_block,
                                            LLVMBasicBlockRef next_block) {
    if (!data || !match_block) {
        return false;
    }

    // Wildcard pattern always matches
    LLVMBuildBr(data->builder, match_block);
    return true;
}

// Generate identifier pattern binding
bool generate_identifier_pattern_binding(LLVMBackendData *data, const ASTNode *pattern,
                                         LLVMValueRef value) {
    if (!data || !pattern || !value) {
        return false;
    }

    // Only bind for identifier patterns
    if (pattern->type == AST_IDENTIFIER) {
        const char *var_name = pattern->data.identifier.name;
        if (var_name) {
            // Create local variable for the binding
            LLVMTypeRef value_type = LLVMTypeOf(value);
            LLVMValueRef alloca = LLVMBuildAlloca(data->builder, value_type, var_name);
            LLVMBuildStore(data->builder, value, alloca);

            // Register the binding for later lookup
            register_local_var(data, var_name, alloca, value_type);
            return true;
        }
    }

    return false;
}