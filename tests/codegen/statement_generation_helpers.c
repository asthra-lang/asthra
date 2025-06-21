/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Helpers
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Helper functions for statement generation tests
 */

#include "semantic_analyzer.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>

/**
 * Setup a semantic analyzer for tests
 */
SemanticAnalyzer* setup_semantic_analyzer(void) {
    return semantic_analyzer_create();
}

/**
 * Destroy a semantic analyzer
 */
void destroy_semantic_analyzer(SemanticAnalyzer* analyzer) {
    semantic_analyzer_destroy(analyzer);
}

/**
 * Parse test source code
 */
ASTNode* parse_test_source(const char* source, const char* filename) {
    Lexer* lexer = lexer_create(source, strlen(source), filename);
    if (!lexer) return NULL;
    
    Parser* parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    ASTNode* program = parser_parse_program(parser);
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return program;
}