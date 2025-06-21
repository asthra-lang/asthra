/**
 * Asthra Programming Language Compiler
 * AST Node Structure Definitions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core AST node structure with all variant data
 */

#ifndef ASTHRA_AST_NODE_H
#define ASTHRA_AST_NODE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "ast_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Phase 2.1: AST node list with flexible array member
struct ASTNodeList {
    size_t count;
    size_t capacity;
    ASTNode *nodes[];  // C17 flexible array member
};

// C17 static assertion for node list structure
_Static_assert(offsetof(ASTNodeList, nodes) == sizeof(size_t) * 2, 
               "ASTNodeList flexible array must be properly aligned");

// Phase 2.1: Enhanced AST node structure with C17 features
struct ASTNode {
    ASTNodeType type;
    SourceLocation location;
    TypeInfo *type_info;  // Set during type checking
    
    // Phase 2.1: C17 atomic reference counting for memory management
    ast_atomic_refcount_t ref_count;
    
    // Phase 2.1: Compact flags using bitfields for memory efficiency
    struct {
        bool is_validated : 1;
        bool is_type_checked : 1;
        bool is_constant_expr : 1;
        bool has_side_effects : 1;
        bool is_lvalue : 1;
        bool is_mutable : 1;
        uint8_t reserved : 2;  // For future use
    } flags;
    
    union {
        // Program structure with flexible arrays - Phase 2.1
        struct {
            ASTNode *package_decl;
            ASTNodeList *imports;     // Phase 2.1: Flexible array
            ASTNodeList *declarations; // Phase 2.1: Flexible array
        } program;
        
        struct {
            char *name;
        } package_decl;
        
        struct {
            char *path;
            char *alias;            // Optional alias for import (e.g., "as str")
        } import_decl;
        
        struct {
            bool is_public;           // true for pub, false for private
        } visibility_modifier;
        
        // Declarations with flexible arrays - Phase 2.1
        struct {
            char *name;
            ASTNodeList *params;      // Phase 2.1: Flexible array
            ASTNode *return_type;
            ASTNode *body;
            VisibilityType visibility; // NEW: pub or private visibility
            ASTNodeList *annotations; // Phase 2.1: Flexible array
        } function_decl;
        
        struct {
            char *name;
            ASTNodeList *fields;      // Phase 2.1: Flexible array
            ASTNodeList *type_params; // Optional type parameters (e.g., Vec<T>) - Phase 2: Generic Structs
            VisibilityType visibility; // NEW: pub or private visibility
            ASTNodeList *annotations; // Phase 2.1: Flexible array
        } struct_decl;
        
        struct {
            char *name;               // Field name
            ASTNode *type;            // Field type
            VisibilityType visibility; // NEW: pub or private visibility
        } struct_field;
        
        struct {
            char *name;
            ASTNodeList *variants;    // List of enum variant declarations
            ASTNodeList *type_params; // Optional type parameters (e.g., Option<T>)
            VisibilityType visibility; // pub or private visibility
            ASTNodeList *annotations; // Annotations like ownership tags
        } enum_decl;
        
        struct {
            char *name;               // Variant name (e.g., "Some", "None")
            ASTNode *associated_type; // Optional associated type (e.g., T in Some(T))
            ASTNode *value;           // Optional explicit value (e.g., OK = 200)
            VisibilityType visibility; // pub or private visibility for variant
        } enum_variant_decl;
        
        struct {
            char *name;
            ASTNodeList *params;      // Phase 2.1: Flexible array
            ASTNode *return_type;
            char *extern_name;  // e.g., "libc"
            ASTNodeList *annotations; // Phase 2.1: Flexible array
        } extern_decl;
        
        struct {
            char *name;
            ASTNode *type;
            ASTNodeList *annotations; // Phase 2.1: Flexible array
        } param_decl;
        
        // Const declaration structure
        struct {
            char *name;                    // Constant name
            ASTNode *type;                 // Type annotation (required)
            ASTNode *value;                // Constant expression (required)
            VisibilityType visibility;     // pub or private visibility
            ASTNodeList *annotations;      // Optional annotations
        } const_decl;
        
        // Statements with flexible arrays - Phase 2.1
        struct {
            ASTNodeList *statements;  // Phase 2.1: Flexible array
        } block;
        
        struct {
            ASTNode *expression;
        } expr_stmt;
        
        struct {
            char *name;
            ASTNode *type;  // Required in v1.15+ (was optional)
            ASTNode *initializer;
            bool is_mutable;          // NEW (v1.25): true if declared with 'mut' keyword
        } let_stmt;
        
