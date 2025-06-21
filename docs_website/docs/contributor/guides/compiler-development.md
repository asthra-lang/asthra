# Compiler Development Guide

**Complete guide to understanding and developing the Asthra compiler**

This guide provides a comprehensive overview of the Asthra compiler architecture, compilation pipeline, and practical guidance for working with each component.

## Overview

- **Purpose**: Understand and contribute to the Asthra compiler implementation
- **Audience**: Intermediate to advanced contributors
- **Prerequisites**: Understanding of compilers, C programming, and Asthra language basics
- **Outcome**: Ability to work effectively with all compiler components

## Compilation Pipeline

The Asthra compiler implements a traditional multi-phase compilation approach optimized for AI-assisted development and memory safety:

```
Source Code (.asthra)
        ↓
    &#91;Lexer&#93; → Token Stream
        ↓
    &#91;Parser&#93; → Abstract Syntax Tree (AST)
        ↓
&#91;Semantic Analysis&#93; → Validated AST + Symbol Tables
        ↓
&#91;Code Generation&#93; → C Code + FFI Assembly
        ↓
    &#91;C Compiler&#93; → Object Files
        ↓
    &#91;Linker&#93; → Executable Binary
```

### Key Design Principles

1\. **AI Generation Efficiency**: Minimal syntax for maximum AI model predictability
2\. **Memory Safety**: Four-zone memory model with ownership tracking
3\. **Modular Architecture**: Component isolation for maintainability
4\. **FFI Safety**: Automatic C binding generation with transfer semantics
5\. **Deterministic Parsing**: No ambiguous constructs for reliable compilation

## Phase 1: Lexical Analysis

### Architecture Overview

**Location**: `src/parser/lexer_*.c`  
**Purpose**: Convert source text into a stream of tokens  
**Key Files**:
- `lexer_core.c` - Main lexer implementation
- `lexer_numbers.c` - Numeric literal parsing
- `lexer_strings.c` - String literal parsing
- `lexer_keywords.c` - Keyword recognition

### How the Lexer Works

```c
// Core lexer structure
typedef struct {
    const char *source;      // Source code text
    size_t position;         // Current position in source
    size_t line;            // Current line number
    size_t column;          // Current column number
    Token *tokens;          // Token array
    size_t token_count;     // Number of tokens
    size_t token_capacity;  // Token array capacity
} Lexer;

// Main lexing function
LexerResult lexer_tokenize(const char *source) {
    Lexer lexer;
    lexer_init(&amp;lexer, source);
    
    while (!lexer_at_end(&amp;lexer)) {
        Token token = lexer_next_token(&amp;lexer);
        lexer_add_token(&amp;lexer, token);
    }
    
    return lexer_finalize(&amp;lexer);
}
```

### Token Types and Structure

```c
typedef enum {
    // Literals
    TOKEN_NUMBER,           // 42, 3.14, 0xFF, 0b1010
    TOKEN_STRING,           // "hello world"
    TOKEN_IDENTIFIER,       // variable_name
    
    // Keywords
    TOKEN_FN,              // fn
    TOKEN_STRUCT,          // struct
    TOKEN_IF,              // if
    TOKEN_WHILE,           // while
    
    // Operators
    TOKEN_PLUS,            // +
    TOKEN_MINUS,           // -
    TOKEN_MULTIPLY,        // *
    TOKEN_DIVIDE,          // /
    
    // Punctuation
    TOKEN_SEMICOLON,       // ;
    TOKEN_COMMA,           // ,
    TOKEN_LEFT_PAREN,      // (
    TOKEN_RIGHT_PAREN,     // )
    
    // Special
    TOKEN_EOF,             // End of file
    TOKEN_ERROR            // Lexing error
} TokenType;

typedef struct {
    TokenType type;
    char *value;           // Token text (owned)
    SourceLocation location; // File position
} Token;
```

### Working with the Lexer

#### Adding New Token Types

1\. **Add token type** to `TokenType` enum:
```c
// In lexer_core.h
typedef enum {
    // ... existing tokens ...
    TOKEN_NEW_OPERATOR,    // Your new token
} TokenType;
```

