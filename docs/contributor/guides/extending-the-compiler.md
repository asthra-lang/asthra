# Guide to Extending the Asthra Compiler

This guide provides step-by-step instructions for adding new features to the Asthra compiler. It covers the complete process from grammar updates to testing.

## Table of Contents
1. [Adding a New Language Feature](#adding-a-new-language-feature)
2. [Adding a New Built-in Type](#adding-a-new-built-in-type)
3. [Adding a New Optimization Pass](#adding-a-new-optimization-pass)
4. [Supporting a New Target Architecture](#supporting-a-new-target-architecture)
5. [Adding New Runtime Functions](#adding-new-runtime-functions)
6. [Extending the Type System](#extending-the-type-system)
7. [Adding New Test Categories](#adding-new-test-categories)

## Adding a New Language Feature

### Example: Adding a `defer` Statement

Let's walk through adding a `defer` statement that executes code at the end of the current scope.

#### Step 1: Update the Grammar

First, update `grammar.txt` to include the new syntax:

```diff
# In the Statement production
Statement      <- VarDecl / AssignStmt / IfLetStmt / IfStmt / ForStmt / 
+                 DeferStmt / ReturnStmt / SpawnStmt / ExprStmt / 
                  UnsafeBlock / MatchStmt

# Add the defer statement production
+DeferStmt      <- 'defer' Block      # Execute block at scope exit
```

#### Step 2: Add Token Type

Update `src/parser/lexer.h`:

```c
typedef enum {
    // ... existing tokens ...
    TOKEN_CONST,
    TOKEN_DEFER,    // Add new token
    TOKEN_ENUM,
    // ... rest of tokens ...
} TokenType;
```

Update `src/parser/keyword.c` to recognize the keyword:

```c
static const KeywordEntry keyword_table[] = {
    // ... existing keywords ...
    {"const", TOKEN_CONST},
    {"defer", TOKEN_DEFER},    // Add entry
    {"enum", TOKEN_ENUM},
    // ... rest of keywords ...
};
```

#### Step 3: Add AST Node Type

Update `src/parser/ast_types.h`:

```c
typedef enum {
    // ... existing node types ...
    AST_RETURN_STMT,
    AST_DEFER_STMT,    // Add new node type
    AST_IF_STMT,
    // ... rest of node types ...
} ASTNodeType;
```

Update `src/parser/ast.h` to add the data structure:

```c
// In the ASTNodeData union
union ASTNodeData {
    // ... existing node data ...
    
    struct {
        ASTNode *body;  // Code to defer
    } defer_stmt;
    
    // ... rest of node data ...
};
```

#### Step 4: Implement Parser Production

Add to `src/parser/grammar_statements.c`:

```c
// Parse defer statement
ASTNode *parse_defer_stmt(Parser *parser) {
    if (!parser || !match_token(parser, TOKEN_DEFER)) {
        return NULL;
    }
    
    SourceLocation defer_loc = parser->current_token.location;
    advance_token(parser);  // consume 'defer'
    
    // Parse the block to defer
    ASTNode *body = parse_block(parser);
    if (!body) {
        parser_error(parser, defer_loc, "Expected block after 'defer'");
        return NULL;
    }
    
    // Create defer statement node
    ASTNode *defer_node = ast_create_node(AST_DEFER_STMT, defer_loc);
    if (!defer_node) {
        ast_free_node(body);
        return NULL;
    }
    
    defer_node->data.defer_stmt.body = body;
    return defer_node;
}
```

Update `parse_statement()` to include defer:

```c
ASTNode *parse_statement(Parser *parser) {
    // ... existing cases ...
    
    if (match_token(parser, TOKEN_DEFER)) {
        return parse_defer_stmt(parser);
    }
    
    // ... rest of statement parsing ...
}
```

#### Step 5: Implement Semantic Analysis

Add to `src/analysis/semantic_statements.c`:

```c
// Semantic analysis for defer statements
static bool analyze_defer_stmt(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    assert(stmt && stmt->type == AST_DEFER_STMT);
    
    // Track defer statements in current scope
    if (!analyzer->current_scope->defer_stack) {
        analyzer->current_scope->defer_stack = defer_stack_create();
    }
    
    // Analyze the deferred block
    bool success = analyze_block(analyzer, stmt->data.defer_stmt.body);
    
    if (success) {
        // Add to defer stack for current scope
        defer_stack_push(analyzer->current_scope->defer_stack, stmt);
        
        // Mark that this scope has defer statements
        analyzer->current_scope->has_defer = true;
    }
    
    return success;
}
```

Update `analyze_statement()` to handle defer:

```c
bool analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    // ... existing cases ...
    
    case AST_DEFER_STMT:
        return analyze_defer_stmt(analyzer, stmt);
        
    // ... rest of cases ...
}
```

#### Step 6: Implement Code Generation

Add to `src/codegen/code_generator_statements.c`:

```c
// Code generation for defer statements
static void generate_defer_stmt(CodeGenerator *gen, ASTNode *stmt) {
    assert(stmt && stmt->type == AST_DEFER_STMT);
    
    // Defer statements are not generated immediately
    // Instead, they're collected and generated at scope exit
    
    // Add to current function's defer list
    if (!gen->current_function_defers) {
        gen->current_function_defers = defer_list_create();
    }
    
    DeferEntry *entry = malloc(sizeof(DeferEntry));
    entry->stmt = stmt;
    entry->scope_depth = gen->current_scope_depth;
    
    defer_list_append(gen->current_function_defers, entry);
}

// Generate deferred statements when exiting scope
void generate_deferred_statements(CodeGenerator *gen, int target_scope_depth) {
    if (!gen->current_function_defers) return;
    
    // Generate defer statements in reverse order (LIFO)
    for (int i = gen->current_function_defers->count - 1; i >= 0; i--) {
        DeferEntry *entry = gen->current_function_defers->entries[i];
        
        // Only generate if we're exiting this defer's scope
        if (entry->scope_depth >= target_scope_depth) {
            generate_block(gen, entry->stmt->data.defer_stmt.body);
            
            // Remove from list
            defer_list_remove(gen->current_function_defers, i);
        }
    }
}
```

Update scope exit points to generate defers:

```c
// In generate_return_stmt()
static void generate_return_stmt(CodeGenerator *gen, ASTNode *stmt) {
    // Generate any deferred statements before returning
    generate_deferred_statements(gen, 0);  // Exit all scopes
    
    // ... existing return generation ...
}

// In generate_block() at the end
static void generate_block(CodeGenerator *gen, ASTNode *block) {
    gen->current_scope_depth++;
    
    // ... generate block statements ...
    
    // Generate defers for this scope before exiting
    generate_deferred_statements(gen, gen->current_scope_depth);
    
    gen->current_scope_depth--;
}
```

#### Step 7: Add Tests

Create `tests/features/test_defer.c`:

```c
#include "../framework/test_framework.h"
#include "../../src/compiler.h"

TEST_CASE("defer_basic_execution") {
    const char *source = R"(
        fn test() -> i32 {
            let mut x: i32 = 1;
            defer {
                x = 2;
            }
            return x;  // Should return 1, defer executes after
        }
    )";
    
    CompileResult result = compile_string(source);
    ASSERT_TRUE(result.success);
    
    // Execute and check deferred assignment happened
    int exit_code = execute_result(&result);
    ASSERT_EQ(exit_code, 1);  // Original value returned
    
    // Check that defer was recorded in metadata
    ASSERT_TRUE(result.metadata.has_defer_statements);
}

TEST_CASE("defer_multiple_reverse_order") {
    const char *source = R"(
        fn test() -> void {
            defer { print("3"); }
            defer { print("2"); }
            defer { print("1"); }
            print("0");
        }
    )";
    
    CompileResult result = compile_string(source);
    ASSERT_TRUE(result.success);
    
    // Capture output
    char output[256];
    execute_with_output(&result, output, sizeof(output));
    
    // Should print "0123" (defers in reverse order)
    ASSERT_STR_EQ(output, "0123");
}

TEST_CASE("defer_with_early_return") {
    const char *source = R"(
        fn test(should_return: bool) -> i32 {
            defer { cleanup(); }
            
            if should_return {
                return 42;  // Defer still executes
            }
            
            return 0;
        }
    )";
    
    CompileResult result = compile_string(source);
    ASSERT_TRUE(result.success);
    
    // Verify defer is called on both paths
    ASSERT_TRUE(verify_cleanup_called(&result, true));
    ASSERT_TRUE(verify_cleanup_called(&result, false));
}

// Add to test registry
void register_defer_tests(TestSuite *suite) {
    test_suite_add(suite, test_defer_basic_execution);
    test_suite_add(suite, test_defer_multiple_reverse_order);
    test_suite_add(suite, test_defer_with_early_return);
}
```

#### Step 8: Update Documentation

Add to language specification and user manual documenting the defer statement behavior.

## Adding a New Built-in Type

### Example: Adding a `u128` Type

#### Step 1: Update Type System

Add to `src/analysis/type_info.h`:

```c
typedef enum {
    // ... existing types ...
    PRIMITIVE_U64,
    PRIMITIVE_U128,   // Add new type
    PRIMITIVE_I8,
    // ... rest of types ...
} PrimitiveType;
```

#### Step 2: Update Type Information

In `src/analysis/type_info_builtins.c`:

```c
static const TypeInfo builtin_types[] = {
    // ... existing types ...
    [PRIMITIVE_U128] = {
        .kind = TYPE_PRIMITIVE,
        .name = "u128",
        .size = 16,
        .alignment = 16,
        .data.primitive = { .type = PRIMITIVE_U128 }
    },
    // ... rest of types ...
};
```

#### Step 3: Update Lexer

Add to keyword recognition in `src/parser/keyword.c`:

```c
static const KeywordEntry type_keywords[] = {
    // ... existing types ...
    {"u64", TOKEN_TYPE_U64},
    {"u128", TOKEN_TYPE_U128},   // Add entry
    {"i8", TOKEN_TYPE_I8},
    // ... rest of types ...
};
```

#### Step 4: Update Parser

In `src/parser/parser_types.c`:

```c
ASTNode *parse_primitive_type(Parser *parser) {
    // ... existing cases ...
    
    case TOKEN_TYPE_U128:
        type_node->data.type.primitive = PRIMITIVE_U128;
        advance_token(parser);
        break;
        
    // ... rest of cases ...
}
```

#### Step 5: Update Code Generation

In `src/codegen/code_generator_types.c`:

```c
size_t get_type_size(TypeInfo *type) {
    if (type->kind == TYPE_PRIMITIVE) {
        switch (type->data.primitive.type) {
            // ... existing cases ...
            case PRIMITIVE_U128: return 16;
            // ... rest of cases ...
        }
    }
    // ... rest of function ...
}

// Add u128 arithmetic operations
void generate_u128_add(CodeGenerator *gen, Register dst_hi, Register dst_lo,
                       Register src1_hi, Register src1_lo,
                       Register src2_hi, Register src2_lo) {
    // Generate 128-bit addition
    emit_add(gen, dst_lo, src1_lo, src2_lo);      // Add low parts
    emit_adc(gen, dst_hi, src1_hi, src2_hi);      // Add high parts with carry
}
```

## Adding a New Optimization Pass

### Example: Dead Store Elimination

#### Step 1: Define the Pass

Create `src/codegen/opt_dead_store.c`:

```c
#include "optimizer.h"

typedef struct {
    InstructionBuffer *buffer;
    DataFlowInfo *df_info;
    BitVector *live_out;
    size_t stores_eliminated;
} DeadStoreContext;

// Dead store elimination pass
OptimizationResult dead_store_elimination(CodeGenerator *gen) {
    DeadStoreContext ctx = {
        .buffer = gen->instruction_buffer,
        .df_info = compute_dataflow_info(gen),
        .stores_eliminated = 0
    };
    
    // Compute liveness information
    compute_liveness(&ctx);
    
    // Eliminate dead stores
    for (size_t i = 0; i < ctx.buffer->count; i++) {
        Instruction *instr = &ctx.buffer->instructions[i];
        
        if (is_store_instruction(instr)) {
            if (!is_store_live(&ctx, i, instr)) {
                // Mark for removal
                instr->opcode = OPCODE_NOP;
                ctx.stores_eliminated++;
            }
        }
    }
    
    // Remove NOPs
    compact_instruction_buffer(ctx.buffer);
    
    return (OptimizationResult){
        .changed = ctx.stores_eliminated > 0,
        .stats = {
            .instructions_removed = ctx.stores_eliminated
        }
    };
}
```

#### Step 2: Register the Pass

In `src/codegen/optimizer.c`:

```c
static const OptimizationPass optimization_passes[] = {
    // ... existing passes ...
    {
        .name = "dead-store-elimination",
        .level = OPT_LEVEL_2,
        .function = dead_store_elimination,
        .requires_cfg = true,
        .requires_dataflow = true
    },
    // ... rest of passes ...
};
```

#### Step 3: Add Pass Configuration

In `src/codegen/optimizer.h`:

```c
typedef enum {
    // ... existing flags ...
    OPT_FLAG_DEAD_STORE_ELIM = 1 << 5,
    // ... rest of flags ...
} OptimizationFlags;
```

## Supporting a New Target Architecture

### Example: Adding RISC-V Support

#### Step 1: Define Architecture

In `src/codegen/code_generator_types.h`:

```c
typedef enum {
    ARCH_X86_64,
    ARCH_ARM64,
    ARCH_RISCV64,    // Add new architecture
    ARCH_WASM32,
} TargetArchitecture;
```

#### Step 2: Create Architecture Module

Create `src/codegen/arch_riscv64.c`:

```c
#include "code_generator.h"

// RISC-V register definitions
typedef enum {
    RISCV_X0 = 0,   // Zero register
    RISCV_X1,       // Return address
    RISCV_X2,       // Stack pointer
    // ... define all registers ...
} RiscVRegister;

// RISC-V instruction encoding
typedef struct {
    uint32_t opcode : 7;
    uint32_t rd : 5;
    uint32_t funct3 : 3;
    uint32_t rs1 : 5;
    uint32_t rs2 : 5;
    uint32_t funct7 : 7;
} RiscVRType;

// Generate RISC-V instructions
void riscv_emit_add(CodeGenerator *gen, int rd, int rs1, int rs2) {
    RiscVRType instr = {
        .opcode = 0x33,  // OP
        .rd = rd,
        .funct3 = 0x0,   // ADD
        .rs1 = rs1,
        .rs2 = rs2,
        .funct7 = 0x00
    };
    
    emit_bytes(gen, &instr, sizeof(instr));
}

// Architecture-specific initialization
void init_riscv64_codegen(CodeGenerator *gen) {
    gen->arch_data = calloc(1, sizeof(RiscV64Data));
    gen->emit_prologue = riscv_emit_function_prologue;
    gen->emit_epilogue = riscv_emit_function_epilogue;
    gen->emit_call = riscv_emit_call;
    // ... set other function pointers ...
}
```

#### Step 3: Update Code Generator Factory

In `src/codegen/code_generator.c`:

```c
CodeGenerator *code_generator_create(TargetArchitecture arch, 
                                   CallingConvention conv) {
    // ... existing code ...
    
    switch (arch) {
        // ... existing cases ...
        
        case ARCH_RISCV64:
            init_riscv64_codegen(generator);
            break;
            
        // ... rest of cases ...
    }
    
    return generator;
}
```

## Adding New Runtime Functions

### Example: Adding Atomic Operations

#### Step 1: Define Runtime Interface

In `runtime/asthra_runtime_atomics.h`:

```c
// Atomic operations for Asthra runtime
typedef struct {
    atomic_int value;
} AsthraAtomicInt;

// Atomic integer operations
AsthraAtomicInt *asthra_atomic_int_create(int initial);
void asthra_atomic_int_destroy(AsthraAtomicInt *atomic);
int asthra_atomic_int_load(AsthraAtomicInt *atomic);
void asthra_atomic_int_store(AsthraAtomicInt *atomic, int value);
int asthra_atomic_int_exchange(AsthraAtomicInt *atomic, int value);
bool asthra_atomic_int_compare_exchange(AsthraAtomicInt *atomic, 
                                       int *expected, int desired);
int asthra_atomic_int_fetch_add(AsthraAtomicInt *atomic, int delta);
```

#### Step 2: Implement Runtime Functions

Create `runtime/asthra_atomic_ops.c`:

```c
#include "asthra_runtime_atomics.h"

AsthraAtomicInt *asthra_atomic_int_create(int initial) {
    AsthraAtomicInt *atomic = asthra_alloc(sizeof(AsthraAtomicInt), 
                                          ASTHRA_ZONE_GC);
    if (atomic) {
        atomic_init(&atomic->value, initial);
    }
    return atomic;
}

int asthra_atomic_int_fetch_add(AsthraAtomicInt *atomic, int delta) {
    return atomic_fetch_add_explicit(&atomic->value, delta, 
                                    memory_order_seq_cst);
}

// ... implement other functions ...
```

#### Step 3: Add Compiler Support

In `src/analysis/semantic_builtins.c`:

```c
// Register atomic type and functions
static void register_atomic_builtins(SemanticAnalyzer *analyzer) {
    // Register AtomicInt type
    TypeInfo *atomic_int_type = type_create_struct("AtomicInt");
    atomic_int_type->data.struct_type.is_builtin = true;
    symbol_table_insert(analyzer->global_scope, "AtomicInt", 
                       SYMBOL_TYPE, atomic_int_type);
    
    // Register atomic functions
    register_builtin_function(analyzer, "atomic_create",
                            atomic_int_type, 1, type_i32());
    register_builtin_function(analyzer, "atomic_load",
                            type_i32(), 1, atomic_int_type);
    register_builtin_function(analyzer, "atomic_fetch_add",
                            type_i32(), 2, atomic_int_type, type_i32());
    // ... register other functions ...
}
```

## Extending the Type System

### Example: Adding Dependent Types

#### Step 1: Define Type Structure

In `src/analysis/type_info.h`:

```c
typedef enum {
    // ... existing kinds ...
    TYPE_DEPENDENT,    // Add dependent type
} TypeKind;

typedef struct {
    TypeInfo *base_type;
    ASTNode *constraint;    // Expression constraining the type
    bool is_verified;
} DependentTypeData;

// In TypeInfo union
union {
    // ... existing type data ...
    DependentTypeData dependent;
};
```

#### Step 2: Add Type Creation

In `src/analysis/type_info_creators.c`:

```c
TypeInfo *type_create_dependent(TypeInfo *base, ASTNode *constraint) {
    TypeInfo *type = type_alloc();
    type->kind = TYPE_DEPENDENT;
    type->name = format_dependent_type_name(base, constraint);
    type->size = base->size;
    type->alignment = base->alignment;
    
    type->data.dependent.base_type = type_retain(base);
    type->data.dependent.constraint = ast_retain_node(constraint);
    type->data.dependent.is_verified = false;
    
    return type;
}
```

#### Step 3: Add Type Checking

In `src/analysis/semantic_types.c`:

```c
bool check_dependent_type_constraint(SemanticAnalyzer *analyzer,
                                   TypeInfo *dep_type,
                                   ASTNode *value_expr) {
    // Evaluate constraint with value
    ConstraintContext ctx = {
        .analyzer = analyzer,
        .value_expr = value_expr,
        .base_type = dep_type->data.dependent.base_type
    };
    
    bool satisfied = evaluate_constraint(&ctx, 
                                       dep_type->data.dependent.constraint);
    
    if (!satisfied) {
        semantic_error(analyzer, value_expr->location,
                      "Value does not satisfy dependent type constraint");
    }
    
    return satisfied;
}
```

## Adding New Test Categories

### Example: Adding Property-Based Tests

#### Step 1: Create Test Category

Create `tests/property/`:
```bash
mkdir tests/property
```

#### Step 2: Define Property Test Framework

Create `tests/property/property_framework.h`:

```c
#ifndef PROPERTY_TEST_FRAMEWORK_H
#define PROPERTY_TEST_FRAMEWORK_H

#include "../framework/test_framework.h"

// Property test generator
typedef void* (*PropertyGenerator)(size_t seed);

// Property checker
typedef bool (*PropertyChecker)(void *input);

// Property test definition
typedef struct {
    const char *name;
    PropertyGenerator generator;
    PropertyChecker checker;
    size_t num_tests;
} PropertyTest;

// Run property test
bool run_property_test(PropertyTest *test);

// Common generators
void* gen_random_int(size_t seed);
void* gen_random_string(size_t seed);
void* gen_random_ast(size_t seed);

#endif
```

#### Step 3: Add to Build System

Update `make/tests.mk`:

```makefile
# Property-based tests
PROPERTY_TEST_SOURCES := $(wildcard tests/property/*.c)
PROPERTY_TEST_OBJECTS := $(PROPERTY_TEST_SOURCES:.c=.o)

test-property: $(PROPERTY_TEST_OBJECTS)
	$(CC) $(CFLAGS) -o test_property $^ $(LDFLAGS)
	./test_property

test-category-property: test-property
```

#### Step 4: Create Property Tests

Create `tests/property/test_parser_properties.c`:

```c
#include "property_framework.h"
#include "../../src/parser/parser.h"

// Property: Parsing and pretty-printing should round-trip
bool prop_parse_print_roundtrip(void *input) {
    char *source = (char *)input;
    
    // Parse source
    Lexer *lexer = lexer_create_from_string(source);
    Parser *parser = parser_create(lexer);
    ASTNode *ast1 = parser_parse_program(parser);
    
    if (!ast1) {
        // Invalid input, skip
        parser_destroy(parser);
        lexer_destroy(lexer);
        return true;
    }
    
    // Pretty-print AST
    char *printed = ast_to_source(ast1);
    
    // Parse printed source
    Lexer *lexer2 = lexer_create_from_string(printed);
    Parser *parser2 = parser_create(lexer2);
    ASTNode *ast2 = parser_parse_program(parser2);
    
    // Compare ASTs
    bool equal = ast_deep_equal(ast1, ast2);
    
    // Cleanup
    ast_free_node(ast1);
    ast_free_node(ast2);
    parser_destroy(parser);
    parser_destroy(parser2);
    lexer_destroy(lexer);
    lexer_destroy(lexer2);
    free(printed);
    
    return equal;
}

// Register property tests
void register_parser_property_tests(void) {
    PropertyTest tests[] = {
        {
            .name = "parse_print_roundtrip",
            .generator = gen_random_valid_source,
            .checker = prop_parse_print_roundtrip,
            .num_tests = 1000
        },
        // ... more properties ...
    };
    
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        run_property_test(&tests[i]);
    }
}
```

## Best Practices for Extensions

### 1. Always Update Grammar First
- The grammar is the source of truth
- Never implement features not in the grammar
- Update `grammar.txt` before any code changes

### 2. Follow the Pipeline
- Lexer → Parser → Semantic → CodeGen → Runtime
- Don't skip phases
- Each phase should be complete before moving on

### 3. Add Comprehensive Tests
- Unit tests for each component
- Integration tests for the feature
- Property-based tests for robustness
- Performance benchmarks if applicable

### 4. Update Documentation
- Add to language specification
- Update user manual
- Add code examples
- Document any limitations

### 5. Consider Error Cases
- What happens with invalid input?
- How are errors reported?
- Is error recovery possible?
- Are error messages helpful?

### 6. Maintain Compatibility
- Don't break existing code
- Use feature flags if needed
- Provide migration guides
- Test with existing codebases

### 7. Performance Considerations
- Profile before and after
- Avoid algorithmic complexity increases
- Use appropriate data structures
- Consider memory usage

## Summary

Extending the Asthra compiler requires:
1. **Understanding the architecture** - Know how components interact
2. **Following established patterns** - Maintain consistency
3. **Comprehensive testing** - Ensure reliability
4. **Good documentation** - Help future developers
5. **Performance awareness** - Keep the compiler fast

The modular architecture makes extensions straightforward, but attention to detail ensures quality.