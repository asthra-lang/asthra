# Asthra Lexer and Parser Architecture

**Version:** 1.0  
**Date:** January 7, 2025  
**Status:** Complete  

## Table of Contents

1\. &#91;Overview&#93;(#overview)
2\. &#91;Lexer Architecture&#93;(#lexer-architecture)
3\. &#91;Parser Architecture&#93;(#parser-architecture)
4\. &#91;AST System&#93;(#ast-system)
5\. &#91;Integration Patterns&#93;(#integration-patterns)
6\. &#91;Error Handling&#93;(#error-handling)
7\. &#91;Memory Management&#93;(#memory-management)
8\. &#91;Performance Characteristics&#93;(#performance-characteristics)
9\. &#91;Development Guidelines&#93;(#development-guidelines)

## Overview

The Asthra lexer and parser form the front-end of the compilation pipeline, transforming source code into an Abstract Syntax Tree (AST). The design emphasizes modularity, AI-friendly patterns, and robust error handling while maintaining high performance.

### Key Design Principles

1\. **Modular Architecture**: Lexer and parser are split into focused, single-responsibility modules
2\. **AI Generation Efficiency**: Grammar designed to minimize ambiguity for AI code generation
3\. **Memory Safety**: Careful memory management with clear ownership patterns
4\. **Error Recovery**: Robust error handling with helpful diagnostic messages
5\. **Performance**: Efficient single-pass lexing and recursive descent parsing

### Compilation Pipeline Position

```
Source Code → &#91;LEXER&#93; → Token Stream → &#91;PARSER&#93; → AST → Semantic Analysis
```

## Lexer Architecture

The lexer transforms source code into a stream of tokens using a modular, single-pass design.

### Core Components

#### File Structure
```
src/lexer/
├── lexer.h              # Main lexer interface
├── lexer_core.c         # Core lexing logic
├── lexer_scan.c         # Token scanning functions
├── lexer_util.c         # Utility functions
├── lexer_internal.h     # Internal definitions
├── token.h              # Token type definitions
├── token.c              # Token management
└── keyword.c            # Keyword recognition
```

#### Token System

**Token Types** (`src/lexer/token.h`):
```c
typedef enum {
    // Literals
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,
    
    // Identifiers and Keywords
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    
    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_XOR,
    TOKEN_LEFT_SHIFT,
    TOKEN_RIGHT_SHIFT,
    
    // Delimiters
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_COLON,
    TOKEN_DOUBLE_COLON,
    TOKEN_ARROW,
    
    // Special
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_NEWLINE,
    TOKEN_QUESTION_MARK,
} TokenType;
```

**Token Structure**:
```c
typedef struct {
    TokenType type;
    char* value;           // Token text (dynamically allocated)
    size_t length;         // Length of token text
    size_t line;           // Line number (1-based)
    size_t column;         // Column number (1-based)
    char* filename;        // Source filename
} Token;
```

#### Lexer State Management

**Lexer Context** (`src/lexer/lexer_internal.h`):
```c
typedef struct {
    const char* source;    // Source code text
    size_t length;         // Total source length
    size_t position;       // Current position
    size_t line;           // Current line (1-based)
    size_t column;         // Current column (1-based)
    char* filename;        // Source filename
    Token current_token;   // Current token
    Token peek_token;      // Lookahead token
    bool has_peek;         // Whether peek token is valid
} LexerContext;
```

#### Scanning Algorithms

**Number Literal Scanning** (`src/lexer/lexer_scan.c`):
```c
// Supports multiple number formats
Token scan_number(LexerContext* ctx) {
    if (ctx->source&#91;ctx->position&#93; == '0') {
        char next = ctx->source&#91;ctx->position + 1&#93;;
        if (next == 'x' || next == 'X') {
            return scan_hex_literal(ctx);      // 0xFF, 0xDEADBEEF
        } else if (next == 'b' || next == 'B') {
            return scan_binary_literal(ctx);   // 0b1010, 0b11111111
        } else if (next == 'o' || next == 'O') {
            return scan_octal_literal(ctx);    // 0o755, 0o644
        }
    }
    return scan_decimal_literal(ctx);          // 42, 3.14159
}
```

