/**
 * Asthra Programming Language Compiler
 * AST Types and Enumerations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core AST types, enums, and forward declarations
 */

#ifndef ASTHRA_AST_TYPES_H
#define ASTHRA_AST_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for AST assumptions
_Static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit for AST node counts");
_Static_assert(sizeof(void*) >= sizeof(uint32_t), "Pointer size must be at least 32-bit for AST references");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for AST modernization");

// Forward declarations
typedef struct ASTNode ASTNode;
typedef struct TypeInfo TypeInfo;
typedef struct ASTNodeList ASTNodeList;
typedef struct SymbolEntry SymbolEntry;

// C17 atomic reference counting for AST nodes
typedef atomic_uint_fast32_t ast_atomic_refcount_t;

// Const expression types for compile-time evaluation
typedef enum {
    CONST_EXPR_LITERAL,        // Literal values
    CONST_EXPR_IDENTIFIER,     // Reference to another constant
    CONST_EXPR_BINARY_OP,      // Binary operations
    CONST_EXPR_UNARY_OP,       // Unary operations
    CONST_EXPR_SIZEOF          // sizeof expressions
} ConstExprType;

// AST node types
typedef enum {
    // Program structure
    AST_PROGRAM,
    AST_PACKAGE_DECL,
    AST_IMPORT_DECL,
    AST_VISIBILITY_MODIFIER,  // New: pub/private visibility
    
    // Declarations
    AST_FUNCTION_DECL,
    AST_STRUCT_DECL,
    AST_STRUCT_FIELD,             // New: struct field declaration with visibility
    AST_ENUM_DECL,                // Enum declaration
    AST_EXTERN_DECL,
    AST_PARAM_DECL,
    AST_CONST_DECL,               // Const declaration for compile-time constants
    AST_IMPL_BLOCK,           // impl block for struct methods
    AST_METHOD_DECL,          // method declaration within impl block
    
    // Statements
    AST_BLOCK,
    AST_EXPR_STMT,
    AST_LET_STMT,
    AST_RETURN_STMT,
    AST_IF_STMT,
    AST_FOR_STMT,
    AST_MATCH_STMT,
    AST_IF_LET_STMT,
    AST_SPAWN_STMT,
    AST_UNSAFE_BLOCK,
    AST_BREAK_STMT,
    AST_CONTINUE_STMT,
    
    // Advanced Concurrency Statements and Expressions
    AST_SPAWN_WITH_HANDLE_STMT,   // spawn_with_handle variable = function(args)
    AST_AWAIT_EXPR,               // await handle [timeout(duration)]
    
    // Expressions
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_POSTFIX_EXPR,
    AST_CALL_EXPR,
    AST_ASSOCIATED_FUNC_CALL,     // New: for StructName::function_name() calls
    AST_FIELD_ACCESS,
    AST_INDEX_ACCESS,
    AST_SLICE_EXPR,               // Slice expression: array[start:end]
    AST_SLICE_LENGTH_ACCESS,
    AST_ASSIGNMENT,
    AST_ARRAY_LITERAL,
    AST_CONST_EXPR,               // Compile-time constant expressions
    
    // Literals
    AST_INTEGER_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_BOOL_LITERAL,
    AST_BOOLEAN_LITERAL,      // Alias for AST_BOOL_LITERAL for compatibility
    AST_CHAR_LITERAL,
    AST_UNIT_LITERAL,         // Unit literal () for void returns
    AST_STRUCT_LITERAL,
    AST_TUPLE_LITERAL,        // Tuple literals: (expr1, expr2, ...)
    AST_IDENTIFIER,
    
    // Types
    AST_BASE_TYPE,
    AST_SLICE_TYPE,
    AST_ARRAY_TYPE,               // Fixed-size array type [T; size]
    AST_STRUCT_TYPE,
    AST_ENUM_TYPE,                // New: enum type with optional type arguments
    AST_PTR_TYPE,
    AST_RESULT_TYPE,
    AST_TUPLE_TYPE,           // Tuple types: (T1, T2, ...)
    
    // Pattern matching
    AST_MATCH_ARM,
    AST_PATTERN,
    AST_ENUM_PATTERN,
    AST_STRUCT_PATTERN,       // New: for struct destructuring patterns
    AST_TUPLE_PATTERN,        // Tuple patterns: (pattern1, pattern2, ...)
    AST_FIELD_PATTERN,        // New: for individual field patterns in struct destructuring
    AST_WILDCARD_PATTERN,     // New: for wildcard patterns (_)
    
    // Enum variants
    AST_ENUM_VARIANT,
    AST_ENUM_VARIANT_DECL,        // Enum variant declaration (within enum declaration)
    
    // Annotations
    AST_SEMANTIC_TAG,
    AST_OWNERSHIP_TAG,
    AST_FFI_ANNOTATION,
    AST_SECURITY_TAG,
    AST_HUMAN_REVIEW_TAG,
    
    // Phase 2.1: Sentinel value for validation
    AST_NODE_TYPE_COUNT
} ASTNodeType;

// C17 static assertion for AST node type enum
_Static_assert(AST_NODE_TYPE_COUNT <= 256, "AST node types must fit in uint8_t for compact storage");

// Binary operators
typedef enum {
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_EQ,
    BINOP_NE,
    BINOP_LT,
    BINOP_LE,
    BINOP_GT,
    BINOP_GE,
    BINOP_AND,
    BINOP_OR,
    BINOP_BITWISE_AND,
    BINOP_BITWISE_OR,
    BINOP_BITWISE_XOR,
    BINOP_LSHIFT,
    BINOP_RSHIFT,
    BINOP_COUNT  // Phase 2.1: For validation
} BinaryOperator;

// Unary operators
typedef enum {
    UNOP_MINUS,
    UNOP_NOT,
    UNOP_BITWISE_NOT,
    UNOP_DEREF,
    UNOP_ADDRESS_OF,
    UNOP_SIZEOF,
    UNOP_COUNT  // Phase 2.1: For validation
} UnaryOperator;

// Ownership types for annotations
typedef enum {
    OWNERSHIP_GC,
    OWNERSHIP_C,
    OWNERSHIP_PINNED,
    OWNERSHIP_COUNT  // Phase 2.1: For validation
} OwnershipType;

// FFI transfer types
typedef enum {
    FFI_TRANSFER_FULL,
    FFI_TRANSFER_NONE,
    FFI_BORROWED,
    FFI_TRANSFER_COUNT  // Phase 2.1: For validation
} FFITransferType;

// Security annotation types
typedef enum {
    SECURITY_CONSTANT_TIME,
    SECURITY_VOLATILE_MEMORY,
    SECURITY_COUNT  // Phase 2.1: For validation
} SecurityType;

// Human review priority
typedef enum {
    REVIEW_NONE,
    REVIEW_LOW,
    REVIEW_MEDIUM,
    REVIEW_HIGH,
    REVIEW_CRITICAL,
    REVIEW_COUNT
} ReviewPriority;

// Visibility types for package system
typedef enum {
    VISIBILITY_PRIVATE,
    VISIBILITY_PUBLIC,
    VISIBILITY_COUNT  // For validation
} VisibilityType;

// C17 static assertions for enum ranges
_Static_assert(BINOP_COUNT <= 32, "Binary operators must fit in compact representation");
_Static_assert(UNOP_COUNT <= 16, "Unary operators must fit in compact representation");
_Static_assert(OWNERSHIP_COUNT <= 8, "Ownership types must fit in compact representation");

// Note: The full ASTNode structure definition is in ast_node.h
// This file only contains the type definitions and forward declarations

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_TYPES_H 
