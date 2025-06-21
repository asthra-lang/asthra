# AST API

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