**String Literal Scanning**:
```c
Token scan_string(LexerContext* ctx) {
    // Handles escape sequences: \n, \t, \r, \\, \", \'
    // Supports multi-line strings
    // Validates UTF-8 encoding
    // Reports unclosed string errors
}
```

**Identifier and Keyword Recognition**:
```c
Token scan_identifier(LexerContext* ctx) {
    // Scans &#91;a-zA-Z_&#93;&#91;a-zA-Z0-9_&#93;*
    // Checks against keyword table using Gperf hash
    // Returns TOKEN_KEYWORD or TOKEN_IDENTIFIER
}
```

#### Keyword System

**Keyword Recognition** (`src/parser/keyword.c`):
- Uses Gperf-generated perfect hash function for O(1) keyword lookup
- Supports 42+ Asthra keywords including `pub`, `priv`, `struct`, `enum`, `fn`, `let`, `match`, `spawn`, `await`
- Automatically generated from grammar specification
- Case-sensitive matching

### Lexer API

#### Core Functions

```c
// Lexer lifecycle
LexerContext* lexer_create(const char* source, const char* filename);
void lexer_destroy(LexerContext* ctx);

// Token access
Token lexer_next_token(LexerContext* ctx);
Token lexer_peek_token(LexerContext* ctx);
Token lexer_current_token(LexerContext* ctx);

// Position management
bool lexer_at_end(LexerContext* ctx);
size_t lexer_get_line(LexerContext* ctx);
size_t lexer_get_column(LexerContext* ctx);

// Error handling
bool lexer_has_error(LexerContext* ctx);
const char* lexer_get_error(LexerContext* ctx);
```

#### Memory Management

**Token Memory Ownership**:
- `lexer_next_token()` returns cloned tokens with separate memory
- Caller responsible for calling `token_destroy()` on returned tokens
- Internal lexer state manages its own token memory
- Prevents double-free errors through careful ownership design

```c
// Safe token usage pattern
Token token = lexer_next_token(ctx);
// Use token...
token_destroy(&amp;token);  // Caller must free
```

## Parser Architecture

The parser implements a recursive descent parser that transforms the token stream into an AST following the Asthra PEG grammar.

### Core Components

#### File Structure
```
src/parser/
├── parser.h                           # Main parser interface
├── parser_core.c                      # Core parsing logic
├── parser_errors.c                    # Error handling and reporting
├── parser_util.c                      # Utility functions
├── grammar_expressions.c              # Expression parsing
├── grammar_statements.c               # Statement parsing
├── grammar_declarations.c             # Declaration parsing
├── grammar_types.c                    # Type parsing
├── grammar_patterns.c                 # Pattern parsing
├── grammar_toplevel_program.c         # Top-level program parsing
├── grammar_toplevel_function.c        # Function declaration parsing
├── grammar_toplevel_struct.c          # Struct declaration parsing
├── grammar_toplevel_enum.c            # Enum declaration parsing
├── grammar_toplevel_import.c          # Import declaration parsing
└── grammar_annotations.c              # Annotation parsing
```

#### Parser State Management

**Parser Context** (`src/parser/parser.h`):
```c
typedef struct {
    LexerContext* lexer;           // Lexer for token stream
    Token current_token;           // Current token being parsed
    bool has_error;                // Error state flag
    char* error_message;           // Current error message
    size_t error_line;             // Error location
    size_t error_column;
    ASTNode* root;                 // Root of parsed AST
    MemoryPool* ast_pool;          // Memory pool for AST nodes
} ParserContext;
```

#### Grammar Implementation

The parser directly implements the Asthra PEG grammar through recursive descent functions:

**Expression Parsing** (`src/parser/grammar_expressions.c`):
```c
// Operator precedence hierarchy (lowest to highest)
ASTNode* parse_expression(ParserContext* ctx);           // Entry point
ASTNode* parse_logical_or(ParserContext* ctx);           // ||
ASTNode* parse_logical_and(ParserContext* ctx);          // &amp;&amp;
ASTNode* parse_bitwise_or(ParserContext* ctx);           // |
ASTNode* parse_bitwise_xor(ParserContext* ctx);          // ^
ASTNode* parse_bitwise_and(ParserContext* ctx);          // &amp;
ASTNode* parse_equality(ParserContext* ctx);             // == !=
ASTNode* parse_relational(ParserContext* ctx);           // &lt; &gt; &lt;= &gt;=
ASTNode* parse_shift(ParserContext* ctx);                // &lt;&lt; &gt;&gt;
ASTNode* parse_additive(ParserContext* ctx);             // + -
ASTNode* parse_multiplicative(ParserContext* ctx);       // * / %
ASTNode* parse_unary(ParserContext* ctx);                // ! - &amp; *
ASTNode* parse_postfix(ParserContext* ctx);              // . &#91;&#93; () ::
ASTNode* parse_primary(ParserContext* ctx);              // literals, identifiers, ()
```

**Statement Parsing** (`src/parser/grammar_statements.c`):
```c
ASTNode* parse_statement(ParserContext* ctx);
ASTNode* parse_let_stmt(ParserContext* ctx);
ASTNode* parse_assignment_stmt(ParserContext* ctx);
ASTNode* parse_if_stmt(ParserContext* ctx);
ASTNode* parse_match_stmt(ParserContext* ctx);
ASTNode* parse_for_stmt(ParserContext* ctx);
ASTNode* parse_while_stmt(ParserContext* ctx);
ASTNode* parse_return_stmt(ParserContext* ctx);
ASTNode* parse_break_stmt(ParserContext* ctx);
ASTNode* parse_continue_stmt(ParserContext* ctx);
ASTNode* parse_block_stmt(ParserContext* ctx);
ASTNode* parse_expression_stmt(ParserContext* ctx);
```

**Declaration Parsing**:
```c
ASTNode* parse_function_decl(ParserContext* ctx);
ASTNode* parse_struct_decl(ParserContext* ctx);
ASTNode* parse_enum_decl(ParserContext* ctx);
ASTNode* parse_import_decl(ParserContext* ctx);
```

#### Grammar Rule Mapping

The parser functions directly correspond to PEG grammar rules:

```c
// Grammar Rule: Primary &lt;- Literal / Identifier / '(' Expression ')'
ASTNode* parse_primary(ParserContext* ctx) {
    if (is_literal_token(ctx->current_token.type)) {
        return parse_literal(ctx);
    } else if (ctx->current_token.type == TOKEN_IDENTIFIER) {
        return parse_identifier(ctx);
    } else if (ctx->current_token.type == TOKEN_LEFT_PAREN) {
        consume_token(ctx, TOKEN_LEFT_PAREN);
        ASTNode* expr = parse_expression(ctx);
        consume_token(ctx, TOKEN_RIGHT_PAREN);
        return expr;
    } else {
        parser_error(ctx, "Expected literal, identifier, or '('");
        return NULL;
    }
}
```

### Parser API

#### Core Functions

```c
// Parser lifecycle
ParserContext* parser_create(LexerContext* lexer);
void parser_destroy(ParserContext* ctx);

// Parsing
ASTNode* parser_parse_program(ParserContext* ctx);
ASTNode* parser_parse_expression(ParserContext* ctx);
ASTNode* parser_parse_statement(ParserContext* ctx);

// Error handling
bool parser_has_error(ParserContext* ctx);
const char* parser_get_error(ParserContext* ctx);
void parser_get_error_location(ParserContext* ctx, size_t* line, size_t* column);

// Utility
void parser_advance(ParserContext* ctx);
bool parser_match(ParserContext* ctx, TokenType type);
bool parser_consume(ParserContext* ctx, TokenType type);
```

## AST System

The Abstract Syntax Tree represents the parsed program structure using a type-safe node system.

### AST Node Types

