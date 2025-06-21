# Code Style Guide

**Comprehensive coding standards and style guidelines for Asthra development**

This guide establishes consistent coding standards across the Asthra codebase, ensuring readability, maintainability, and quality.

## Overview

- **Purpose**: Maintain consistent, high-quality code across the Asthra project
- **Standard**: C17 with modern best practices
- **Tools**: Clang-format, Clang-tidy, Cppcheck
- **Enforcement**: Automated formatting and static analysis

## C17 Standards

### File Organization

#### File Headers
Every source file should include a comprehensive header:

```c
/**
 * @file lexer_core.c
 * @brief Core lexical analysis implementation for Asthra compiler
 * 
 * This module implements the main lexer functionality including:
 * - Token stream generation
 * - Source position tracking
 * - Error recovery and reporting
 * - Memory management for tokens
 * 
 * The lexer uses a streaming approach to minimize memory usage
 * and supports all Asthra language constructs including:
 * - Keywords and identifiers
 * - Numeric literals (decimal, hex, binary, octal)
 * - String literals with escape sequences
 * - Operators and punctuation
 * - Comments and whitespace handling
 * 
 * @author Asthra Language Team
 * @date January 2025
 * @version 1.0
 */
```

#### Include Order
```c
// 1. System includes first
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 2. Third-party includes
#include <json-c/json.h>

// 3. Project includes (relative to src/)
#include "lexer/lexer_core.h"
#include "parser/parser_core.h"
#include "utils/memory_utils.h"

// 4. Local includes (same directory)
#include "lexer_internal.h"
```

#### Header Guards
Use include guards for all header files:

```c
#ifndef ASTHRA_LEXER_CORE_H
#define ASTHRA_LEXER_CORE_H

// Header content here

#endif // ASTHRA_LEXER_CORE_H
```

### Naming Conventions

#### Functions
Use snake_case for all function names:

```c
// ✅ Good
void parse_expression(Parser *parser);
ASTNode *create_binary_node(ASTNode *left, ASTNode *right);
bool validate_symbol_table(SymbolTable *table);

// ❌ Bad
void parseExpression(Parser *parser);
ASTNode *CreateBinaryNode(ASTNode *left, ASTNode *right);
bool ValidateSymbolTable(SymbolTable *table);
```

#### Variables
Use snake_case for variables:

```c
// ✅ Good
int token_count = 0;
char *source_file_path = NULL;
bool is_valid_identifier = true;

// ❌ Bad
int tokenCount = 0;
char *sourceFilePath = NULL;
bool isValidIdentifier = true;
```

#### Types and Structures
Use PascalCase for type names:

```c
// ✅ Good
typedef struct {
    TokenType type;
    char *value;
    SourceLocation location;
} Token;

typedef enum {
    AST_EXPRESSION,
    AST_STATEMENT,
    AST_DECLARATION
} ASTNodeType;

// ❌ Bad
typedef struct {
    TokenType type;
    char *value;
    SourceLocation location;
} token;

typedef enum {
    ast_expression,
    ast_statement,
    ast_declaration
} ast_node_type;
```

#### Constants and Macros
Use UPPER_SNAKE_CASE for constants and macros:

```c
// ✅ Good
#define MAX_TOKEN_LENGTH 256
#define DEFAULT_BUFFER_SIZE 1024
static const int MAX_RECURSION_DEPTH = 100;

// ❌ Bad
#define maxTokenLength 256
#define default_buffer_size 1024
static const int max_recursion_depth = 100;
```

### Code Formatting

#### Indentation
- Use 4 spaces for indentation (no tabs)
- Align continuation lines with opening parenthesis or use 8 spaces

```c
// ✅ Good
if (condition_one && condition_two &&
    condition_three && condition_four) {
    do_something();
}

// Alternative alignment
if (condition_one && condition_two &&
        condition_three && condition_four) {
    do_something();
}

// ❌ Bad
if (condition_one && condition_two &&
condition_three && condition_four) {
    do_something();
}
```

#### Braces
Use K&R style braces:

```c
// ✅ Good
if (condition) {
    statement1();
    statement2();
} else {
    alternative();
}

void function_name(int param) {
    // function body
}

// ❌ Bad
if (condition)
{
    statement1();
    statement2();
}
else
{
    alternative();
}
```

