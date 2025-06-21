/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Declarations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains destruction operations for top-level declaration AST nodes
 * Uses common patterns from ast_destruction_common.h
 */

#include "ast_destruction_common.h"
#include "ast_destruction_declarations.h"

void ast_free_declaration_nodes(ASTNode *node) {
    AST_DESTRUCTION_NULL_CHECK(node);
    
    switch (node->type) {
        case AST_PROGRAM:
            AST_RELEASE_NODE(node->data.program.package_decl);
            AST_DESTROY_NODE_LIST(node->data.program.imports);
            AST_DESTROY_NODE_LIST(node->data.program.declarations);
            break;
            
        case AST_PACKAGE_DECL:
            AST_FREE_STRING(node->data.package_decl.name);
            break;
            
        case AST_IMPORT_DECL:
            AST_FREE_STRING(node->data.import_decl.path);
            AST_FREE_STRING(node->data.import_decl.alias);
            break;
            
        case AST_FUNCTION_DECL:
            AST_DESTROY_FUNCTION_DECL(node);
            break;
            
        case AST_STRUCT_DECL:
            AST_DESTROY_STRUCT_DECL(node);
            break;
            
        case AST_STRUCT_FIELD:
            AST_FREE_STRING(node->data.struct_field.name);
            AST_RELEASE_NODE(node->data.struct_field.type);
            break;
            
        case AST_ENUM_DECL:
            AST_DESTROY_ENUM_DECL(node);
            break;
            
        case AST_ENUM_VARIANT_DECL:
            AST_FREE_STRING(node->data.enum_variant_decl.name);
            AST_RELEASE_NODE(node->data.enum_variant_decl.associated_type);
            AST_RELEASE_NODE(node->data.enum_variant_decl.value);
            break;
            
        case AST_EXTERN_DECL:
            AST_FREE_STRING(node->data.extern_decl.name);
            AST_FREE_STRING(node->data.extern_decl.extern_name);
            AST_DESTROY_NODE_LIST(node->data.extern_decl.params);
            AST_RELEASE_NODE(node->data.extern_decl.return_type);
            AST_DESTROY_NODE_LIST(node->data.extern_decl.annotations);
            break;
            
        case AST_PARAM_DECL:
            AST_FREE_STRING(node->data.param_decl.name);
            AST_RELEASE_NODE(node->data.param_decl.type);
            AST_DESTROY_NODE_LIST(node->data.param_decl.annotations);
            break;
            
        case AST_IMPL_BLOCK:
            AST_FREE_STRING(node->data.impl_block.struct_name);
            AST_DESTROY_NODE_LIST(node->data.impl_block.methods);
            AST_DESTROY_NODE_LIST(node->data.impl_block.annotations);
            break;
            
        case AST_METHOD_DECL:
            AST_FREE_STRING(node->data.method_decl.name);
            AST_DESTROY_NODE_LIST(node->data.method_decl.params);
            AST_RELEASE_NODE(node->data.method_decl.return_type);
            AST_RELEASE_NODE(node->data.method_decl.body);
            AST_DESTROY_NODE_LIST(node->data.method_decl.annotations);
            break;
            
        default:
            // Not a declaration node
            break;
    }
} 