**Node Type Enumeration** (`src/ast/ast_nodes.h`):
```c
typedef enum {
    // Literals
    AST_INTEGER_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_BOOLEAN_LITERAL,
    
    // Identifiers and Types
    AST_IDENTIFIER,
    AST_TYPE_REFERENCE,
    AST_GENERIC_TYPE,
    
    // Expressions
    AST_BINARY_EXPRESSION,
    AST_UNARY_EXPRESSION,
    AST_CALL_EXPRESSION,
    AST_FIELD_ACCESS,
    AST_INDEX_ACCESS,
    AST_CAST_EXPRESSION,
    AST_MATCH_EXPRESSION,
    
    // Statements
    AST_LET_STATEMENT,
    AST_ASSIGNMENT_STATEMENT,
    AST_IF_STATEMENT,
    AST_MATCH_STATEMENT,
    AST_FOR_STATEMENT,
    AST_WHILE_STATEMENT,
    AST_RETURN_STATEMENT,
    AST_BREAK_STATEMENT,
    AST_CONTINUE_STATEMENT,
    AST_BLOCK_STATEMENT,
    AST_EXPRESSION_STATEMENT,
    
    // Declarations
    AST_FUNCTION_DECLARATION,
    AST_STRUCT_DECLARATION,
    AST_ENUM_DECLARATION,
    AST_IMPORT_DECLARATION,
    AST_FIELD_DECLARATION,
    AST_PARAMETER_DECLARATION,
    
    // Patterns
    AST_WILDCARD_PATTERN,
    AST_LITERAL_PATTERN,
    AST_IDENTIFIER_PATTERN,
    AST_STRUCT_PATTERN,
    AST_ENUM_PATTERN,
    AST_TUPLE_PATTERN,
    
    // Program Structure
    AST_PROGRAM,
    AST_PACKAGE_DECLARATION
} ASTNodeType;
```

### AST Node Structure

**Base Node Structure**:
```c
typedef struct ASTNode {
    ASTNodeType type;              // Node type discriminator
    size_t line;                   // Source line number
    size_t column;                 // Source column number
    char* filename;                // Source filename
    
    // Type information (filled by semantic analysis)
    TypeInfo* type_info;
    
    // Node-specific data (union based on type)
    union {
        // Literals
        struct {
            int64_t value;
        } integer_literal;
        
        struct {
            double value;
        } float_literal;
        
        struct {
            char* value;
            size_t length;
        } string_literal;
        
        // Expressions
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            TokenType operator;
        } binary_expression;
        
        struct {
            struct ASTNode* operand;
            TokenType operator;
        } unary_expression;
        
        struct {
            struct ASTNode* function;
            struct ASTNode** arguments;
            size_t argument_count;
        } call_expression;
        
        // Statements
        struct {
            char* name;
            struct ASTNode* type;
            struct ASTNode* initializer;
            bool is_mutable;
        } let_statement;
        
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_statement;
        
        // Declarations
        struct {
            char* name;
            struct ASTNode** parameters;
            size_t parameter_count;
            struct ASTNode* return_type;
            struct ASTNode* body;
            VisibilityModifier visibility;
            struct ASTNode** annotations;
            size_t annotation_count;
        } function_declaration;
        
        struct {
            char* name;
            struct ASTNode** fields;
            size_t field_count;
            struct ASTNode** type_parameters;
            size_t type_parameter_count;
            VisibilityModifier visibility;
        } struct_declaration;
        
        // Program structure
        struct {
            struct ASTNode** declarations;
            size_t declaration_count;
            struct ASTNode* package_decl;
        } program;
    } data;
} ASTNode;
```

### AST Construction