#### Line Length
- Prefer lines under 80 characters
- Hard limit at 120 characters
- Break long lines at logical points

```c
// ✅ Good
result = very_long_function_name(parameter_one, parameter_two,
                                parameter_three, parameter_four);

// ✅ Also good
result = very_long_function_name(
    parameter_one,
    parameter_two, 
    parameter_three,
    parameter_four
);

// ❌ Bad
result = very_long_function_name(parameter_one, parameter_two, parameter_three, parameter_four, parameter_five);
```

#### Spacing
```c
// ✅ Good spacing
int result = calculate_value(a, b, c);
if (x > 0 && y < 10) {
    array[index] = value;
}

for (int i = 0; i < count; i++) {
    process_item(items[i]);
}

// ❌ Bad spacing
int result=calculate_value(a,b,c);
if(x>0&&y<10){
    array[index]=value;
}

for(int i=0;i<count;i++){
    process_item(items[i]);
}
```

### Documentation Standards

#### Function Documentation
Use Doxygen-style comments for all public functions:

```c
/**
 * @brief Parse a struct declaration from the token stream
 * 
 * Parses a complete struct declaration according to the grammar:
 * StructDecl <- 'struct' SimpleIdent TypeParams? '{' StructFields? '}'
 * 
 * @param parser The parser instance (must not be NULL)
 * @param visibility The visibility modifier (pub or priv)
 * @param annotations Optional annotations array (can be NULL)
 * @param annotation_count Number of annotations (0 if annotations is NULL)
 * 
 * @return AST node for struct declaration on success, NULL on error
 * 
 * @note The returned AST node must be freed with ast_node_destroy()
 * @note Parser errors are reported through parser->error_handler
 * 
 * @example
 * ```c
 * ASTNode* struct_node = parse_struct_declaration(parser, VISIBILITY_PUB, NULL, 0);
 * if (struct_node) {
 *     // Use struct_node
 *     ast_node_destroy(struct_node);
 * }
 * ```
 */
ASTNode *parse_struct_declaration(Parser *parser, 
                                 VisibilityModifier visibility,
                                 Annotation *annotations,
                                 size_t annotation_count);
```

#### Inline Comments
Use clear, concise comments for complex logic:

```c
// ✅ Good comments
// Check if we've reached the end of the token stream
if (parser->current_token >= parser->token_count) {
    return NULL;
}

// Handle operator precedence for binary expressions
// Higher precedence operators bind more tightly
int precedence = get_operator_precedence(operator);

// ❌ Bad comments
// Increment i
i++;

// Check condition
if (x > 0) {
    // Do something
    process();
}
```

### Error Handling

#### Return Value Conventions
```c
// ✅ Good: Consistent error handling
typedef enum {
    PARSE_SUCCESS = 0,
    PARSE_ERROR_SYNTAX = -1,
    PARSE_ERROR_MEMORY = -2,
    PARSE_ERROR_INVALID_INPUT = -3
} ParseResult;

ParseResult parse_expression(Parser *parser, ASTNode **result) {
    if (!parser || !result) {
        return PARSE_ERROR_INVALID_INPUT;
    }
    
    *result = NULL;  // Initialize output parameter
    
    // ... parsing logic ...
    
    if (memory_allocation_failed) {
        return PARSE_ERROR_MEMORY;
    }
    
    *result = node;
    return PARSE_SUCCESS;
}
```

#### Error Messages
```c
// ✅ Good: Descriptive error messages
void parser_error(Parser *parser, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    fprintf(stderr, "Parse error at line %d, column %d: ",
            parser->current_line, parser->current_column);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}

// Usage
parser_error(parser, "Expected ';' after expression, found '%s'", 
             token_to_string(parser->current_token));
```

### Memory Management

