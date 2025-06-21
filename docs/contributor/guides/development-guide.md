# Asthra Compiler Development Guide

This comprehensive guide provides everything needed to understand, develop, and extend the Asthra compiler effectively.

## Table of Contents
1. [Quick Start](#quick-start)
2. [Development Environment Setup](#development-environment-setup)
3. [Understanding the Codebase](#understanding-the-codebase)
4. [Development Workflow](#development-workflow)
5. [Debugging Techniques](#debugging-techniques)
6. [Performance Optimization](#performance-optimization)
7. [Contributing Guidelines](#contributing-guidelines)

## Quick Start

### Building the Compiler

```bash
# Clean build
make clean
make

# Development build with debug info
make debug

# Run all tests
make test

# Run specific test category
make test-category CATEGORY=parser
```

### Using the Compiler

```bash
# Compile a single file
./asthra compile hello.asthra

# Use the Ampu build tool
cd your-project/
ampu build
ampu run
```

### Project Structure Overview

```
asthra/
├── src/                    # Compiler source code
│   ├── parser/            # Lexer and parser
│   ├── analysis/          # Semantic analysis
│   ├── codegen/           # Code generation
│   └── main.c            # Compiler entry point
├── runtime/               # Runtime library
├── tests/                 # Test suite
├── ampu/                  # Build tool (Rust)
├── docs/                  # Documentation
└── make/                  # Modular build system
```

## Development Environment Setup

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential clang gcc git make
sudo apt-get install llvm-dev libclang-dev

# macOS (with Homebrew)
brew install llvm clang gcc make git
xcode-select --install

# Install Rust for Ampu
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

### IDE Configuration

#### VS Code

Create `.vscode/settings.json`:

```json
{
    "C_Cpp.default.compilerPath": "/usr/bin/clang",
    "C_Cpp.default.cStandard": "c17",
    "C_Cpp.default.intelliSenseMode": "clang-x64",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/src/**",
        "${workspaceFolder}/runtime/**",
        "${workspaceFolder}/tests/framework/**"
    ],
    "C_Cpp.default.defines": [
        "ASTHRA_DEBUG=1",
        "_GNU_SOURCE"
    ],
    "files.associations": {
        "*.asthra": "asthra"
    }
}
```

Create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build",
            "type": "shell",
            "command": "make",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false
            }
        },
        {
            "label": "test",
            "type": "shell",
            "command": "make test",
            "group": "test",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false
            }
        }
    ]
}
```

#### CLion

Configure CMake if using CLion:

```cmake
# CMakeLists.txt (for IDE support only)
cmake_minimum_required(VERSION 3.15)
project(asthra C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(src)
include_directories(runtime)
include_directories(tests/framework)

# Add all source files
file(GLOB_RECURSE SOURCES "src/*.c" "runtime/*.c")
add_executable(asthra ${SOURCES})

# Compile definitions
target_compile_definitions(asthra PRIVATE ASTHRA_DEBUG=1)
```

### Environment Variables

```bash
# Add to ~/.bashrc or ~/.zshrc
export ASTHRA_ROOT=/path/to/asthra
export PATH=$ASTHRA_ROOT:$PATH

# Development flags
export ASTHRA_DEBUG=1
export ASTHRA_VERBOSE=1
export PARSER_DEBUG=1
export SEMANTIC_DEBUG=1
```

## Understanding the Codebase

### Key Concepts

#### 1. Grammar-Driven Development

Everything follows `grammar.txt`:

```peg
# Example: Function declaration
FunctionDecl <- 'fn' SimpleIdent '(' ParamList ')' '->' Type Block

# Implementation must match exactly
ASTNode *parse_function_decl(Parser *parser) {
    if (!match_token(parser, TOKEN_FN)) return NULL;
    // ... follow grammar exactly
}
```

#### 2. Reference Counting

AST nodes use atomic reference counting:

```c
// Always use these functions
ASTNode *node = ast_retain_node(other_node);  // Increment ref
ast_release_node(node);                       // Decrement ref

// Never directly access ref_count
```

#### 3. Error Propagation

Consistent error handling pattern:

```c
// Function that can fail
bool analyze_expression(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!expr) return false;
    
    // Try operation
    if (!some_operation(expr)) {
        semantic_error(analyzer, expr->location, "Operation failed");
        return false;  // Propagate failure
    }
    
    return true;  // Success
}
```

#### 4. Modular Architecture

Each component is split into focused modules:

```c
// Main header includes all modules
#include "semantic_analyzer.h"  // Includes everything

// Individual modules for specific work
#include "semantic_types.h"     // Type system only
#include "semantic_symbols.h"   // Symbol tables only
```

### Navigation Guide

#### Finding Code Locations

| Feature | Location | Key Files |
|---------|----------|-----------|
| Keywords | `src/parser/keyword.c` | `keyword.c` |
| Grammar Rules | `src/parser/grammar_*.c` | `grammar_toplevel.c`, `grammar_expressions.c` |
| Type System | `src/analysis/semantic_types*` | `semantic_types.c`, `type_info.h` |
| Symbol Tables | `src/analysis/semantic_symbols*` | `semantic_symbols.c` |
| Code Generation | `src/codegen/code_generator_*.c` | `code_generator_statements.c` |
| Optimization | `src/codegen/optimizer*.c` | `optimizer_passes.c` |
| Runtime | `runtime/asthra_runtime*.h` | `asthra_runtime_modular.h` |

#### Understanding Data Flow

1. **Source → Tokens**
   ```c
   // src/parser/lexer_core.c
   Token next_token(Lexer *lexer);
   ```

2. **Tokens → AST**
   ```c
   // src/parser/parser_core.c
   ASTNode *parser_parse_program(Parser *parser);
   ```

3. **AST → Annotated AST**
   ```c
   // src/analysis/semantic_core.c
   bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program);
   ```

4. **AST → Assembly**
   ```c
   // src/codegen/code_generator.c
   bool generate_program(CodeGenerator *gen, ASTNode *program);
   ```

5. **Assembly → Binary**
   ```c
   // src/codegen/elf_writer.c
   bool write_elf_executable(ELFWriter *writer, const char *filename);
   ```

## Development Workflow

### 1. Setting Up a Feature Branch

```bash
git checkout -b feature/your-feature-name
git checkout -b fix/bug-description
git checkout -b perf/optimization-name
```

### 2. Development Cycle

#### Step 1: Plan the Change

1. **Read the grammar** - Understand what needs to change
2. **Check existing tests** - See what's already covered
3. **Design the implementation** - Plan the approach

#### Step 2: Implement Changes

```bash
# 1. Update grammar if needed
edit grammar.txt

# 2. Run grammar validation
make validate-grammar

# 3. Implement in order: lexer → parser → semantic → codegen
edit src/parser/...
edit src/analysis/...
edit src/codegen/...

# 4. Build frequently
make clean && make

# 5. Test each phase
make test-category CATEGORY=parser
make test-category CATEGORY=semantic
make test-category CATEGORY=codegen
```

#### Step 3: Add Tests

```bash
# Add unit tests
edit tests/features/test_your_feature.c

# Add integration tests
edit tests/integration/test_your_feature_integration.c

# Run tests
make test
```

#### Step 4: Performance Check

```bash
# Run benchmarks
make benchmark

# Check memory usage
make asan-test

# Check for leaks
make valgrind-test
```

### 3. Code Review Checklist

Before submitting changes:

- [ ] Grammar updated if needed
- [ ] All tests pass
- [ ] New tests added for new features
- [ ] Performance benchmarks run
- [ ] Memory safety checked
- [ ] Documentation updated
- [ ] Code follows established patterns

### 4. Testing Strategy

#### Unit Tests

Test individual components:

```c
TEST_CASE("parser_function_declaration") {
    // Test specific parser function
    const char *source = "fn test() -> i32 { return 42; }";
    ASTNode *ast = parse_function_from_string(source);
    
    ASSERT_NOT_NULL(ast);
    ASSERT_EQ(ast->type, AST_FUNCTION_DECL);
    ASSERT_STR_EQ(ast->data.function_decl.name, "test");
}
```

#### Integration Tests

Test component interactions:

```c
TEST_CASE("compile_simple_program") {
    const char *source = R"(
        fn main() -> i32 {
            let x: i32 = 42;
            return x;
        }
    )";
    
    CompileResult result = compile_program_from_string(source);
    ASSERT_TRUE(result.success);
    ASSERT_EQ(execute_program(&result), 42);
}
```

#### Property-Based Tests

Test invariants:

```c
// Property: Parsing then pretty-printing should be identity
PROPERTY_TEST("parse_print_identity", 1000) {
    char *source = generate_random_valid_program();
    ASTNode *ast = parse_program_from_string(source);
    char *printed = ast_pretty_print(ast);
    ASTNode *ast2 = parse_program_from_string(printed);
    
    ASSERT_TRUE(ast_structurally_equal(ast, ast2));
}
```

#### Performance Tests

```c
BENCHMARK("parse_large_file") {
    char *large_source = load_test_file("large_program.asthra");
    
    BENCHMARK_START();
    ASTNode *ast = parse_program_from_string(large_source);
    BENCHMARK_END();
    
    ASSERT_NOT_NULL(ast);
    // Should parse in under 100ms
    ASSERT_LT(BENCHMARK_TIME_MS(), 100);
}
```

## Debugging Techniques

### 1. Parser Debugging

#### Enable Parser Debug Mode

```bash
export PARSER_DEBUG=1
./asthra compile test.asthra
```

#### Custom Debug Output

```c
// In parser code
#ifdef PARSER_DEBUG
    printf("Parsing %s at line %d\n", 
           token_type_name(parser->current_token.type),
           parser->current_token.location.line);