2\. **Add recognition logic** in appropriate lexer file:
```c
// In lexer_core.c or specialized file
static Token lexer_scan_operator(Lexer *lexer) {
    char current = lexer_current_char(lexer);
    
    switch (current) {
        case '+': return lexer_make_token(lexer, TOKEN_PLUS);
        case '-': return lexer_make_token(lexer, TOKEN_MINUS);
        case '&amp;': // Your new operator
            if (lexer_peek_char(lexer) == '&amp;') {
                lexer_advance(lexer); // Consume second '&amp;'
                return lexer_make_token(lexer, TOKEN_NEW_OPERATOR);
            }
            break;
    }
    
    return lexer_error_token(lexer, "Unexpected character");
}
```

3\. **Add string representation**:
```c
// In lexer_core.c
const char *token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_NEW_OPERATOR: return "&amp;&amp;";
        // ... other cases ...
        default: return "UNKNOWN";
    }
}
```

#### Testing Lexer Changes

```c
// Example test in tests/lexer/test_lexer_operators.c
void test_new_operator_lexing(void) {
    const char *source = "a &amp;&amp; b";
    LexerResult result = lexer_tokenize(source);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQUALS_INT(result.token_count, 4); // a, &amp;&amp;, b, EOF
    
    ASSERT_EQUALS_INT(result.tokens&#91;0&#93;.type, TOKEN_IDENTIFIER);
    ASSERT_EQUALS_STRING(result.tokens&#91;0&#93;.value, "a");
    
    ASSERT_EQUALS_INT(result.tokens&#91;1&#93;.type, TOKEN_NEW_OPERATOR);
    ASSERT_EQUALS_STRING(result.tokens&#91;1&#93;.value, "&amp;&amp;");
    
    ASSERT_EQUALS_INT(result.tokens&#91;2&#93;.type, TOKEN_IDENTIFIER);
    ASSERT_EQUALS_STRING(result.tokens&#91;2&#93;.value, "b");
    
    lexer_result_free(&amp;result);
}
```

## Phase 2: Parsing

### Architecture Overview

**Location**: `src/parser/grammar_*.c`  
**Purpose**: Build Abstract Syntax Tree from token stream  
**Key Files**:
- `grammar_expressions.c` - Expression parsing
- `grammar_statements.c` - Statement parsing
- `grammar_declarations.c` - Declaration parsing
- `grammar_types.c` - Type parsing

### Parser Structure

```c
typedef struct {
    Token *tokens;          // Token array from lexer
    size_t token_count;     // Number of tokens
    size_t current;         // Current token index
    ASTNode *root;          // Root of AST
    SymbolTable *symbols;   // Symbol table
    ErrorHandler *errors;   // Error reporting
} Parser;

// Main parsing function
ParseResult parser_parse(Token *tokens, size_t token_count) {
    Parser parser;
    parser_init(&amp;parser, tokens, token_count);
    
    ASTNode *root = parse_program(&amp;parser);
    
    return parser_finalize(&amp;parser, root);
}
```

### AST Node Structure

```c
typedef enum {
    AST_PROGRAM,           // Root program node
    AST_FUNCTION,          // Function declaration
    AST_STRUCT,            // Struct declaration
    AST_VARIABLE,          // Variable declaration
    AST_BINARY_OP,         // Binary operation (a + b)
    AST_UNARY_OP,          // Unary operation (-a)
    AST_FUNCTION_CALL,     // Function call
    AST_LITERAL,           // Literal value
    AST_IDENTIFIER,        // Variable reference
    AST_BLOCK,             // Code block { ... }
    AST_IF,                // If statement
    AST_WHILE,             // While loop
    AST_RETURN             // Return statement
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    SourceLocation location;
    
    union {
        struct {
            char *name;
            struct ASTNode **parameters;
            size_t parameter_count;
            struct ASTNode *body;
            struct ASTNode *return_type;
        } function;
        
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
            TokenType operator;
        } binary_op;
        
        struct {
            char *value;
            TokenType literal_type;
        } literal;
        
        // ... other node types
    } data;
    
    // Metadata
    struct ASTNode *parent;
    struct ASTNode **children;
    size_t child_count;
} ASTNode;
```

### Working with the Parser

#### Adding New Grammar Rules

1\. **Define the grammar rule** (using PEG notation):
```
// Example: Adding a new statement type
NewStatement &lt;- 'new' Identifier '=' Expression ';'
```

