/**
 * Asthra Programming Language LLVM Unary Operations
 * Implementation of unary operation code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_unary_ops.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_types.h"
#include "llvm_locals.h"
#include "llvm_access_expr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration
static LLVMValueRef generate_lvalue(LLVMBackendData *data, const ASTNode *node);

// Generate code for unary operations
LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    if (!node->data.unary_expr.operand) {
        LLVM_REPORT_ERROR(data, node, "Unary operation missing operand");
    }

    // Handle sizeof specially since its operand is a type, not an expression
    if (node->data.unary_expr.operator == UNOP_SIZEOF) {
        // sizeof operator processing
        // sizeof operator - operand is a type node, not an expression
        // The operand doesn't have type_info because it's a type specification
        // We need to resolve the type from the operand AST node
        
        if (!node->data.unary_expr.operand) {
            LLVM_REPORT_ERROR(data, node, "sizeof missing operand");
        }
        
        // For sizeof, we need to get the type from the operand AST node
        // The operand represents a type (like i32, Point, etc.), not an expression
        LLVMTypeRef llvm_type = NULL;
        
        const ASTNode *type_operand = node->data.unary_expr.operand;
        
        // Handle different types of type operands
        if (type_operand->type == AST_BASE_TYPE) {
            // Base type like i32, usize, etc.
            const char *type_name = type_operand->data.base_type.name;
            
            // Get primitive type
            if (strcmp(type_name, "i8") == 0) {
                llvm_type = LLVMInt8TypeInContext(data->context);
            } else if (strcmp(type_name, "i16") == 0) {
                llvm_type = LLVMInt16TypeInContext(data->context);
            } else if (strcmp(type_name, "i32") == 0) {
                llvm_type = data->i32_type;
            } else if (strcmp(type_name, "i64") == 0) {
                llvm_type = data->i64_type;
            } else if (strcmp(type_name, "u8") == 0) {
                llvm_type = LLVMInt8TypeInContext(data->context);
            } else if (strcmp(type_name, "u16") == 0) {
                llvm_type = LLVMInt16TypeInContext(data->context);
            } else if (strcmp(type_name, "u32") == 0) {
                llvm_type = LLVMInt32TypeInContext(data->context);
            } else if (strcmp(type_name, "u64") == 0) {
                llvm_type = LLVMInt64TypeInContext(data->context);
            } else if (strcmp(type_name, "usize") == 0) {
                llvm_type = data->i64_type; // usize is i64 on 64-bit platforms
            } else if (strcmp(type_name, "bool") == 0) {
                llvm_type = data->bool_type;
            } else if (strcmp(type_name, "f32") == 0) {
                llvm_type = data->f32_type;
            } else if (strcmp(type_name, "f64") == 0) {
                llvm_type = data->f64_type;
            } else {
                LLVM_REPORT_ERROR_PRINTF(data, node, "Unknown type for sizeof: %s", type_name);
            }
        } else if (type_operand->type == AST_IDENTIFIER) {
            // Simple type like i32, usize, etc. (fallback for some cases)
            const char *type_name = type_operand->data.identifier.name;
            
            // Get primitive type
            if (strcmp(type_name, "i8") == 0) {
                llvm_type = LLVMInt8TypeInContext(data->context);
            } else if (strcmp(type_name, "i16") == 0) {
                llvm_type = LLVMInt16TypeInContext(data->context);
            } else if (strcmp(type_name, "i32") == 0) {
                llvm_type = data->i32_type;
            } else if (strcmp(type_name, "i64") == 0) {
                llvm_type = data->i64_type;
            } else if (strcmp(type_name, "u8") == 0) {
                llvm_type = LLVMInt8TypeInContext(data->context);
            } else if (strcmp(type_name, "u16") == 0) {
                llvm_type = LLVMInt16TypeInContext(data->context);
            } else if (strcmp(type_name, "u32") == 0) {
                llvm_type = LLVMInt32TypeInContext(data->context);
            } else if (strcmp(type_name, "u64") == 0) {
                llvm_type = LLVMInt64TypeInContext(data->context);
            } else if (strcmp(type_name, "usize") == 0) {
                llvm_type = data->i64_type; // usize is i64 on 64-bit platforms
            } else if (strcmp(type_name, "bool") == 0) {
                llvm_type = data->bool_type;
            } else if (strcmp(type_name, "f32") == 0) {
                llvm_type = data->f32_type;
            } else if (strcmp(type_name, "f64") == 0) {
                llvm_type = data->f64_type;
            } else {
                LLVM_REPORT_ERROR_PRINTF(data, node, "Unknown type for sizeof: %s", type_name);
            }
        } else if (type_operand->type == AST_PTR_TYPE) {
            // Pointer types like *const i32, *mut i32
            llvm_type = data->ptr_type; // All pointers are the same size
        } else if (type_operand->type == AST_ARRAY_TYPE) {
            // Array types like [3]i32, [5]u8
            const ASTNode *element_type_node = type_operand->data.array_type.element_type;
            const ASTNode *size_node = type_operand->data.array_type.size;
            
            if (!element_type_node || !size_node) {
                LLVM_REPORT_ERROR(data, node, "Array type missing element type or size");
            }
            
            // Get the element type
            LLVMTypeRef element_type = NULL;
            if (element_type_node->type == AST_BASE_TYPE) {
                const char *type_name = element_type_node->data.base_type.name;
                if (strcmp(type_name, "i8") == 0) {
                    element_type = LLVMInt8TypeInContext(data->context);
                } else if (strcmp(type_name, "i16") == 0) {
                    element_type = LLVMInt16TypeInContext(data->context);
                } else if (strcmp(type_name, "i32") == 0) {
                    element_type = data->i32_type;
                } else if (strcmp(type_name, "i64") == 0) {
                    element_type = data->i64_type;
                } else if (strcmp(type_name, "u8") == 0) {
                    element_type = LLVMInt8TypeInContext(data->context);
                } else if (strcmp(type_name, "u16") == 0) {
                    element_type = LLVMInt16TypeInContext(data->context);
                } else if (strcmp(type_name, "u32") == 0) {
                    element_type = LLVMInt32TypeInContext(data->context);
                } else if (strcmp(type_name, "u64") == 0) {
                    element_type = LLVMInt64TypeInContext(data->context);
                } else if (strcmp(type_name, "f32") == 0) {
                    element_type = data->f32_type;
                } else if (strcmp(type_name, "f64") == 0) {
                    element_type = data->f64_type;
                } else {
                    LLVM_REPORT_ERROR_PRINTF(data, node, "Unknown array element type: %s", type_name);
                }
            } else {
                LLVM_REPORT_ERROR(data, node, "Complex array element types not yet supported");
            }
            
            // Get the array size (must be a constant integer)
            unsigned array_size = 0;
            if (size_node->type == AST_INTEGER_LITERAL) {
                array_size = (unsigned)size_node->data.integer_literal.value;
            } else {
                LLVM_REPORT_ERROR(data, node, "Array size must be a constant integer");
            }
            
            // Create the array type
            llvm_type = LLVMArrayType(element_type, array_size);
        } else {
            LLVM_REPORT_ERROR(data, node, "Complex types in sizeof not yet supported");
        }
        
        if (!llvm_type) {
            LLVM_REPORT_ERROR(data, node, "Failed to resolve type for sizeof");
        }
        
        // Get size of the type
        LLVMValueRef size = LLVMSizeOf(llvm_type);
        
        // The result type should be usize (i64 on 64-bit platforms)
        // LLVMSizeOf returns the target's size type, which might be i32 or smaller
        // We need to extend it to i64 to match usize
        LLVMTypeRef size_type = LLVMTypeOf(size);
        if (LLVMGetTypeKind(size_type) == LLVMIntegerTypeKind) {
            unsigned width = LLVMGetIntTypeWidth(size_type);
            if (width < 64) {
                // Zero-extend to i64
                size = LLVMBuildZExt(data->builder, size, data->i64_type, "sizeof_usize");
            }
        }
        return size;
    }

    // For all other unary operators, generate the operand as an expression
    LLVMValueRef operand = generate_expression(data, node->data.unary_expr.operand);
    if (!operand) {
        LLVM_REPORT_ERROR(data, node, "Failed to generate operand for unary operation");
    }

    switch (node->data.unary_expr.operator) {
    case UNOP_MINUS:
        if (LLVMGetTypeKind(LLVMTypeOf(operand)) == LLVMFloatTypeKind ||
            LLVMGetTypeKind(LLVMTypeOf(operand)) == LLVMDoubleTypeKind) {
            return LLVMBuildFNeg(data->builder, operand, "neg");
        } else {
            return LLVMBuildNeg(data->builder, operand, "neg");
        }

    case UNOP_NOT:
        // Logical NOT - convert to boolean first if needed
        if (LLVMGetTypeKind(LLVMTypeOf(operand)) != LLVMIntegerTypeKind ||
            LLVMGetIntTypeWidth(LLVMTypeOf(operand)) != 1) {
            // Convert to bool by comparing with zero
            operand = LLVMBuildICmp(data->builder, LLVMIntNE, operand,
                                    LLVMConstNull(LLVMTypeOf(operand)), "tobool");
        }
        return LLVMBuildNot(data->builder, operand, "not");

    case UNOP_BITWISE_NOT:
        return LLVMBuildNot(data->builder, operand, "bitnot");

    case UNOP_DEREF: {
        // Dereference pointer
        // Get the result type from this node's type info (the dereference result type)
        if (!node->type_info) {
            LLVM_REPORT_ERROR(data, node, "Dereference node missing type info");
        }
        
        LLVMTypeRef elem_type = asthra_type_to_llvm(data, node->type_info);
        if (!elem_type) {
            LLVM_REPORT_ERROR(data, node, "Failed to convert dereference result type to LLVM type");
        }
        
        return LLVMBuildLoad2(data->builder, elem_type, operand, "deref");
    }

    case UNOP_ADDRESS_OF: {
        // Take address - operand must be an lvalue
        // Generate the address of the operand without loading it
        return generate_lvalue(data, node->data.unary_expr.operand);
    }


    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported unary operator: %d",
                                 node->data.unary_expr.operator);
    }
}

// Generate an lvalue (address) for an expression
// This is used by the address-of operator to get the address of a variable
// instead of loading its value
static LLVMValueRef generate_lvalue(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    switch (node->type) {
    case AST_IDENTIFIER: {
        // For identifiers, return the alloca/global address directly
        const char *name = node->data.identifier.name;
        if (!name) {
            LLVM_REPORT_ERROR(data, node, "Identifier has no name");
        }

        // Check local variables
        if (data->current_function) {
            LocalVar *var_entry = lookup_local_var_entry(data, name);
            if (var_entry) {
                // Return the alloca pointer directly (don't load)
                return var_entry->alloca;
            }

            // Check function parameters
            // Parameters need special handling - they're values, not addresses
            // We need to create an alloca and store the parameter value
            LLVMTypeRef fn_type = LLVMGlobalGetValueType(data->current_function);
            unsigned param_count = LLVMCountParamTypes(fn_type);
            
            for (unsigned i = 0; i < param_count; i++) {
                LLVMValueRef param = LLVMGetParam(data->current_function, i);
                const char *param_name = LLVMGetValueName(param);
                if (param_name && strcmp(param_name, name) == 0) {
                    // Parameters are values, not addresses, so we can't take their address
                    LLVM_REPORT_ERROR_PRINTF(data, node, 
                        "Cannot take address of parameter '%s'. Consider copying to a local variable first.", 
                        name);
                }
            }
        }

        // Check global variables
        LLVMValueRef global_var = LLVMGetNamedGlobal(data->module, name);
        if (global_var) {
            // Return the global variable pointer directly
            return global_var;
        }

        // Check if it's a function
        LLVMValueRef global_fn = LLVMGetNamedFunction(data->module, name);
        if (global_fn) {
            // Functions already are addresses
            return global_fn;
        }

        LLVM_REPORT_ERROR_PRINTF(data, node, "Undefined identifier: '%s'", name);
    }

    case AST_FIELD_ACCESS: {
        // For field access, generate GEP to get field address
        // This requires implementing field address calculation
        // For now, report an error
        LLVM_REPORT_ERROR(data, node, "Taking address of struct fields not yet implemented");
    }

    case AST_INDEX_ACCESS: {
        // For array/slice access, generate GEP to get element address
        // This requires implementing element address calculation
        // For now, report an error
        LLVM_REPORT_ERROR(data, node, "Taking address of array/slice elements not yet implemented");
    }

    case AST_UNARY_EXPR:
        if (node->data.unary_expr.operator == UNOP_DEREF) {
            // For &(*ptr), just return ptr
            return generate_expression(data, node->data.unary_expr.operand);
        }
        LLVM_REPORT_ERROR(data, node, "Cannot take address of unary expression");

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Cannot take address of expression type %d", node->type);
    }
}