# Asthra Compiler Implementation Patterns

This document describes common implementation patterns used throughout the Asthra compiler codebase. Understanding these patterns is essential for maintaining consistency and extending the compiler.

## Table of Contents
1. [Error Handling Patterns](#error-handling-patterns)
2. [Memory Management Patterns](#memory-management-patterns)
3. [AST Construction Patterns](#ast-construction-patterns)
4. [Type System Patterns](#type-system-patterns)
5. [Code Generation Patterns](#code-generation-patterns)
6. [Testing Patterns](#testing-patterns)
7. [Concurrency Patterns](#concurrency-patterns)

## Error Handling Patterns

### Result Type Pattern

The compiler uses a consistent Result type for operations that can fail:

```c
// Generic result type definition
typedef struct {
    bool success;
    union {
        void *value;
        struct {
            AsthraErrorCode code;
            char *message;
            SourceLocation location;
        } error;
    };
} AsthraResult;

// Usage example in parser
AsthraResult parse_expression_result(Parser *parser) {
    AsthraResult result = {.success = false};
    
    ASTNode *expr = parse_expression(parser);
    if (!expr) {
        result.error.code = ASTHRA_ERROR_PARSE_FAILED;
        result.error.message = parser->last_error->message;
        result.error.location = parser->current_token.location;
        return result;
    }
    
    result.success = true;
    result.value = expr;
    return result;
}
```

### Error Propagation Pattern

Errors are propagated up the call stack with context:

```c
// Semantic analyzer error handling
static bool analyze_function_call(SemanticAnalyzer *analyzer, ASTNode *call) {
    // Validate function exists
    SymbolEntry *func = symbol_table_lookup(analyzer->current_scope, 
                                           call->data.call_expr.name);
    if (!func) {
        semantic_error(analyzer, call->location, 
                      "Undefined function: %s", call->data.call_expr.name);
        return false;
    }
    
    // Validate argument count
    if (!validate_argument_count(analyzer, call, func)) {
        return false;  // Error already reported by validate_argument_count
    }
    
    // Type check arguments
    for (size_t i = 0; i < call->data.call_expr.arg_count; i++) {
        if (!analyze_expression(analyzer, call->data.call_expr.args[i])) {
            return false;  // Propagate error from argument analysis
        }
    }
    
    return true;
}
```

### Error Recovery Pattern

The parser implements synchronized error recovery:

```c
// Parser error recovery mechanism
static void synchronize_parser(Parser *parser) {
    parser->panic_mode = false;
    
    // Skip tokens until we find a synchronization point
    while (parser->current_token.type != TOKEN_EOF) {
        // Previous token was a semicolon - likely statement boundary
        if (parser->previous_token.type == TOKEN_SEMICOLON) {
            return;
        }
        
        // Current token starts a new statement/declaration
        switch (parser->current_token.type) {
            case TOKEN_FN:
            case TOKEN_LET:
            case TOKEN_IF:
            case TOKEN_FOR:
            case TOKEN_RETURN:
            case TOKEN_STRUCT:
            case TOKEN_ENUM:
                return;
            default:
                break;
        }
        
        advance_token(parser);
    }
}
```

## Memory Management Patterns

### Reference Counting Pattern

AST nodes use atomic reference counting:

```c
// AST node with reference counting
struct ASTNode {
    ASTNodeType type;
    SourceLocation location;
    atomic_uint_fast32_t ref_count;  // C17 atomic
    ASTNodeData data;
    TypeInfo *type_info;
    ASTNodeFlags flags;
};

// Reference management functions
ASTNode *ast_retain_node(ASTNode *node) {
    if (!node) return NULL;
    atomic_fetch_add_explicit(&node->ref_count, 1, memory_order_relaxed);
    return node;
}

void ast_release_node(ASTNode *node) {
    if (!node) return;
    
    uint32_t old_count = atomic_fetch_sub_explicit(&node->ref_count, 1, 
                                                   memory_order_acq_rel);
    if (old_count == 1) {
        // Last reference - free the node
        ast_free_node(node);
    }
}
```

### Arena Allocation Pattern

The compiler uses arena allocators for temporary allocations:

```c
// Arena allocator for parse-time allocations
typedef struct Arena {
    uint8_t *memory;
    size_t size;
    size_t used;
    struct Arena *next;
} Arena;

typedef struct ArenaAllocator {
    Arena *current;
    Arena *first;
    size_t default_size;
} ArenaAllocator;

// Arena allocation
void *arena_alloc(ArenaAllocator *arena, size_t size) {
    // Align to 8 bytes
    size = (size + 7) & ~7;
    
    if (arena->current->used + size > arena->current->size) {
        // Need new arena
        Arena *new_arena = create_arena(max(arena->default_size, size));
        new_arena->next = arena->current;
        arena->current = new_arena;
    }
    
    void *ptr = arena->current->memory + arena->current->used;
    arena->current->used += size;
    return ptr;
}

// Usage in parser for temporary strings
char *parser_intern_string(Parser *parser, const char *str) {
    size_t len = strlen(str) + 1;
    char *interned = arena_alloc(parser->string_arena, len);
    memcpy(interned, str, len);
    return interned;
}
```

### Pool Allocation Pattern

Frequently allocated objects use pool allocators:

```c
// Pool allocator for uniform-sized objects
typedef struct PoolAllocator {
    void *free_list;
    size_t object_size;
    size_t objects_per_chunk;
    void *chunks;
    atomic_size_t allocated;
    atomic_size_t freed;
} PoolAllocator;

// Symbol entry allocation from pool
SymbolEntry *symbol_entry_alloc(SymbolTable *table) {
    return pool_alloc(table->symbol_pool);
}

void symbol_entry_free(SymbolTable *table, SymbolEntry *entry) {
    // Clear sensitive data
    memset(entry->name, 0, strlen(entry->name));
    pool_free(table->symbol_pool, entry);
}
```

## AST Construction Patterns

### Builder Pattern for Complex Nodes

Complex AST nodes use builder patterns:

```c
// Function declaration builder
typedef struct FunctionBuilder {
    char *name;
    ASTNode **params;
    size_t param_count;
    size_t param_capacity;
    ASTNode *return_type;
    ASTNode *body;
    bool is_public;
    AnnotationList *annotations;
} FunctionBuilder;

FunctionBuilder *function_builder_create(const char *name) {
    FunctionBuilder *builder = calloc(1, sizeof(FunctionBuilder));
    builder->name = strdup(name);
    builder->param_capacity = 4;
    builder->params = calloc(builder->param_capacity, sizeof(ASTNode*));
    return builder;
}

void function_builder_add_param(FunctionBuilder *builder, ASTNode *param) {
    if (builder->param_count >= builder->param_capacity) {
        builder->param_capacity *= 2;
        builder->params = realloc(builder->params, 
                                 builder->param_capacity * sizeof(ASTNode*));
    }
    builder->params[builder->param_count++] = ast_retain_node(param);
}

ASTNode *function_builder_build(FunctionBuilder *builder, SourceLocation loc) {
    ASTNode *func = ast_create_node(AST_FUNCTION_DECL, loc);
    func->data.function_decl.name = builder->name;  // Transfer ownership
    func->data.function_decl.params = builder->params;
    func->data.function_decl.param_count = builder->param_count;
    func->data.function_decl.return_type = builder->return_type;
    func->data.function_decl.body = builder->body;
    func->data.function_decl.is_public = builder->is_public;
    
    free(builder);  // Builder no longer needed
    return func;
}
```

### Visitor Pattern for AST Traversal

AST traversal uses the visitor pattern:

```c
// Generic AST visitor
typedef struct ASTVisitor {
    void *context;
    
    // Visit methods for each node type
    bool (*visit_program)(struct ASTVisitor *visitor, ASTNode *node);
    bool (*visit_function)(struct ASTVisitor *visitor, ASTNode *node);
    bool (*visit_expression)(struct ASTVisitor *visitor, ASTNode *node);
    bool (*visit_statement)(struct ASTVisitor *visitor, ASTNode *node);
    // ... more visit methods
    
    // Pre/post traversal hooks
    void (*enter_scope)(struct ASTVisitor *visitor);
    void (*exit_scope)(struct ASTVisitor *visitor);
} ASTVisitor;

// Generic traversal function
bool ast_traverse(ASTNode *node, ASTVisitor *visitor) {
    if (!node || !visitor) return true;
    
    bool continue_traversal = true;
    
    switch (node->type) {
        case AST_PROGRAM:
            if (visitor->visit_program) {
                continue_traversal = visitor->visit_program(visitor, node);
            }
            break;
            
        case AST_FUNCTION_DECL:
            if (visitor->visit_function) {
                continue_traversal = visitor->visit_function(visitor, node);
            }
            if (continue_traversal && visitor->enter_scope) {
                visitor->enter_scope(visitor);
            }
            // Traverse function body
            if (continue_traversal && node->data.function_decl.body) {
                continue_traversal = ast_traverse(node->data.function_decl.body, 
                                                visitor);
            }
            if (visitor->exit_scope) {
                visitor->exit_scope(visitor);
            }
            break;
            
        // ... handle other node types
    }
    
    return continue_traversal;
}
```

## Type System Patterns

### Type Descriptor Pattern

Types are represented as immutable descriptors:

```c
// Type descriptor hierarchy
typedef enum {
    TYPE_PRIMITIVE,
    TYPE_SLICE,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_POINTER,
    TYPE_RESULT,
    TYPE_GENERIC,
    TYPE_FUNCTION
} TypeKind;

typedef struct TypeInfo {
    TypeKind kind;
    const char *name;
    size_t size;
    size_t alignment;
    atomic_uint_fast32_t ref_count;
    
    union {
        struct {
            PrimitiveType type;
        } primitive;
        
        struct {
            TypeInfo *element_type;
        } slice;
        
        struct {
            TypeInfo *struct_type;
            TypeInfo **type_args;
            size_t type_arg_count;
        } generic_instance;
        
        // ... other type data
    } data;
} TypeInfo;

// Type creation with caching
static TypeInfo *primitive_type_cache[PRIMITIVE_TYPE_COUNT];

TypeInfo *type_create_primitive(PrimitiveType prim) {
    if (primitive_type_cache[prim]) {
        return type_retain(primitive_type_cache[prim]);
    }
    
    TypeInfo *type = type_alloc();
    type->kind = TYPE_PRIMITIVE;
    type->data.primitive.type = prim;
    type->name = primitive_type_names[prim];
    type->size = primitive_type_sizes[prim];
    type->alignment = primitive_type_alignments[prim];
    
    primitive_type_cache[prim] = type_retain(type);
    return type;
}
```

### Type Inference Pattern

Type inference uses constraint propagation:

```c
// Type constraint for inference
typedef struct TypeConstraint {
    TypeInfo *type;
    ASTNode *source;
    enum {
        CONSTRAINT_EXACT,      // Type must match exactly
        CONSTRAINT_ASSIGNABLE, // Type must be assignable
        CONSTRAINT_NUMERIC,    // Type must be numeric
        CONSTRAINT_COMPARABLE  // Type must support comparison
    } kind;
} TypeConstraint;

// Type inference context
typedef struct InferenceContext {
    TypeConstraint *constraints;
    size_t constraint_count;
    size_t constraint_capacity;
    bool has_conflicts;
} InferenceContext;

// Constraint collection during analysis
void collect_constraints(InferenceContext *ctx, ASTNode *node) {
    switch (node->type) {
        case AST_BINARY_EXPR:
            if (is_arithmetic_op(node->data.binary_expr.operator)) {
                // Both operands must be numeric
                add_constraint(ctx, node->data.binary_expr.left, 
                             CONSTRAINT_NUMERIC);
                add_constraint(ctx, node->data.binary_expr.right, 
                             CONSTRAINT_NUMERIC);
                // Result type matches operand types
                add_type_equality_constraint(ctx, 
                    node->data.binary_expr.left,
                    node->data.binary_expr.right);
            }
            break;
            
        // ... other constraint collection
    }
}
```

## Code Generation Patterns

### Instruction Builder Pattern

Code generation uses instruction builders:

```c
// Instruction builder for type-safe instruction generation
typedef struct InstructionBuilder {
    CodeGenerator *generator;
    InstructionBuffer *buffer;
    RegisterAllocator *registers;
} InstructionBuilder;

// MOV instruction generation
void emit_mov(InstructionBuilder *builder, Register dst, Register src) {
    Instruction instr = {
        .opcode = OPCODE_MOV,
        .dst = dst,
        .src1 = src,
        .encoding = select_optimal_encoding(dst, src)
    };
    
    // Emit REX prefix if needed (x86_64)
    if (needs_rex_prefix(dst, src)) {
        emit_rex_prefix(builder, dst, src);
    }
    
    instruction_buffer_append(builder->buffer, &instr);
}

// High-level emission with register allocation
void emit_load_variable(InstructionBuilder *builder, 
                       const char *var_name,
                       Register dst) {
    VariableLocation loc = lookup_variable_location(builder->generator, var_name);
    
    switch (loc.type) {
        case LOC_REGISTER:
            if (loc.reg != dst) {
                emit_mov(builder, dst, loc.reg);
            }
            break;
            
        case LOC_STACK:
            emit_load_from_stack(builder, dst, loc.stack_offset);
            break;
            
        case LOC_GLOBAL:
            emit_load_global(builder, dst, loc.global_symbol);
            break;
    }
}
```

### Peephole Optimization Pattern

Local optimizations use pattern matching:

```c
// Peephole optimization patterns
typedef struct OptPattern {
    Instruction pattern[MAX_PATTERN_LENGTH];
    size_t pattern_length;
    void (*apply)(InstructionBuffer *buffer, size_t match_index);
} OptPattern;

// Common optimization: eliminate redundant moves
static void eliminate_redundant_mov(InstructionBuffer *buffer, size_t index) {
    // Pattern: MOV R1, R2; MOV R2, R1 -> MOV R1, R2
    if (index + 1 < buffer->count) {
        Instruction *i1 = &buffer->instructions[index];
        Instruction *i2 = &buffer->instructions[index + 1];
        
        if (i1->opcode == OPCODE_MOV && i2->opcode == OPCODE_MOV &&
            i1->dst == i2->src1 && i1->src1 == i2->dst) {
            // Remove second instruction
            instruction_buffer_remove(buffer, index + 1);
        }
    }
}

// Pattern matching engine
void apply_peephole_optimizations(InstructionBuffer *buffer) {
    bool changed;
    do {
        changed = false;
        
        for (size_t i = 0; i < buffer->count; i++) {
            for (size_t p = 0; p < pattern_count; p++) {
                if (matches_pattern(buffer, i, &patterns[p])) {
                    patterns[p].apply(buffer, i);
                    changed = true;
                    break;
                }
            }
        }
    } while (changed);
}
```

## Testing Patterns

### Test Fixture Pattern

Tests use fixtures for setup/teardown:

```c
// Test fixture for parser tests
typedef struct ParserTestFixture {
    Lexer *lexer;
    Parser *parser;
    char *source_buffer;
    size_t buffer_size;
} ParserTestFixture;

ParserTestFixture *parser_fixture_create(void) {
    ParserTestFixture *fixture = calloc(1, sizeof(ParserTestFixture));
    fixture->buffer_size = 4096;
    fixture->source_buffer = malloc(fixture->buffer_size);
    return fixture;
}

void parser_fixture_destroy(ParserTestFixture *fixture) {
    if (fixture->parser) parser_destroy(fixture->parser);
    if (fixture->lexer) lexer_destroy(fixture->lexer);
    free(fixture->source_buffer);
    free(fixture);
}

// Helper to parse source string
ASTNode *parser_fixture_parse(ParserTestFixture *fixture, const char *source) {
    strncpy(fixture->source_buffer, source, fixture->buffer_size - 1);
    
    fixture->lexer = lexer_create_from_string(fixture->source_buffer);
    fixture->parser = parser_create(fixture->lexer);
    
    return parser_parse_program(fixture->parser);
}

// Test using fixture
TEST_CASE("parser_function_declaration") {
    ParserTestFixture *fixture = parser_fixture_create();
    
    ASTNode *ast = parser_fixture_parse(fixture, 
        "fn add(x: i32, y: i32) -> i32 { return x + y; }");
    
    ASSERT_NOT_NULL(ast);
    ASSERT_EQ(ast->type, AST_PROGRAM);
    ASSERT_EQ(ast->data.program.declarations[0]->type, AST_FUNCTION_DECL);
    
    parser_fixture_destroy(fixture);
}
```

### Parameterized Test Pattern

Data-driven tests for comprehensive coverage:

```c
// Parameterized test data
typedef struct BinaryOpTestCase {
    const char *name;
    const char *source;
    BinaryOperator expected_op;
    bool should_pass;
} BinaryOpTestCase;

static const BinaryOpTestCase binary_op_tests[] = {
    {"addition", "1 + 2", BINOP_ADD, true},
    {"subtraction", "5 - 3", BINOP_SUB, true},
    {"multiplication", "4 * 2", BINOP_MUL, true},
    {"division", "8 / 2", BINOP_DIV, true},
    {"logical_and", "true && false", BINOP_AND, true},
    {"logical_or", "false || true", BINOP_OR, true},
    {"bitwise_and", "0xFF & 0x0F", BINOP_BITWISE_AND, true},
    {"shift_left", "1 << 4", BINOP_SHL, true},
    {"shift_right", "16 >> 2", BINOP_SHR, true},
    {NULL, NULL, 0, false}  // Sentinel
};

// Parameterized test runner
TEST_CASE("parser_binary_operators") {
    for (const BinaryOpTestCase *tc = binary_op_tests; tc->name; tc++) {
        TEST_SUBCASE(tc->name) {
            ParserTestFixture *fixture = parser_fixture_create();
            
            char source[256];
            snprintf(source, sizeof(source), "fn test() -> i32 { return %s; }", 
                    tc->source);
            
            ASTNode *ast = parser_fixture_parse(fixture, source);
            
            if (tc->should_pass) {
                ASSERT_NOT_NULL(ast);
                // Navigate to binary expression
                ASTNode *ret = ast->data.program.declarations[0]
                                  ->data.function_decl.body
                                  ->data.block.statements[0];
                ASSERT_EQ(ret->type, AST_RETURN_STMT);
                ASTNode *expr = ret->data.return_stmt.value;
                ASSERT_EQ(expr->type, AST_BINARY_EXPR);
                ASSERT_EQ(expr->data.binary_expr.operator, tc->expected_op);
            } else {
                ASSERT_NULL(ast);
            }
            
            parser_fixture_destroy(fixture);
        }
    }
}
```

## Concurrency Patterns

### Lock-Free Data Structure Pattern

The compiler uses lock-free structures for concurrent access:

```c
// Lock-free stack for work stealing
typedef struct LockFreeNode {
    void *data;
    atomic_uintptr_t next;
} LockFreeNode;

typedef struct LockFreeStack {
    atomic_uintptr_t head;
    PoolAllocator *node_pool;
} LockFreeStack;

void lockfree_stack_push(LockFreeStack *stack, void *data) {
    LockFreeNode *node = pool_alloc(stack->node_pool);
    node->data = data;
    
    uintptr_t head = atomic_load(&stack->head);
    do {
        atomic_store(&node->next, head);
    } while (!atomic_compare_exchange_weak(&stack->head, &head, 
                                          (uintptr_t)node));
}

void *lockfree_stack_pop(LockFreeStack *stack) {
    uintptr_t head = atomic_load(&stack->head);
    LockFreeNode *node;
    
    do {
        node = (LockFreeNode *)head;
        if (!node) return NULL;
        
        uintptr_t next = atomic_load(&node->next);
        if (!atomic_compare_exchange_weak(&stack->head, &head, next)) {
            continue;  // Retry
        }
        break;
    } while (true);
    
    void *data = node->data;
    pool_free(stack->node_pool, node);
    return data;
}
```

### Thread Pool Pattern

Parallel compilation uses thread pools:

```c
// Thread pool for parallel semantic analysis
typedef struct WorkItem {
    void (*func)(void *arg);
    void *arg;
    atomic_bool *completed;
} WorkItem;

typedef struct ThreadPool {
    pthread_t *threads;
    size_t thread_count;
    LockFreeStack *work_queue;
    atomic_bool shutdown;
    pthread_cond_t work_available;
    pthread_mutex_t work_mutex;
} ThreadPool;

// Submit work to thread pool
void thread_pool_submit(ThreadPool *pool, void (*func)(void *), void *arg) {
    WorkItem *item = malloc(sizeof(WorkItem));
    item->func = func;
    item->arg = arg;
    item->completed = malloc(sizeof(atomic_bool));
    atomic_init(item->completed, false);
    
    lockfree_stack_push(pool->work_queue, item);
    
    // Signal waiting threads
    pthread_cond_signal(&pool->work_available);
}

// Worker thread function
static void *thread_pool_worker(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    
    while (!atomic_load(&pool->shutdown)) {
        WorkItem *item = lockfree_stack_pop(pool->work_queue);
        
        if (!item) {
            // Wait for work
            pthread_mutex_lock(&pool->work_mutex);
            pthread_cond_wait(&pool->work_available, &pool->work_mutex);
            pthread_mutex_unlock(&pool->work_mutex);
            continue;
        }
        
        // Execute work item
        item->func(item->arg);
        atomic_store(item->completed, true);
    }
    
    return NULL;
}
```

## Summary

These implementation patterns provide:
- **Consistency** across the codebase
- **Performance** through careful design
- **Maintainability** via clear abstractions
- **Safety** with proper error handling
- **Scalability** through concurrent design

When extending the compiler, follow these established patterns to maintain code quality and consistency.