2\. **Implement the parsing function**:
```c
// In appropriate grammar_*.c file
ASTNode *parse_new_statement(Parser *parser) {
    // Expect 'new' keyword
    if (!parser_expect_token(parser, TOKEN_NEW)) {
        return NULL;
    }
    
    // Parse identifier
    ASTNode *identifier = parse_identifier(parser);
    if (!identifier) {
        parser_error(parser, "Expected identifier after 'new'");
        return NULL;
    }
    
    // Expect '=' token
    if (!parser_expect_token(parser, TOKEN_EQUALS)) {
        ast_node_destroy(identifier);
        return NULL;
    }
    
    // Parse expression
    ASTNode *expression = parse_expression(parser);
    if (!expression) {
        ast_node_destroy(identifier);
        parser_error(parser, "Expected expression after '='");
        return NULL;
    }
    
    // Expect ';' token
    if (!parser_expect_token(parser, TOKEN_SEMICOLON)) {
        ast_node_destroy(identifier);
        ast_node_destroy(expression);
        return NULL;
    }
    
    // Create AST node
    ASTNode *node = ast_node_create(AST_NEW_STATEMENT);
    node-&gt;data.new_statement.identifier = identifier;
    node->data.new_statement.expression = expression;
    
    return node;
}
```

3\. **Integrate into statement parsing**:
```c
// In grammar_statements.c
ASTNode *parse_statement(Parser *parser) {
    Token current = parser_current_token(parser);
    
    switch (current.type) {
        case TOKEN_IF:
            return parse_if_statement(parser);
        case TOKEN_WHILE:
            return parse_while_statement(parser);
        case TOKEN_NEW:  // Your new statement
            return parse_new_statement(parser);
        case TOKEN_IDENTIFIER:
            return parse_assignment_or_call(parser);
        default:
            parser_error(parser, "Unexpected token in statement");
            return NULL;
    }
}
```

#### Error Recovery Strategies

```c
// Synchronization points for error recovery
static void parser_synchronize(Parser *parser) {
    parser_advance(parser); // Skip the problematic token
    
    while (!parser_at_end(parser)) {
        Token previous = parser_previous_token(parser);
        if (previous.type == TOKEN_SEMICOLON) {
            return; // Found statement boundary
        }
        
        Token current = parser_current_token(parser);
        switch (current.type) {
            case TOKEN_FN:
            case TOKEN_STRUCT:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_RETURN:
                return; // Found statement start
            default:
                parser_advance(parser);
        }
    }
}

// Use in parsing functions
ASTNode *parse_statement_with_recovery(Parser *parser) {
    ASTNode *stmt = parse_statement(parser);
    if (!stmt) {
        parser_synchronize(parser);
        // Continue parsing after error
    }
    return stmt;
}
```

## Phase 3: Semantic Analysis

### Architecture Overview

**Location**: `src/analysis/semantic_*.c`  
**Purpose**: Type checking, symbol resolution, and validation  
**Key Files**:
- `semantic_analysis.c` - Main analysis coordinator
- `type_checker.c` - Type inference and checking
- `symbol_table.c` - Symbol table management
- `scope_manager.c` - Scope tracking

### Symbol Table Structure

```c
typedef struct Symbol {
    char *name;              // Symbol name
    SymbolType type;         // Variable, function, struct, etc.
    ASTNode *declaration;    // Declaration AST node
    TypeInfo *type_info;     // Type information
    ScopeLevel scope;        // Scope level
    bool is_mutable;         // Mutability flag
    MemoryZone memory_zone;  // Memory management zone
} Symbol;

typedef struct SymbolTable {
    Symbol **symbols;        // Symbol array
    size_t symbol_count;     // Number of symbols
    size_t capacity;         // Array capacity
    struct SymbolTable *parent; // Parent scope
    ScopeLevel level;        // Current scope level
} SymbolTable;
```

### Type System