#### Allocation Patterns
```c
// ✅ Good: RAII-style resource management
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} Buffer;

Buffer *buffer_create(size_t initial_capacity) {
    Buffer *buffer = malloc(sizeof(Buffer));
    if (!buffer) {
        return NULL;
    }
    
    buffer->data = malloc(initial_capacity);
    if (!buffer->data) {
        free(buffer);
        return NULL;
    }
    
    buffer->size = 0;
    buffer->capacity = initial_capacity;
    return buffer;
}

void buffer_destroy(Buffer *buffer) {
    if (!buffer) {
        return;
    }
    
    free(buffer->data);
    free(buffer);
}
```

#### Ownership Conventions
```c
// ✅ Good: Clear ownership semantics
/**
 * @brief Create a new AST node (caller owns the result)
 * @return Newly allocated AST node, must be freed with ast_node_destroy()
 */
ASTNode *ast_node_create(ASTNodeType type);

/**
 * @brief Add child to parent node (parent takes ownership of child)
 * @param parent Parent node (must not be NULL)
 * @param child Child node (ownership transferred to parent)
 */
void ast_node_add_child(ASTNode *parent, ASTNode *child);

/**
 * @brief Destroy AST node and all children (recursive cleanup)
 * @param node Node to destroy (can be NULL)
 */
void ast_node_destroy(ASTNode *node);
```

### Performance Guidelines

#### Efficient Algorithms
```c
// ✅ Good: Use appropriate data structures
typedef struct {
    char **keys;
    void **values;
    size_t *hashes;
    size_t capacity;
    size_t count;
} HashTable;

// O(1) average case lookup
void *hash_table_get(HashTable *table, const char *key);

// ❌ Bad: Linear search for large datasets
void *linear_search(Array *array, const char *key) {
    for (size_t i = 0; i < array->count; i++) {
        if (strcmp(array->items[i].key, key) == 0) {
            return array->items[i].value;
        }
    }
    return NULL;
}
```

#### Memory Efficiency
```c
// ✅ Good: Minimize allocations
void process_tokens(Token *tokens, size_t count) {
    // Use stack allocation for small, temporary data
    char buffer[256];
    
    // Reuse allocations when possible
    StringBuilder *sb = string_builder_create(1024);
    
    for (size_t i = 0; i < count; i++) {
        string_builder_clear(sb);  // Reuse instead of reallocating
        format_token(tokens[i], sb);
        process_formatted_token(string_builder_get(sb));
    }
    
    string_builder_destroy(sb);
}
```

## Automated Tools

### Clang-Format Configuration

Create `.clang-format` in project root:

```yaml
---
Language: C
BasedOnStyle: LLVM
IndentWidth: 4
TabWidth: 4
UseTab: Never
ColumnLimit: 80
BreakBeforeBraces: Linux
AlignConsecutiveAssignments: false
AlignConsecutiveDeclarations: false
AlignOperands: true
AllowShortFunctionsOnASingleLine: None
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
BinPackArguments: true
BinPackParameters: true
BreakBeforeBinaryOperators: None
BreakBeforeTernaryOperators: true
BreakStringLiterals: true
IndentCaseLabels: true
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 2
PointerAlignment: Right
SpaceAfterCStyleCast: true
SpaceBeforeAssignmentOperators: true
SpaceBeforeParens: ControlStatements
SpaceInEmptyParentheses: false
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false
```

### Clang-Tidy Configuration

Create `.clang-tidy` in project root:

```yaml
Checks: >
  clang-diagnostic-*,
  clang-analyzer-*,
  cppcoreguidelines-*,
  modernize-*,
  performance-*,
  portability-*,
  readability-*,
  -readability-magic-numbers,
  -cppcoreguidelines-avoid-magic-numbers

CheckOptions:
  - key: readability-identifier-naming.VariableCase
    value: snake_case
  - key: readability-identifier-naming.FunctionCase
    value: snake_case
  - key: readability-identifier-naming.TypeCase
    value: CamelCase
  - key: readability-identifier-naming.MacroCase
    value: UPPER_CASE
  - key: readability-identifier-naming.ConstantCase
    value: UPPER_CASE
```

### Usage Commands

```bash
# Format all source files
find src tests -name "*.c" -o -name "*.h" | xargs clang-format -i

# Run static analysis
clang-tidy src/**/*.c -- -Isrc -Iruntime/include

# Check formatting without modifying files
clang-format --dry-run --Werror src/**/*.c

# Automated formatting in build system
make format          # Format all files
make format-check    # Check formatting without changes
make static-analysis # Run clang-tidy and cppcheck
```