        struct {
            ASTNode *expression;  // Required in v1.14+ (was optional)
        } return_stmt;
        
        struct {
            ASTNode *condition;
            ASTNode *then_block;
            ASTNode *else_block;  // Optional
        } if_stmt;
        
        struct {
            char *variable;       // Loop variable name (e.g., "i")
            ASTNode *iterable;    // What to iterate over (e.g., range(3))
            ASTNode *body;        // Loop body
        } for_stmt;
        
        struct {
            ASTNode *expression;
            ASTNodeList *arms;    // Phase 2.1: Flexible array
        } match_stmt;
        
        struct {
            ASTNode *pattern;
            ASTNode *expression;
            ASTNode *then_block;
            ASTNode *else_block;  // Optional
        } if_let_stmt;
        
        struct {
            char *function_name;
            ASTNodeList *args;    // Phase 2.1: Flexible array
        } spawn_stmt;
        
        struct {
            ASTNode *block;
        } unsafe_block;
        
        struct {
            // Break statement - no data needed
            char _unused;  // Placeholder to avoid empty struct
        } break_stmt;
        
        struct {
            // Continue statement - no data needed
            char _unused;  // Placeholder to avoid empty struct
        } continue_stmt;
        
        // Expressions
        struct {
            BinaryOperator operator;
            ASTNode *left;
            ASTNode *right;
        } binary_expr;
        
        struct {
            UnaryOperator operator;
            ASTNode *operand;
        } unary_expr;
        
        struct {
            ASTNode *base;
            ASTNodeList *suffixes;  // Phase 2.1: Flexible array
        } postfix_expr;
        
        struct {
            ASTNode *function;
            ASTNodeList *args;      // Phase 2.1: Flexible array
        } call_expr;
        
        struct {
            char *struct_name;      // Struct name (e.g., Point in Point::new())
            char *function_name;    // Function name (e.g., new in Point::new())
            ASTNodeList *type_args; // Optional type arguments (e.g., <i32> in Vec<i32>::new()) - Phase 2: Generic Type Support
            ASTNodeList *args;      // Function arguments
        } associated_func_call;
        
        struct {
            ASTNode *object;
            char *field_name;
        } field_access;
        

        
        struct {
            ASTNode *array;
            ASTNode *index;
        } index_access;
        
        struct {
            ASTNode *array;
            ASTNode *start;  // Optional - NULL for [:end]
            ASTNode *end;    // Optional - NULL for [start:]
        } slice_expr;
        
        struct {
            ASTNode *slice;
        } slice_length_access;
        
        struct {
            ASTNode *target;
            ASTNode *value;
        } assignment;
        

        
        struct {
            ASTNodeList *elements;     // Phase 2.1: Flexible array
        } array_literal;
        
        // Literals
        struct {
            int64_t value;
        } integer_literal;
        
        struct {
            double value;
        } float_literal;
        
        struct {
            char *value;
        } string_literal;
        
        struct {
            bool value;
        } bool_literal;
        
        struct {
            uint32_t value;  // Unicode code point
        } char_literal;
        
        struct {
            // Unit literal () - no data needed
            char _unused;  // Placeholder to avoid empty struct
        } unit_literal;
        
        // Const expression structure (for compile-time evaluation)
        struct {
            ConstExprType expr_type;       // LITERAL, BINARY_OP, UNARY_OP, SIZEOF, IDENTIFIER
            union {
                ASTNode *literal;          // For literal constants
                struct {
                    ASTNode *left;         // Binary operation left operand
                    BinaryOperator op;     // Binary operator
                    ASTNode *right;        // Binary operation right operand
                } binary;
                struct {
                    UnaryOperator op;      // Unary operator
                    ASTNode *operand;      // Unary operation operand
                } unary;
                struct {
                    ASTNode *type;         // sizeof(Type)
                } sizeof_expr;
                char *identifier;          // Reference to another constant
            } data;
        } const_expr;
        
        struct {
            char *struct_name;        // Name of the struct type
            ASTNodeList *type_args;   // Optional type arguments (e.g., <i32> in Vec<i32> { ... }) - Phase 2: Generic Structs
            ASTNodeList *field_inits; // Field initializations (name: value pairs)
        } struct_literal;
        
        struct {
            ASTNodeList *elements;    // List of expressions in the tuple
        } tuple_literal;
        
        struct {
            char *name;
        } identifier;
        
        // Types
        struct {
            char *name;
        } base_type;
        