```c
typedef enum {
    TYPE_PRIMITIVE,          // i32, f64, bool, char
    TYPE_POINTER,            // *T
    TYPE_ARRAY,              // &#91;T; N&#93;
    TYPE_SLICE,              // &#91;T&#93;
    TYPE_STRUCT,             // struct { ... }
    TYPE_FUNCTION,           // fn(T, U) -&gt; V
    TYPE_GENERIC,            // T (generic parameter)
    TYPE_INFERRED            // Type to be inferred
} TypeKind;

typedef struct TypeInfo {
    TypeKind kind;
    union {
        struct {
            PrimitiveType primitive;
        } primitive;
        
        struct {
            struct TypeInfo *element_type;
            bool is_mutable;
        } pointer;
        
        struct {
            struct TypeInfo *element_type;
            size_t size;
        } array;
        
        struct {
            char *name;
            struct TypeInfo **field_types;
            char **field_names;
            size_t field_count;
        } struct_type;
        
        struct {
            struct TypeInfo **parameter_types;
            size_t parameter_count;
            struct TypeInfo *return_type;
        } function;
    } data;
} TypeInfo;
```

### Working with Semantic Analysis

#### Adding Type Checking Rules

```c
// Example: Type checking for new statement
TypeCheckResult type_check_new_statement(TypeChecker *checker, ASTNode *node) {
    // Check identifier type
    Symbol *symbol = symbol_table_lookup(checker->symbols, 
                                        node->data.new_statement.identifier->data.identifier.name);
    if (!symbol) {
        return type_error(checker, node, "Undefined variable");
    }
    
    // Check expression type
    TypeInfo *expr_type = type_check_expression(checker, node->data.new_statement.expression);
    if (!expr_type) {
        return type_error(checker, node, "Invalid expression");
    }
    
    // Check type compatibility
    if (!types_are_compatible(symbol->type_info, expr_type)) {
        return type_error(checker, node, "Type mismatch in assignment");
    }
    
    // Check mutability
    if (!symbol->is_mutable) {
        return type_error(checker, node, "Cannot assign to immutable variable");
    }
    
    return type_check_success(expr_type);
}
```

#### Symbol Resolution

```c
// Symbol resolution with scope handling
Symbol *resolve_symbol(SymbolTable *table, const char *name) {
    // Search current scope first
    for (size_t i = 0; i &lt; table->symbol_count; i++) {
        if (strcmp(table->symbols&#91;i&#93;->name, name) == 0) {
            return table->symbols&#91;i&#93;;
        }
    }
    
    // Search parent scopes
    if (table->parent) {
        return resolve_symbol(table->parent, name);
    }
    
    return NULL; // Symbol not found
}

// Add symbol with scope checking
bool add_symbol(SymbolTable *table, Symbol *symbol) {
    // Check for redefinition in current scope
    for (size_t i = 0; i &lt; table->symbol_count; i++) {
        if (strcmp(table->symbols&#91;i&#93;->name, symbol->name) == 0) {
            return false; // Redefinition error
        }
    }
    
    // Add to current scope
    symbol_table_add(table, symbol);
    return true;
}
```

## Phase 4: Code Generation

### Architecture Overview

**Location**: `src/codegen/`  
**Purpose**: Generate C code and FFI assembly  
**Key Files**:
- `codegen_core.c` - Main code generation
- `codegen_expressions.c` - Expression code generation
- `codegen_statements.c` - Statement code generation
- `codegen_ffi.c` - FFI binding generation

### Code Generation Structure

```c
typedef struct CodeGenerator {
    FILE *output_file;       // Output C file
    SymbolTable *symbols;    // Symbol table
    TypeTable *types;        // Type information
    int indent_level;        // Current indentation
    bool in_function;        // Function context flag
    char *current_function;  // Current function name
} CodeGenerator;

// Main code generation function
CodeGenResult codegen_generate(ASTNode *ast, const char *output_path) {
    CodeGenerator generator;
    codegen_init(&amp;generator, output_path);
    
    codegen_emit_headers(&amp;generator);
    codegen_visit_node(&amp;generator, ast);
    codegen_emit_main(&amp;generator);
    
    return codegen_finalize(&amp;generator);
}
```

### Working with Code Generation

#### Adding Code Generation for New Constructs

