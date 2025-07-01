/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Checking Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Type compatibility checking and casting utilities
 */

#include "type_checking.h"
#include "semantic_errors.h"
#include "semantic_macros.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TYPE CHECKING UTILITIES
// =============================================================================

bool semantic_check_type_compatibility(SemanticAnalyzer *analyzer, TypeDescriptor *type1,
                                       TypeDescriptor *type2) {
    (void)analyzer;

    if (!type1 || !type2) {
        return false;
    }

    // Exact type match
    if (type_descriptor_equals(type1, type2)) {
        return true;
    }

    // Pointer type compatibility: mutable pointer can be assigned to const pointer
    // e.g., *i32 can be assigned to *const i32
    if (type1->category == TYPE_POINTER && type2->category == TYPE_POINTER) {
        TypeDescriptor *pointee1 = type1->data.pointer.pointee_type;
        TypeDescriptor *pointee2 = type2->data.pointer.pointee_type;

        // Check if pointee types are compatible
        if (pointee1 && pointee2 && type_descriptor_equals(pointee1, pointee2)) {
            // Mutable pointer (*i32) can be assigned to const pointer (*const i32)
            // but not vice versa (covariance)
            if (type1->flags.is_mutable && !type2->flags.is_mutable) {
                return true; // *mut T -> *const T (allowed)
            }
            // Same mutability is always fine (exact match already handled above)
            if (type1->flags.is_mutable == type2->flags.is_mutable) {
                return true;
            }
        }
    }

    // Special case: Result<T, E> type compatibility with Result generic instances
    // Handle case where variable is declared as Result<T, E> (TYPE_RESULT) but
    // expression produces Result generic instance (TYPE_GENERIC_INSTANCE)
    if (type1->category == TYPE_GENERIC_INSTANCE && type2->category == TYPE_RESULT) {
        // Check if type1 is a Result generic instance with matching types
        TypeDescriptor *base_type = type1->data.generic_instance.base_type;
        if (base_type && base_type->name && strcmp(base_type->name, "Result") == 0 &&
            type1->data.generic_instance.type_arg_count == 2) {
            TypeDescriptor *arg1 = type1->data.generic_instance.type_args[0];
            TypeDescriptor *arg2 = type1->data.generic_instance.type_args[1];
            if (type_descriptor_equals(arg1, type2->data.result.ok_type) &&
                type_descriptor_equals(arg2, type2->data.result.err_type)) {
                return true;
            }
        }
    }

    // Reverse case: TYPE_RESULT to TYPE_GENERIC_INSTANCE
    if (type1->category == TYPE_RESULT && type2->category == TYPE_GENERIC_INSTANCE) {
        TypeDescriptor *base_type = type2->data.generic_instance.base_type;
        if (base_type && base_type->name && strcmp(base_type->name, "Result") == 0 &&
            type2->data.generic_instance.type_arg_count == 2) {
            TypeDescriptor *arg1 = type2->data.generic_instance.type_args[0];
            TypeDescriptor *arg2 = type2->data.generic_instance.type_args[1];
            if (type_descriptor_equals(type1->data.result.ok_type, arg1) &&
                type_descriptor_equals(type1->data.result.err_type, arg2)) {
                return true;
            }
        }
    }

    // Never type is a bottom type - it's compatible with any type
    // (Never can be used anywhere any type is expected)
    if (type1->category == TYPE_PRIMITIVE &&
        type1->data.primitive.primitive_kind == PRIMITIVE_NEVER) {
        return true;
    }

    // Check for primitive type compatibility (widening conversions)
    // Can type1 be used where type2 is expected?
    if (type1->category == TYPE_PRIMITIVE && type2->category == TYPE_PRIMITIVE) {
        int type1_prim = type1->data.primitive.primitive_kind;
        int type2_prim = type2->data.primitive.primitive_kind;

        // Integer widening: i8 -> i16 -> i32 -> i64
        // Can type1 be used where type2 is expected?
        if ((type1_prim == PRIMITIVE_I8 &&
             (type2_prim == PRIMITIVE_I16 || type2_prim == PRIMITIVE_I32 ||
              type2_prim == PRIMITIVE_I64)) ||
            (type1_prim == PRIMITIVE_I16 &&
             (type2_prim == PRIMITIVE_I32 || type2_prim == PRIMITIVE_I64)) ||
            (type1_prim == PRIMITIVE_I32 && type2_prim == PRIMITIVE_I64)) {
            return true;
        }

        // Integer narrowing: Allow i32 literals to fit into smaller types (for literals)
        // This is a special case for assignment contexts where literals are involved
        if (type1_prim == PRIMITIVE_I32 &&
            (type2_prim == PRIMITIVE_I8 || type2_prim == PRIMITIVE_I16 ||
             type2_prim == PRIMITIVE_U8 || type2_prim == PRIMITIVE_U16 ||
             type2_prim == PRIMITIVE_U32 || type2_prim == PRIMITIVE_U64)) {
            return true; // Allow for literal assignments like "let x: u8 = 255;"
        }

        // Size type conversions: usize/isize to integer types
        // NOTE: These conversions require explicit casts, not implicit conversion
        // Removing these rules to enforce type safety
        /*
        if (type1_prim == PRIMITIVE_USIZE &&
            (type2_prim == PRIMITIVE_I8 || type2_prim == PRIMITIVE_I16 ||
             type2_prim == PRIMITIVE_I32 || type2_prim == PRIMITIVE_I64 ||
             type2_prim == PRIMITIVE_U8 || type2_prim == PRIMITIVE_U16 ||
             type2_prim == PRIMITIVE_U32 || type2_prim == PRIMITIVE_U64)) {
            return true; // Allow usize to integer conversion (e.g., len() result to i32)
        }

        // Allow isize to signed integer types
        if (type1_prim == PRIMITIVE_ISIZE &&
            (type2_prim == PRIMITIVE_I8 || type2_prim == PRIMITIVE_I16 ||
             type2_prim == PRIMITIVE_I32 || type2_prim == PRIMITIVE_I64)) {
            return true; // Allow isize to signed integer conversion
        }
        */

        // Float widening: f32 -> f64
        // Can type1 be used where type2 is expected?
        if (type1_prim == PRIMITIVE_F32 && type2_prim == PRIMITIVE_F64) {
            return true;
        }

        // Float narrowing: Allow f64 literals to fit into f32 (for literals)
        if (type1_prim == PRIMITIVE_F64 && type2_prim == PRIMITIVE_F32) {
            return true; // Allow for literal assignments like "let x: f32 = 3.14;"
        }
    }

    // Pointer compatibility (same pointee type)
    if (type1->category == TYPE_POINTER && type2->category == TYPE_POINTER) {
        return type_descriptor_equals(type1->data.pointer.pointee_type,
                                      type2->data.pointer.pointee_type);
    }

    // Array to slice conversion: [N]T can be assigned to []T
    if (type1->category == TYPE_ARRAY && type2->category == TYPE_SLICE) {
        return type_descriptor_equals(type1->data.array.element_type,
                                      type2->data.slice.element_type);
    }

    // Tuple compatibility (all element types must be compatible)
    if (type1->category == TYPE_TUPLE && type2->category == TYPE_TUPLE) {
        // Check element count matches
        if (type1->data.tuple.element_count != type2->data.tuple.element_count) {
            return false;
        }

        // Check each element type is compatible
        for (size_t i = 0; i < type1->data.tuple.element_count; i++) {
            if (!semantic_check_type_compatibility(analyzer, type1->data.tuple.element_types[i],
                                                   type2->data.tuple.element_types[i])) {
                return false;
            }
        }

        return true;
    }

    // Enum type compatibility
    if (type1->category == TYPE_ENUM && type2->category == TYPE_ENUM) {
        // For basic enum compatibility, check if they refer to the same enum
        return type_descriptor_equals(type1, type2);
    }

    // Generic instance vs generic enum base type compatibility
    if (type1->category == TYPE_ENUM && type2->category == TYPE_GENERIC_INSTANCE) {
        // Check if type2 is a generic instance of type1
        return (type2->data.generic_instance.base_type == type1);
    }

    if (type1->category == TYPE_GENERIC_INSTANCE && type2->category == TYPE_ENUM) {
        // Check if type1 is a generic instance of type2
        return (type1->data.generic_instance.base_type == type2);
    }

    // Generic instance compatibility
    if (type1->category == TYPE_GENERIC_INSTANCE && type2->category == TYPE_GENERIC_INSTANCE) {
        // Must have same base type and compatible type arguments
        if (type1->data.generic_instance.base_type != type2->data.generic_instance.base_type) {
            return false;
        }

        // Check type argument count
        if (type1->data.generic_instance.type_arg_count !=
            type2->data.generic_instance.type_arg_count) {
            return false;
        }

        // Check each type argument for compatibility
        for (size_t i = 0; i < type1->data.generic_instance.type_arg_count; i++) {
            if (!semantic_check_type_compatibility(analyzer,
                                                   type1->data.generic_instance.type_args[i],
                                                   type2->data.generic_instance.type_args[i])) {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool semantic_can_cast(SemanticAnalyzer *analyzer, TypeDescriptor *from, TypeDescriptor *to) {
    (void)analyzer;

    if (!from || !to) {
        return false;
    }

    // Exact type match
    if (type_descriptor_equals(from, to)) {
        return true;
    }

    // Primitive type casting
    if (from->category == TYPE_PRIMITIVE && to->category == TYPE_PRIMITIVE) {
        int from_prim = from->data.primitive.primitive_kind;
        int to_prim = to->data.primitive.primitive_kind;

        // Integer widening (implicit)
        if ((from_prim == PRIMITIVE_I8 &&
             (to_prim == PRIMITIVE_I16 || to_prim == PRIMITIVE_I32 || to_prim == PRIMITIVE_I64)) ||
            (from_prim == PRIMITIVE_I16 &&
             (to_prim == PRIMITIVE_I32 || to_prim == PRIMITIVE_I64)) ||
            (from_prim == PRIMITIVE_I32 && to_prim == PRIMITIVE_I64)) {
            return true;
        }

        // Float widening (implicit)
        if (from_prim == PRIMITIVE_F32 && to_prim == PRIMITIVE_F64) {
            return true;
        }

        // Integer narrowing (explicit cast allowed)
        if ((from_prim == PRIMITIVE_I64 &&
             (to_prim == PRIMITIVE_I32 || to_prim == PRIMITIVE_I16 || to_prim == PRIMITIVE_I8)) ||
            (from_prim == PRIMITIVE_I32 && (to_prim == PRIMITIVE_I16 || to_prim == PRIMITIVE_I8)) ||
            (from_prim == PRIMITIVE_I16 && to_prim == PRIMITIVE_I8)) {
            return false; // Narrowing requires explicit cast, not implicit
        }

        // Float narrowing (explicit cast allowed)
        if (from_prim == PRIMITIVE_F64 && to_prim == PRIMITIVE_F32) {
            return false; // Narrowing requires explicit cast, not implicit
        }

        // Integer to float conversion (explicit)
        if ((from_prim == PRIMITIVE_I8 || from_prim == PRIMITIVE_I16 ||
             from_prim == PRIMITIVE_I32 || from_prim == PRIMITIVE_I64) &&
            (to_prim == PRIMITIVE_F32 || to_prim == PRIMITIVE_F64)) {
            return false; // Requires explicit cast
        }

        // Float to integer conversion (explicit)
        if ((from_prim == PRIMITIVE_F32 || from_prim == PRIMITIVE_F64) &&
            (to_prim == PRIMITIVE_I8 || to_prim == PRIMITIVE_I16 || to_prim == PRIMITIVE_I32 ||
             to_prim == PRIMITIVE_I64)) {
            return false; // Requires explicit cast
        }
    }

    return false;
}