#endif
```

#### AST Visualization

```c
// Debug AST structure
void debug_print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    printf("%*s%s", indent * 2, "", ast_node_type_name(node->type));
    
    switch (node->type) {
        case AST_IDENTIFIER:
            printf(": %s", node->data.identifier.name);
            break;
        case AST_INTEGER_LITERAL:
            printf(": %ld", node->data.integer_literal.value);
            break;
        // ... other cases
    }
    printf("\n");
    
    // Print children
    ast_visit_children(node, debug_print_ast, indent + 1);
}
```

### 2. Semantic Analysis Debugging

#### Symbol Table Inspection

```c
// Print symbol table contents
void debug_print_symbol_table(SymbolTable *table) {
    printf("Symbol Table (scope depth %d):\n", table->scope_depth);
    
    for (size_t i = 0; i < table->bucket_count; i++) {
        SymbolEntry *entry = table->buckets[i];
        while (entry) {
            printf("  %s: %s\n", entry->name, 
                   symbol_type_name(entry->type));
            entry = entry->next;
        }
    }
}
```

#### Type Resolution Tracing

```c
#ifdef SEMANTIC_DEBUG
    printf("Resolving type for %s: %s -> %s\n",
           expr_to_string(expr),
           type_to_string(old_type),
           type_to_string(new_type));