        struct {
            ASTNode *element_type;
        } slice_type;
        
        struct {
            ASTNode *element_type;
            ASTNode *size;           // Size expression
        } array_type;
        
        struct {
            char *name;
            ASTNodeList *type_args;   // Optional type arguments (e.g., <T> in Vec<T>) - Phase 2: Generic Structs
        } struct_type;
        
        struct {
            char *name;               // Enum name (e.g., "Option", "Result")
            ASTNodeList *type_args;   // Optional type arguments (e.g., <T> in Option<T>)
        } enum_type;
        
        struct {
            bool is_mutable;
            ASTNode *pointee_type;
        } ptr_type;
        
        struct {
            ASTNode *ok_type;
            ASTNode *err_type;
        } result_type;
        
        struct {
            ASTNodeList *element_types;  // List of types in the tuple
        } tuple_type;
        
        // Pattern matching
        struct {
            ASTNode *pattern;
            ASTNode *body;
            ASTNode *guard;           // Optional guard expression
        } match_arm;
        
        struct {
            char *enum_name;
            char *variant_name;
            char *binding;  // Optional variable binding
        } enum_pattern;
        
        // Struct pattern for destructuring: Point { x, y } or Point { x: new_x, y }
        struct {
            char *struct_name;        // Name of the struct type (e.g., "Point")
            ASTNodeList *type_args;   // Optional type arguments (e.g., <T> in Vec<T> { ... }) - Phase 2: Generic Structs
            ASTNodeList *field_patterns; // List of field pattern bindings
            ASTNodeList *fields;      // Alias for field_patterns for compatibility
            bool is_partial;          // true if pattern uses ".." (partial matching)
        } struct_pattern;
        
        // Field pattern for struct destructuring: x, x: new_x, or x: _
        struct {
            char *field_name;         // Name of the field in the struct (e.g., "x")
            char *binding_name;       // Variable name to bind to (e.g., "new_x"), NULL for same as field_name
            bool is_ignored;          // true if pattern is "_" (ignored binding)
            ASTNode *pattern;         // Pattern for this field
        } field_pattern;
        
        // Tuple pattern for destructuring: (pattern1, pattern2, ...)
        struct {
            ASTNodeList *patterns;    // List of patterns for each element
        } tuple_pattern;
        
        // Enum variants
        struct {
            char *enum_name;     // "Result"
            char *variant_name;  // "Ok" or "Err"
            ASTNode *value;      // Optional value expression
        } enum_variant;
        
        // Annotations with flexible arrays - Phase 2.1
        struct {
            char *name;
            ASTNodeList *params;  // Phase 2.1: Flexible array for string params
        } semantic_tag;
        
        struct {
            OwnershipType ownership;
        } ownership_tag;
        
        struct {
            FFITransferType transfer_type;
        } ffi_annotation;
        
        struct {
            SecurityType security_type;
        } security_tag;
        
        struct {
            ReviewPriority priority;
        } human_review_tag;
        
        struct {
            char *struct_name;        // Name of the struct this impl is for
            ASTNodeList *methods;     // List of method declarations
            ASTNodeList *annotations; // Phase 2.1: Flexible array
        } impl_block;
        
        struct {
            char *name;               // Method name
            ASTNodeList *params;      // Parameters (including self if instance method)
            ASTNode *return_type;     // Return type
            ASTNode *body;            // Method body
            bool is_instance_method;  // true if first param is 'self'
            VisibilityType visibility; // NEW: pub or private visibility
            ASTNodeList *annotations; // Phase 2.1: Flexible array
        } method_decl;
        
        // Advanced concurrency structures - Tier 1 (Core & Simple)
        struct {
            char *function_name;
            ASTNodeList *args;
            char *handle_var_name;  // Variable to store the task handle
        } spawn_with_handle_stmt;
        
        struct {
            ASTNode *task_handle_expr;  // Expression that evaluates to a task handle
            ASTNode *timeout_expr;      // Optional timeout expression
        } await_expr;
        
        /*
         * Note: Tier 3 concurrency structures moved to stdlib:
         * - channel_decl, send_stmt, recv_expr  
         * - select_stmt, select_case
         * - worker_pool_decl, close_stmt
         * - channel_type, task_handle_type, worker_pool_type, timeout_expr
         */
    } data;
};

// C17 static assertion for AST node structure size
_Static_assert(sizeof(ASTNode) <= 128, "ASTNode should be reasonably sized for memory efficiency");

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_NODE_H 
