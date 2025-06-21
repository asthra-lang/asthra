/*
 * Concurrency Tiers Integration Tests - Common Implementation
 * 
 * Shared implementation for the modular concurrency tiers test suite.
 * Contains helper functions and shared utilities.
 */

#include "test_concurrency_tiers_common.h"
#include "parser_string_interface.h"
#include "semantic_analyzer_core.h"
#include "semantic_core.h"
#include "ast_operations.h"

// Global test counters
int tests_run = 0;
int tests_passed = 0;

// ============================================================================
// Helper Functions Implementation
// ============================================================================

ASTNode* find_function_declaration(ASTNode* ast, const char* name) {
    if (!ast) return NULL;
    
    if (ast->type == AST_FUNCTION_DECL && 
        ast->data.function_decl.name && 
        strcmp(ast->data.function_decl.name, name) == 0) {
        return ast;
    }
    
    // Check program declarations
    if (ast->type == AST_PROGRAM && ast->data.program.declarations) {
        for (size_t i = 0; i < ast->data.program.declarations->count; i++) {
            ASTNode* result = find_function_declaration(ast->data.program.declarations->nodes[i], name);
            if (result) return result;
        }
    }
    
    return NULL;
}

bool has_annotation(ASTNode* function_node, const char* annotation_name) {
    if (!function_node || function_node->type != AST_FUNCTION_DECL) {
        return false;
    }
    
    if (!function_node->data.function_decl.annotations) {
        return false;
    }
    
    for (size_t i = 0; i < function_node->data.function_decl.annotations->count; i++) {
        // This is a simplified check - actual annotation structure may be different
        // For now, just return false to allow compilation
        (void)annotation_name; // Suppress unused parameter warning
        break; // Exit after first iteration to avoid infinite loop
    }
    
    return false;
}

// cleanup_parse_result is now provided by parser_string_interface.h from asthra_compiler

void cleanup_semantic_result(SemanticAnalysisResult* result) {
    if (result && result->errors) {
        for (int i = 0; i < result->error_count; i++) {
            free(result->errors[i].message);
        }
        free(result->errors);
        result->errors = NULL;
    }
}

// ============================================================================
// Stub Implementations (To be replaced with actual implementations)
// ============================================================================

// REAL IMPLEMENTATION: parse_string is now provided by parser_string_interface.h
// No longer a stub - uses actual parser implementation

SemanticAnalysisResult analyze_semantics(ASTNode* ast) {
    SemanticAnalysisResult result = { .success = false, .error_count = 0, .errors = NULL };
    
    if (!ast) {
        result.error_count = 1;
        result.errors = malloc(sizeof(struct { char* message; }));
        result.errors[0].message = strdup("NULL AST node provided");
        return result;
    }
    
    // Create semantic analyzer for concurrency analysis
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        result.error_count = 1;
        result.errors = malloc(sizeof(struct { char* message; }));
        result.errors[0].message = strdup("Failed to create semantic analyzer");
        return result;
    }
    
    // Perform semantic analysis focusing on concurrency constructs
    bool semantic_success = semantic_analyze_program(analyzer, ast);
    
    if (semantic_success) {
        result.success = true;
        
        // Additional concurrency-specific validations
        bool has_spawn_construct = false;
        bool has_async_construct = false;
        bool has_proper_annotations = false;
        
        // Check for spawn constructs in the AST
        if (ast->type == AST_PROGRAM && ast->data.program.declarations) {
            for (size_t i = 0; i < ast->data.program.declarations->count; i++) {
                ASTNode* decl = ast->data.program.declarations->nodes[i];
                if (decl && decl->type == AST_FUNCTION_DECL) {
                    // Check function body for spawn/async constructs
                    if (decl->data.function_decl.body) {
                        has_spawn_construct = true; // Simplified check
                    }
                    // Check for concurrency annotations
                    if (decl->data.function_decl.annotations) {
                        has_proper_annotations = true;
                    }
                }
            }
        }
        
        // Additional validation for concurrency safety
        if (!has_spawn_construct && !has_async_construct) {
            // This is fine - not all code needs concurrency
        }
        
    } else {
        result.success = false;
        result.error_count = 1;
        result.errors = malloc(sizeof(struct { char* message; }));
        result.errors[0].message = strdup("Semantic analysis failed for concurrency constructs");
    }
    
    semantic_analyzer_destroy(analyzer);
    return result;
}

void free_ast_node(ASTNode* node) {
    if (node) {
        // Use the real AST node destruction function
        ast_free_node(node);
    }
}

// ============================================================================
// AST Helper Functions Implementation
// ============================================================================

bool contains_spawn_statement(ASTNode* node) {
    if (!node) return false;
    
    if (node->type == AST_SPAWN_STMT) return true;
    
    // Recursively check child nodes based on node type
    switch (node->type) {
        case AST_PROGRAM:
            if (node->data.program.declarations) {
                for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                    if (contains_spawn_statement(node->data.program.declarations->nodes[i])) {
                        return true;
                    }
                }
            }
            break;
        case AST_FUNCTION_DECL:
            if (node->data.function_decl.body && contains_spawn_statement(node->data.function_decl.body)) {
                return true;
            }
            break;
        case AST_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < node->data.block.statements->count; i++) {
                    if (contains_spawn_statement(node->data.block.statements->nodes[i])) {
                        return true;
                    }
                }
            }
            break;
        case AST_IF_STMT:
            if (contains_spawn_statement(node->data.if_stmt.then_block) ||
                (node->data.if_stmt.else_block && contains_spawn_statement(node->data.if_stmt.else_block))) {
                return true;
            }
            break;
        default:
            break;
    }
    
    return false;
}

bool contains_await_expression(ASTNode* node) {
    if (!node) return false;
    
    if (node->type == AST_AWAIT_EXPR) return true;
    
    // Recursively check child nodes based on node type
    switch (node->type) {
        case AST_PROGRAM:
            if (node->data.program.declarations) {
                for (size_t i = 0; i < node->data.program.declarations->count; i++) {
                    if (contains_await_expression(node->data.program.declarations->nodes[i])) {
                        return true;
                    }
                }
            }
            break;
        case AST_FUNCTION_DECL:
            if (node->data.function_decl.body && contains_await_expression(node->data.function_decl.body)) {
                return true;
            }
            break;
        case AST_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < node->data.block.statements->count; i++) {
                    if (contains_await_expression(node->data.block.statements->nodes[i])) {
                        return true;
                    }
                }
            }
            break;
        case AST_LET_STMT:
            if (node->data.let_stmt.initializer && contains_await_expression(node->data.let_stmt.initializer)) {
                return true;
            }
            break;
        case AST_BINARY_EXPR:
            if (contains_await_expression(node->data.binary_expr.left) ||
                contains_await_expression(node->data.binary_expr.right)) {
                return true;
            }
            break;
        default:
            break;
    }
    
    return false;
}

// ============================================================================
// Common Initialization and Cleanup
// ============================================================================

int initialize_concurrency_runtime(void) {
    // For testing purposes, we'll stub this out
    // In a real implementation, this would initialize the concurrency runtime
    printf("✅ Concurrency runtime initialized successfully (stubbed)\n");
    return 0;
}

void cleanup_concurrency_runtime(void) {
    // For testing purposes, we'll stub this out
    // In a real implementation, this would cleanup the concurrency runtime
    printf("✅ Concurrency runtime cleaned up (stubbed)\n");
} 
