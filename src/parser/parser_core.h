/**
 * Asthra Programming Language Compiler
 * Core parser types and main interface
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_CORE_H
#define ASTHRA_PARSER_CORE_H

#include "lexer.h"
#include "ast.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for parser assumptions
_Static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t must be at least 32-bit for parser state");
_Static_assert(__STDC_VERSION__ >= 201710L, "C17 standard required for parser modernization");

// Forward declarations
typedef struct Parser Parser;
typedef struct SymbolTable SymbolTable;

// Parse context enumeration for enhanced parsing
typedef enum {
    PARSE_CONTEXT_EXPRESSION,
    PARSE_CONTEXT_LVALUE,
    PARSE_CONTEXT_TYPE,
    PARSE_CONTEXT_PATTERN,
    PARSE_CONTEXT_ANNOTATION,
    PARSE_CONTEXT_STATEMENT,
    PARSE_CONTEXT_DECLARATION,
    PARSE_CONTEXT_COUNT
} ParseContext;

// C17 static assertion for parse context enum
_Static_assert(PARSE_CONTEXT_COUNT <= 16, "Parse contexts must fit in compact representation");

// Atomic parser statistics for thread-safe parsing
typedef struct ParseStatistics {
    atomic_uint_fast64_t nodes_created;
    atomic_uint_fast64_t tokens_consumed;
    atomic_uint_fast64_t errors_reported;
    atomic_uint_fast64_t warnings_reported;
    atomic_uint_fast64_t parse_time_ms;
    atomic_uint_fast32_t max_recursion_depth;
    atomic_uint_fast32_t current_recursion_depth;
} ParseStatistics;

// Parser configuration with C17 designated initializers
typedef struct ParserConfig {
    bool allow_incomplete_parse;
    bool strict_mode;
    bool enable_recovery;
    bool collect_statistics;
    size_t max_errors;
    size_t max_recursion_depth;
    size_t initial_token_buffer_size;
} ParserConfig;

// Default parser configuration using C17 compound literal
#define PARSER_DEFAULT_CONFIG ((ParserConfig){ \
    .allow_incomplete_parse = false, \
    .strict_mode = true, \
    .enable_recovery = true, \
    .collect_statistics = true, \
    .max_errors = 100, \
    .max_recursion_depth = 1000, \
    .initial_token_buffer_size = 256 \
})

// Parse error information with enhanced C17 features
typedef struct ParseError {
    SourceLocation location;
    char *message;
    bool is_warning;
    uint32_t error_code;
    struct ParseError *next;
    
    // Enhanced error context
    struct {
        TokenType expected_token;
        TokenType actual_token;
        ASTNodeType expected_node_type;
        const char *context_description;
    } context;
} ParseError;

// C17 static assertion for parse error structure
_Static_assert(sizeof(ParseError) <= 128, "ParseError should be reasonably sized");

// Parser state with C17 enhancements
struct Parser {
    Lexer *lexer;
    Token current_token;
    ParseError *errors;
    ParseError *last_error;
    size_t error_count;
    bool panic_mode;
    bool had_error;
    SymbolTable *symbol_table;
    
    // Enhanced parser state
    ParserConfig config;
    ParseStatistics stats;
    uint32_t current_recursion_depth;
    
    // Token lookahead buffer with flexible array
    struct {
        size_t capacity;
        size_t count;
        size_t head;
        Token tokens[];  // C17 flexible array member
    } *token_buffer;
};

// C17 static assertion for parser structure
_Static_assert(offsetof(Parser, token_buffer) > 0, "Parser structure layout must be valid");

// =============================================================================
// MAIN PARSER INTERFACE
// =============================================================================

// Create and initialize parser with configuration
Parser *parser_create_with_config(Lexer *lexer, ParserConfig config);
Parser *parser_create(Lexer *lexer);  // Uses default config

// Destroy parser and free resources
void parser_destroy(Parser *parser);

// Parse complete program with enhanced error handling
ASTNode *parser_parse_program(Parser *parser);

// Parse individual constructs (for testing and incremental parsing)
ASTNode *parser_parse_expression(Parser *parser);
ASTNode *parser_parse_statement(Parser *parser);
ASTNode *parser_parse_declaration(Parser *parser);
ASTNode *parser_parse_type(Parser *parser);

// Parse specific declaration types
ASTNode *parse_function_decl(Parser *parser);
ASTNode *parse_struct_decl(Parser *parser);
ASTNode *parse_extern_decl(Parser *parser);
ASTNode *parse_impl_block(Parser *parser);
ASTNode *parse_method_decl(Parser *parser);
ASTNode *parse_trait_decl(Parser *parser);
ASTNode *parse_impl_decl(Parser *parser);

// Enhanced parsing with context
ASTNode *parser_parse_in_context(Parser *parser, ParseContext context);
ASTNode *parser_parse_with_recovery(Parser *parser, ASTNodeType expected_type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_CORE_H 