#endif
```

### 3. Code Generation Debugging

#### Assembly Output

```c
// Enable assembly comments
generator->config.emit_comments = true;

// This produces:
/*
    ; Load variable 'x' from stack
    mov %rax, -8(%rbp)  ; x: i32
    
    ; Function call to 'print'
    call print@PLT       ; print(i32) -> void
*/
```

#### Register Allocation Debugging

```c
void debug_print_register_allocation(RegisterAllocator *allocator) {
    printf("Register Allocation:\n");
    for (int reg = 0; reg < NUM_REGISTERS; reg++) {
        if (allocator->registers[reg].in_use) {
            printf("  %s: %s (live: %s)\n",
                   register_name(reg),
                   allocator->registers[reg].variable_name,
                   allocator->registers[reg].is_live ? "yes" : "no");
        }
    }
}
```

### 4. Runtime Debugging

#### Memory Debugging

```bash
# Run with AddressSanitizer
make asan-test

# Run with Valgrind
make valgrind-test

# Enable GC debugging
export ASTHRA_GC_DEBUG=1
export ASTHRA_GC_VERBOSE=1
```

#### Task Debugging

```c
// Enable task system debugging
#ifdef ASTHRA_TASK_DEBUG
    printf("Task %p: state=%s, function=%p\n",
           task, task_state_name(task->state), task->function);
#endif
```

### 5. Using External Tools

#### GDB Integration

Create `.gdbinit`:

```gdb
# Pretty-print AST nodes
define print-ast
    call debug_print_ast($arg0, 0)
end

# Print symbol table
define print-symbols
    call debug_print_symbol_table($arg0)