## Quality Assurance

### Pre-commit Checks

Create `.pre-commit-config.yaml`:

```yaml
repos:
  - repo: local
    hooks:
      - id: clang-format
        name: clang-format
        entry: clang-format
        language: system
        files: \.(c|h)$
        args: [--dry-run, --Werror]
      
      - id: clang-tidy
        name: clang-tidy
        entry: clang-tidy
        language: system
        files: \.c$
        args: [--warnings-as-errors=*]
```

### Build Integration

```makefile
# Add to Makefile
.PHONY: format format-check static-analysis

format:
	@echo "Formatting source code..."
	@find src tests -name "*.c" -o -name "*.h" | xargs clang-format -i

format-check:
	@echo "Checking code formatting..."
	@find src tests -name "*.c" -o -name "*.h" | xargs clang-format --dry-run --Werror

static-analysis:
	@echo "Running static analysis..."
	@clang-tidy src/**/*.c -- -Isrc -Iruntime/include
	@cppcheck --enable=all --std=c17 src/

# Integrate with main build
all: format-check static-analysis compile

# CI/CD integration
ci-check: format-check static-analysis test-curated
```

## Common Patterns

### Error Handling Patterns

```c
// Pattern 1: Result structures
typedef struct {
    bool success;
    union {
        ASTNode *node;      // On success
        char *error_message; // On failure
    };
} ParseResult;

// Pattern 2: Output parameters
int parse_expression(Parser *parser, ASTNode **result) {
    // Implementation
    return error_code;
}

// Pattern 3: NULL return with global error
ASTNode *parse_expression(Parser *parser) {
    // Set parser->last_error on failure
    return node_or_null;
}
```

### Resource Management Patterns

```c
// Pattern 1: Constructor/destructor pairs
Parser *parser_create(const char *source);
void parser_destroy(Parser *parser);

// Pattern 2: Init/cleanup for stack objects
void parser_init(Parser *parser, const char *source);
void parser_cleanup(Parser *parser);

// Pattern 3: RAII-style with cleanup functions
#define CLEANUP_PARSER __attribute__((cleanup(parser_cleanup_auto)))
void parser_cleanup_auto(Parser **parser);

void example_function(void) {
    CLEANUP_PARSER Parser parser;
    parser_init(&parser, source);
    // Automatic cleanup on scope exit
}
```

### Iterator Patterns

```c
// Pattern: Iterator with state
typedef struct {
    ASTNode *current;
    ASTNode *root;
    int depth;
    // Internal state
} ASTIterator;

void ast_iterator_init(ASTIterator *iter, ASTNode *root);
ASTNode *ast_iterator_next(ASTIterator *iter);
bool ast_iterator_has_next(ASTIterator *iter);

// Usage
ASTIterator iter;
ast_iterator_init(&iter, root_node);
while (ast_iterator_has_next(&iter)) {
    ASTNode *node = ast_iterator_next(&iter);
    process_node(node);
}
```

## Success Criteria

Your code follows Asthra style guidelines when:

- [ ] **Formatting**: Passes `make format-check` without errors
- [ ] **Static Analysis**: Passes `make static-analysis` without warnings
- [ ] **Naming**: Uses consistent snake_case/PascalCase conventions
- [ ] **Documentation**: All public functions have Doxygen comments
- [ ] **Error Handling**: Consistent error handling patterns used
- [ ] **Memory Management**: Clear ownership semantics and no leaks
- [ ] **Performance**: Appropriate algorithms and data structures
- [ ] **Readability**: Code is self-documenting with clear logic flow

## Related Guides

- **[Development Environment](development-environment.md)** - Setting up tools and IDE
- **[Testing Workflows](../workflows/testing-workflows.md)** - Writing and running tests
- **[Code Review Process](../workflows/code-review.md)** - Review guidelines
- **[Adding Features](../workflows/adding-features.md)** - Feature development workflow

---

**📝 Code Style Mastered!**

*Consistent code style makes the codebase more maintainable, readable, and professional. These guidelines ensure all contributors can work effectively together.* 