```c
// Example: Code generation for new statement
void codegen_new_statement(CodeGenerator *gen, ASTNode *node) {
    // Generate indentation
    codegen_emit_indent(gen);
    
    // Generate assignment
    codegen_emit(gen, "%s = ", 
                node->data.new_statement.identifier->data.identifier.name);
    
    // Generate expression
    codegen_visit_node(gen, node->data.new_statement.expression);
    
    // End statement
    codegen_emit(gen, ";\n");
}

// Add to main visitor
void codegen_visit_node(CodeGenerator *gen, ASTNode *node) {
    switch (node->type) {
        case AST_FUNCTION:
            codegen_function(gen, node);
            break;
        case AST_BINARY_OP:
            codegen_binary_op(gen, node);
            break;
        case AST_NEW_STATEMENT:  // Your new statement
            codegen_new_statement(gen, node);
            break;
        // ... other cases
    }
}
```

#### Memory Management in Generated Code

```c
// Generate memory management calls
void codegen_memory_management(CodeGenerator *gen, ASTNode *node) {
    TypeInfo *type = get_node_type(node);
    
    switch (type->memory_zone) {
        case MEMORY_ZONE_GC:
            codegen_emit(gen, "gc_alloc(sizeof(%s))", type_to_c_string(type));
            break;
        case MEMORY_ZONE_MANUAL:
            codegen_emit(gen, "malloc(sizeof(%s))", type_to_c_string(type));
            break;
        case MEMORY_ZONE_PINNED:
            codegen_emit(gen, "pinned_alloc(sizeof(%s))", type_to_c_string(type));
            break;
        case MEMORY_ZONE_EXTERNAL:
            codegen_emit(gen, "external_alloc(sizeof(%s))", type_to_c_string(type));
            break;
    }
}
```

## Development Workflows

### Adding a New Language Feature

1\. **Design the feature** - Define syntax and semantics
2\. **Update lexer** - Add new tokens if needed
3\. **Update parser** - Add grammar rules and AST nodes
4\. **Update semantic analysis** - Add type checking and validation
5\. **Update code generation** - Generate appropriate C code
6\. **Add tests** - Comprehensive test coverage
7\. **Update documentation** - Language reference and examples

### Debugging Compiler Issues

#### Lexer Debugging
```bash
# Test lexer in isolation
make test-lexer-specific
./build/tests/lexer/test_lexer_debug input.asthra

# Enable lexer debugging
export ASTHRA_DEBUG_LEXER=1
./bin/asthra input.asthra
```

#### Parser Debugging
```bash
# Dump AST for inspection
export ASTHRA_DUMP_AST=1
./bin/asthra input.asthra

# Enable parser debugging
export ASTHRA_DEBUG_PARSER=1
./bin/asthra input.asthra
```

#### Semantic Analysis Debugging
```bash
# Dump symbol tables
export ASTHRA_DUMP_SYMBOLS=1
./bin/asthra input.asthra

# Enable type checking debug
export ASTHRA_DEBUG_TYPES=1
./bin/asthra input.asthra
```

#### Code Generation Debugging
```bash
# Keep generated C code
export ASTHRA_KEEP_C_CODE=1
./bin/asthra input.asthra
cat input.c  # Inspect generated code

# Enable codegen debugging
export ASTHRA_DEBUG_CODEGEN=1
./bin/asthra input.asthra
```

### Performance Optimization

#### Profiling the Compiler
```bash
# Profile compilation performance
make profile
./bin/asthra large_file.asthra
gprof ./bin/asthra gmon.out &gt; profile.txt

# Memory usage profiling
valgrind --tool=massif ./bin/asthra large_file.asthra
ms_print massif.out.* &gt; memory_profile.txt
```

#### Optimization Strategies

1\. **Lexer Optimizations**:
   - Use lookup tables for keywords
   - Minimize string allocations
   - Optimize character scanning loops

2\. **Parser Optimizations**:
   - Cache frequently accessed tokens
   - Minimize AST node allocations
   - Use object pools for common nodes

3\. **Semantic Analysis Optimizations**:
   - Hash tables for symbol lookup
   - Type caching and interning
   - Incremental analysis for large files

4\. **Code Generation Optimizations**:
   - Template-based generation
   - Minimize file I/O operations
   - Batch similar operations

## Testing Strategies