end

# Set useful breakpoints
break semantic_error
break parser_error
break ast_create_node
```

#### LLDB for macOS

Create `.lldbinit`:

```lldb
# AST printing
command alias print-ast expression debug_print_ast($arg0, 0)

# Symbol table printing  
command alias print-symbols expression debug_print_symbol_table($arg0)
```

#### Static Analysis

```bash
# Clang static analyzer
make analyze

# Cppcheck
cppcheck --enable=all src/

# Clang-tidy
clang-tidy src/**/*.c -- -Isrc -Iruntime
```

### 6. Performance Profiling

#### Compilation Performance

```bash
# Profile compilation time
time make clean && time make

# Profile specific phases
ASTHRA_PROFILE=1 ./asthra compile large_file.asthra
```

#### Runtime Performance

```bash
# Profile generated code
perf record ./your_program
perf report

# Callgrind profiling
valgrind --tool=callgrind ./your_program
kcachegrind callgrind.out.*
```

#### Memory Profiling

```bash
# Massif memory profiler
valgrind --tool=massif ./your_program
ms_print massif.out.*

# Heaptrack (Linux)
heaptrack ./your_program
heaptrack_gui heaptrack.your_program.*
```

## Performance Optimization

### 1. Compilation Performance

#### Hot Paths

Focus optimization on:
- Lexer token scanning
- Parser recursive descent
- Symbol table lookups
- Type checking operations
- Code generation

#### Optimization Techniques

```c
// Use lookup tables instead of switch statements
static const char* const token_names[TOKEN_COUNT] = {
    [TOKEN_IDENTIFIER] = "identifier",
    [TOKEN_INTEGER] = "integer",
    // ... all tokens
};

// Use perfect hashing for keywords
static uint32_t keyword_hash(const char *str, size_t len) {
    // Perfect hash function generated offline
    return hash_function(str, len) % KEYWORD_TABLE_SIZE;
}

// Cache frequently computed values
static TypeInfo *cached_primitive_types[PRIMITIVE_TYPE_COUNT];

TypeInfo *get_primitive_type(PrimitiveType type) {
    if (cached_primitive_types[type]) {
        return type_retain(cached_primitive_types[type]);
    }
    
    TypeInfo *new_type = create_primitive_type(type);
    cached_primitive_types[type] = type_retain(new_type);
    return new_type;
}
```

### 2. Memory Optimization

#### Allocation Strategies

```c
// Use specialized allocators
typedef struct {
    ArenaAllocator *parser_arena;    // Parse-time temps
    PoolAllocator *symbol_pool;      // Symbol entries
    SlabAllocator *type_cache;       // Type descriptors
} CompilerMemory;

// Batch allocations
void batch_allocate_symbols(SymbolTable *table, size_t count) {
    SymbolEntry *batch = pool_alloc_batch(table->symbol_pool, count);
    // Initialize batch...
}
```

#### Data Structure Optimization

```c
// Pack structures to reduce memory
struct __attribute__((packed)) Token {
    TokenType type : 8;        // Only need 8 bits
    uint32_t line : 24;        // 24 bits for line number
    uint16_t column;           // 16 bits for column
    uint32_t length;           // String length
    const char *start;         // String start
};

// Use bit fields for flags
struct ASTNodeFlags {
    bool is_validated : 1;
    bool is_type_checked : 1;
    bool is_constant_expr : 1;
    bool has_side_effects : 1;
    bool is_lvalue : 1;
    bool is_mutable : 1;
    uint8_t reserved : 2;
};
```

### 3. Code Generation Optimization

#### Instruction Selection

```c
// Pattern-based instruction selection
typedef struct {
    ASTNodeType pattern;
    void (*generate)(CodeGenerator *gen, ASTNode *node);
} InstructionPattern;

static const InstructionPattern patterns[] = {
    {AST_BINARY_EXPR, generate_binary_expr},
    {AST_CALL_EXPR, generate_call_expr},
    // ... more patterns
};

void generate_optimized(CodeGenerator *gen, ASTNode *node) {
    for (size_t i = 0; i < pattern_count; i++) {
        if (patterns[i].pattern == node->type) {
            patterns[i].generate(gen, node);
            return;
        }
    }
    
    // Fallback to generic generation
    generate_generic(gen, node);
}
```

#### Register Allocation

```c
// Graph coloring register allocation
typedef struct {
    ASTNode **nodes;
    size_t node_count;
    bool **interference_matrix;
    int *colors;  // Register assignments
} InterferenceGraph;

