/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) - Main Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main AST header that includes all AST components
 * Split into modular components for better organization and faster compilation
 */

#ifndef ASTHRA_AST_H
#define ASTHRA_AST_H

// Include all AST components in proper order
#include "ast_generic.h"    // C17 generic type-safe operations
#include "ast_node.h"       // ASTNode and ASTNodeList structures
#include "ast_node_list.h"  // AST node list operations
#include "ast_operations.h" // Traditional AST interface
#include "ast_types.h"      // Core types, enums, forward declarations

#ifdef __cplusplus
extern "C" {
#endif

// No additional content needed - all functionality is provided by included headers

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_H
