/**
 * AST-related stub functions for comprehensive tests
 */

#include <stdbool.h>
#include <stdlib.h>

// Forward declarations
typedef struct SourceLocation {
    int line;
    int column;
    const char* file;
} SourceLocation;

typedef struct ASTNode {
    int type;
    SourceLocation location;
    void* data;
} ASTNode;

// AST functions
void ast_free_node(ASTNode* node) {
    free(node);
}

ASTNode* create_identifier(const char* name, SourceLocation loc) {
    (void)name;
    (void)loc;
    return malloc(sizeof(ASTNode));
}

SourceLocation current_location(void* lexer) {
    (void)lexer;
    SourceLocation loc = {0};
    return loc;
}

// Optimization functions
void* optimize_ast(void* ast) {
    return ast;
}

void* optimize_ir(void* ir) {
    return ir;
}