void color_graph(InterferenceGraph *graph) {
    // Simplified graph coloring
    for (size_t i = 0; i < graph->node_count; i++) {
        int color = find_available_color(graph, i);
        graph->colors[i] = color;
    }
}
```

## Contributing Guidelines

### 1. Code Style

#### Naming Conventions

```c
// Functions: snake_case
bool parse_expression(Parser *parser);
void semantic_analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

// Types: PascalCase
typedef struct ASTNode ASTNode;
typedef enum TokenType TokenType;

// Constants: UPPER_SNAKE_CASE
#define MAX_IDENTIFIER_LENGTH 256
#define DEFAULT_SYMBOL_TABLE_SIZE 64

// Variables: snake_case
ASTNode *current_node = NULL;
size_t symbol_count = 0;
```

#### Code Formatting

Use `clang-format` with the project's `.clang-format`:

```bash
# Format all source files
make format

# Format specific file
clang-format -i src/parser/parser.c
```

#### Documentation

```c
/**
 * Parse a function declaration from the token stream.
 * 
 * Grammar: 'fn' SimpleIdent '(' ParamList ')' '->' Type Block
 * 
 * @param parser The parser instance
 * @return AST node for function declaration, or NULL on error
 * 
 * @note The parser must be positioned at a 'fn' token
 * @note On error, the parser's error list is updated
 */
ASTNode *parse_function_decl(Parser *parser);
```

### 2. Testing Requirements

#### Test Coverage

All new features must include:

1. **Unit tests** for individual functions
2. **Integration tests** for component interaction
3. **Error case tests** for failure modes
4. **Performance tests** for critical paths

#### Test Organization

```
tests/
├── unit/                  # Unit tests by component
│   ├── parser/
│   ├── semantic/
│   └── codegen/
├── integration/           # End-to-end tests
├── features/             # Feature-specific tests
├── performance/          # Benchmarks
└── property/            # Property-based tests
```

### 3. Documentation Requirements

#### When Adding Features

1. Update `grammar.txt` if syntax changes
2. Update language specification
3. Add usage examples
4. Update user manual
5. Add to this development guide

#### Documentation Standards

- Use clear, concise language
- Provide code examples
- Include error handling examples
- Document performance characteristics
- Add diagrams for complex features

### 4. Pull Request Process

#### Before Submitting

```bash
# 1. Ensure all tests pass
make test

# 2. Check code style
make format
make lint

# 3. Run static analysis
make analyze

# 4. Check performance
make benchmark

# 5. Update documentation
# Edit relevant docs
```

#### PR Template

```markdown
## Description
Brief description of changes.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Performance improvement
- [ ] Documentation update
- [ ] Refactoring

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] All tests pass
- [ ] Performance benchmarks run

## Documentation
- [ ] Grammar updated (if applicable)
- [ ] Language spec updated
- [ ] User manual updated
- [ ] Code comments added

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] No compiler warnings
- [ ] Memory leaks checked
```

### 5. Release Process

#### Version Management

```bash
# Update version numbers
edit src/version.h
edit ampu/Cargo.toml
edit docs/spec/README.md

# Tag release
git tag -a v1.x.x -m "Release version 1.x.x"
git push origin v1.x.x
```

#### Release Checklist

- [ ] All tests pass on all platforms
- [ ] Documentation updated
- [ ] Changelog updated
- [ ] Performance benchmarks stable
- [ ] Memory usage within targets
- [ ] Backward compatibility maintained

## Summary

This development guide provides:

1. **Environment setup** for productive development
2. **Codebase navigation** to find relevant code quickly
3. **Development workflow** for consistent practices
4. **Debugging techniques** for efficient problem-solving
5. **Performance optimization** strategies
6. **Contributing guidelines** for quality maintenance

Following these guidelines ensures:
- **Consistent code quality**
- **Maintainable architecture**
- **Reliable functionality**
- **Good performance**
- **Clear documentation**

The Asthra compiler's modular design and comprehensive testing make it straightforward to extend and maintain when following these established patterns and practices.