**Node Creation Functions** (`src/ast/ast_nodes.c`):
```c
// Generic node creation
ASTNode* ast_node_create(ASTNodeType type, size_t line, size_t column);

// Specific node creators
ASTNode* ast_create_integer_literal(int64_t value, size_t line, size_t column);
ASTNode* ast_create_binary_expression(ASTNode* left, TokenType op, ASTNode* right);
ASTNode* ast_create_function_declaration(const char* name, ASTNode** params, 
                                       size_t param_count, ASTNode* return_type, 
                                       ASTNode* body);

// Node manipulation
void ast_node_destroy(ASTNode* node);
ASTNode* ast_node_clone(ASTNode* node);
void ast_node_set_type_info(ASTNode* node, TypeInfo* type_info);
```

### Memory Management

**AST Memory Pool**:
- AST nodes allocated from memory pool for efficient batch deallocation
- Automatic cleanup when parser context is destroyed
- Reference counting for shared nodes (rare cases)
- Clear ownership semantics: parser owns AST until transferred

```c
// Memory pool allocation
ASTNode* ast_node_create_pooled(MemoryPool* pool, ASTNodeType type, 
                               size_t line, size_t column);

// Batch cleanup
void ast_memory_pool_destroy(MemoryPool* pool);  // Frees all AST nodes
```

## Integration Patterns

### Lexer-Parser Integration

**Token Stream Management**:
```c
// Parser advances through token stream
void parser_advance(ParserContext* ctx) {
    token_destroy(&amp;ctx->current_token);
    ctx->current_token = lexer_next_token(ctx->lexer);
}

// Lookahead for parsing decisions
bool parser_check(ParserContext* ctx, TokenType type) {
    return ctx->current_token.type == type;
}

// Consume expected token
bool parser_consume(ParserContext* ctx, TokenType expected) {
    if (ctx->current_token.type == expected) {
        parser_advance(ctx);
        return true;
    } else {
        parser_error(ctx, "Unexpected token");
        return false;
    }
}
```

**Error Synchronization**:
- Parser synchronizes on statement boundaries after errors
- Lexer continues after lexical errors to find more tokens
- Both components maintain error state independently

### AST-Semantic Analysis Integration

**Type Information Attachment**:
```c
// Semantic analysis fills type information
void semantic_analyze_node(ASTNode* node, SymbolTable* symbols) {
    // Analyze node and determine type
    TypeInfo* type = infer_node_type(node, symbols);
    ast_node_set_type_info(node, type);
}
```

**Symbol Table Population**:
- Parser creates AST structure
- Semantic analysis walks AST to build symbol tables
- Type information flows back to AST nodes

## Error Handling

### Lexer Error Handling

**Lexical Error Types**:
```c
typedef enum {
    LEXER_ERROR_NONE,
    LEXER_ERROR_UNTERMINATED_STRING,
    LEXER_ERROR_INVALID_CHARACTER,
    LEXER_ERROR_INVALID_NUMBER,
    LEXER_ERROR_INVALID_ESCAPE_SEQUENCE,
    LEXER_ERROR_FILE_READ_ERROR
} LexerErrorType;
```

**Error Reporting**:
```c
void lexer_error(LexerContext* ctx, LexerErrorType type, const char* message) {
    ctx->has_error = true;
    ctx->error_type = type;
    ctx->error_message = strdup(message);
    ctx->error_line = ctx->line;
    ctx->error_column = ctx->column;
}
```

**Error Recovery**:
- Skip invalid characters and continue lexing
- Report multiple errors in single pass
- Provide helpful suggestions for common mistakes

### Parser Error Handling

**Parser Error Types**:
```c
typedef enum {
    PARSER_ERROR_NONE,
    PARSER_ERROR_UNEXPECTED_TOKEN,
    PARSER_ERROR_MISSING_TOKEN,
    PARSER_ERROR_INVALID_EXPRESSION,
    PARSER_ERROR_INVALID_STATEMENT,
    PARSER_ERROR_INVALID_DECLARATION,
    PARSER_ERROR_SYNTAX_ERROR
} ParserErrorType;
```

**Error Recovery Strategies**:

1\. **Panic Mode Recovery**:
```c
void parser_synchronize(ParserContext* ctx) {
    // Skip tokens until statement boundary
    while (!parser_at_end(ctx) &amp;&amp; 
           ctx->current_token.type != TOKEN_SEMICOLON &amp;&amp;
           ctx->current_token.type != TOKEN_LEFT_BRACE) {
        parser_advance(ctx);
    }
}
```