### Unit Testing
```c
// Test individual compiler components
void test_lexer_number_parsing(void) {
    const char *source = "42 3.14 0xFF 0b1010";
    LexerResult result = lexer_tokenize(source);
    
    ASSERT_TRUE(result.success);
    ASSERT_EQUALS_INT(result.token_count, 5); // 4 numbers + EOF
    
    // Test each number token
    ASSERT_EQUALS_INT(result.tokens&#91;0&#93;.type, TOKEN_NUMBER);
    ASSERT_EQUALS_STRING(result.tokens&#91;0&#93;.value, "42");
    
    lexer_result_free(&amp;result);
}
```

### Integration Testing
```c
// Test complete compilation pipeline
void test_complete_compilation(void) {
    const char *source = "fn main() -&gt; i32 { return 42; }";
    
    // Compile to executable
    CompilationResult result = compile_source(source, "test_output");
    ASSERT_TRUE(result.success);
    
    // Run executable and check output
    int exit_code = run_executable("test_output");
    ASSERT_EQUALS_INT(exit_code, 42);
    
    cleanup_compilation_result(&amp;result);
}
```

### Regression Testing
```bash
# Run comprehensive test suite
make test-all

# Test specific components
make test-lexer
make test-parser
make test-semantic
make test-codegen

# Performance regression tests
make test-performance
```

## Common Patterns

### Error Handling
```c
// Consistent error handling pattern
typedef struct {
    bool success;
    union {
        void *result;        // On success
        char *error_message; // On failure
    };
} CompilerResult;

#define COMPILER_SUCCESS(value) ((CompilerResult){.success = true, .result = (value)})
#define COMPILER_ERROR(msg) ((CompilerResult){.success = false, .error_message = (msg)})
```

### Memory Management
```c
// RAII-style cleanup
#define CLEANUP_LEXER __attribute__((cleanup(lexer_cleanup_auto)))
void lexer_cleanup_auto(Lexer **lexer);

void example_function(void) {
    CLEANUP_LEXER Lexer lexer;
    lexer_init(&amp;lexer, source);
    // Automatic cleanup on scope exit
}
```

### Visitor Pattern
```c
// AST visitor pattern for extensibility
typedef struct ASTVisitor {
    void (*visit_function)(struct ASTVisitor *visitor, ASTNode *node);
    void (*visit_binary_op)(struct ASTVisitor *visitor, ASTNode *node);
    void (*visit_literal)(struct ASTVisitor *visitor, ASTNode *node);
    // ... other visit methods
    void *context; // Visitor-specific data
} ASTVisitor;

void ast_accept(ASTNode *node, ASTVisitor *visitor) {
    switch (node->type) {
        case AST_FUNCTION:
            visitor->visit_function(visitor, node);
            break;
        case AST_BINARY_OP:
            visitor->visit_binary_op(visitor, node);
            break;
        // ... other cases
    }
}
```

## Success Criteria

You're ready to contribute to compiler development when:

- &#91; &#93; **Architecture Understanding**: Can explain the compilation pipeline
- &#91; &#93; **Component Knowledge**: Understand lexer, parser, semantic analysis, and codegen
- &#91; &#93; **Development Skills**: Can add new language features end-to-end
- &#91; &#93; **Debugging Proficiency**: Can diagnose and fix compiler issues
- &#91; &#93; **Testing Competency**: Write comprehensive tests for compiler components
- &#91; &#93; **Performance Awareness**: Understand optimization strategies and trade-offs

## Related Guides

- **&#91;Adding Features Workflow&#93;(../workflows/adding-features.md)** - Step-by-step feature development
- **&#91;Testing Workflows&#93;(../workflows/testing-workflows.md)** - Comprehensive testing strategies
- **&#91;Performance Optimization&#93;(../workflows/performance-optimization.md)** - Optimization techniques
- **&#91;Code Style Guide&#93;(code-style.md)** - Coding standards and conventions

## Reference Materials

- **&#91;Architecture Documentation&#93;(../reference/architecture/)** - Detailed architecture docs
- **&#91;API Reference&#93;(../reference/api/)** - Internal API documentation
- **&#91;Language Specification&#93;(../reference/language-spec.md)** - Asthra language definition
- **&#91;Build System Guide&#93;(../reference/build-system.md)** - Build system documentation

---

**🔧 Compiler Development Mastered!**

*Understanding the compiler architecture and development process enables you to contribute effectively to the core language implementation. The modular design makes it possible to work on individual components while understanding their interactions.* 