2\. **Phrase-Level Recovery**:
```c
ASTNode* parse_expression_with_recovery(ParserContext* ctx) {
    ASTNode* expr = parse_expression(ctx);
    if (!expr) {
        // Insert error node and continue
        expr = ast_create_error_node(ctx->current_token.line, 
                                   ctx->current_token.column);
        parser_advance(ctx);  // Skip problematic token
    }
    return expr;
}
```

**Error Message Quality**:
- Specific error messages with context
- Suggestions for common mistakes
- Multiple error reporting in single pass
- Source location information

Example error messages:
```
Error at line 15, column 8: Expected ';' after let statement
  let x = 42
         ^
  
Error at line 23, column 12: Invalid field pattern syntax
  match point { x, y } =&gt; ...
               ^
  Suggestion: Use explicit syntax like { x: x, y: y }
```

## Memory Management

### Memory Ownership Model

**Clear Ownership Semantics**:
1\. **Lexer**: Owns internal token storage, returns cloned tokens
2\. **Parser**: Owns AST nodes allocated from memory pool
3\. **Caller**: Owns tokens returned by lexer (must free)
4\. **Semantic Analysis**: Borrows AST, adds type information

### Memory Safety Patterns

**Token Memory Management**:
```c
// Safe token usage
Token token = lexer_next_token(ctx);
if (token.type != TOKEN_ERROR) {
    // Use token...
}
token_destroy(&amp;token);  // Always cleanup
```

**AST Memory Management**:
```c
// Parser creates AST with memory pool
ParserContext* parser = parser_create(lexer);
ASTNode* ast = parser_parse_program(parser);

// Transfer ownership to semantic analysis
SemanticContext* semantic = semantic_create(ast);

// Cleanup (AST destroyed with parser)
semantic_destroy(semantic);
parser_destroy(parser);
```

**Memory Pool Benefits**:
- Efficient allocation for many small AST nodes
- Automatic cleanup prevents memory leaks
- Better cache locality for AST traversal
- Simplified error handling (no partial cleanup needed)

## Performance Characteristics

### Lexer Performance

**Time Complexity**: O(n) where n is source code length
- Single pass through source code
- Constant time per character (amortized)
- O(1) keyword lookup using perfect hash

**Space Complexity**: O(1) working space + O(k) for tokens
- Constant working memory for lexer state
- Token storage proportional to token count
- No backtracking or lookahead buffering

**Optimizations**:
- Gperf-generated perfect hash for keywords
- Efficient string scanning with minimal copying
- Batch character processing for whitespace/comments

### Parser Performance

**Time Complexity**: O(n) where n is number of tokens
- Single pass through token stream
- No backtracking in recursive descent
- Linear time for each grammar rule

**Space Complexity**: O(d) where d is maximum nesting depth
- Call stack depth proportional to expression/statement nesting
- AST size proportional to program size
- Memory pool reduces allocation overhead

**Optimizations**:
- Operator precedence parsing avoids deep recursion
- Memory pool allocation for AST nodes
- Minimal token copying and string duplication

### Benchmark Results

**Lexer Performance** (typical source file):
- **Speed**: ~50MB/s source code processing
- **Memory**: &lt;1KB working memory + token storage
- **Keyword Lookup**: ~12.86 nanoseconds average

**Parser Performance** (typical source file):
- **Speed**: ~25MB/s token stream processing  
- **Memory**: &lt;10KB working memory + AST storage
- **AST Construction**: ~100 nanoseconds per node

## Development Guidelines

### Adding New Token Types

1\. **Add to TokenType enum** in `src/lexer/token.h`
2\. **Update token_type_to_string()** in `src/lexer/token.c`
3\. **Add scanning logic** in `src/lexer/lexer_scan.c`
4\. **Update parser** to handle new token type
5\. **Add tests** for lexer and parser

Example:
```