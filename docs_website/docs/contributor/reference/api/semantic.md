# Semantic API

**Type System Interface**:
```c
// Type creation and management
TypeInfo* type_info_create(TypeCategory category);
void type_info_destroy(TypeInfo* type);
TypeInfo* type_info_clone(TypeInfo* type);

// Type queries
bool type_info_is_numeric(TypeInfo* type);
bool type_info_is_pointer(TypeInfo* type);
bool type_info_is_generic(TypeInfo* type);
size_t type_info_get_size(TypeInfo* type);

// Type operations
bool type_info_equals(TypeInfo* type1, TypeInfo* type2);
bool type_info_get_element_type(TypeInfo* array_or_slice);
TypeInfo* type_info_instantiate(TypeInfo* generic, TypeInfo** args, size_t count);
```

**Symbol Table Interface**:
```c
// Symbol creation
Symbol* symbol_create(SymbolType type, const char* name, TypeInfo* type_info);
void symbol_destroy(Symbol* symbol);

// Symbol table operations
SymbolTable* symbol_table_create_global(void);
SymbolTable* symbol_table_create_function(SymbolTable* parent, const char* name);
SymbolTable* symbol_table_create_block(SymbolTable* parent);

// Symbol resolution
Symbol* resolve_symbol(const char* name, SymbolTable* table);
bool add_symbol(SymbolTable* table, Symbol* symbol);
```

**Error Reporting System**:
```c
typedef struct ErrorReporter {
    SemanticError* errors;         // Error array
    size_t error_count;
    size_t error_capacity;
    bool has_fatal_error;          // Fatal error flag
    size_t max_errors;             // Error limit
} ErrorReporter;

// Error reporting functions
void report_type_mismatch_error(ErrorReporter* reporter, ASTNode* node,
                               TypeInfo* expected, TypeInfo* actual);
void report_undefined_symbol_error(ErrorReporter* reporter, ASTNode* node,
                                  const char* symbol_name);
void report_annotation_conflict_error(ErrorReporter* reporter, 
                                     ASTNode* annotation1, ASTNode* annotation2);
```

**AST Enhancement**:
```c
// Attach semantic information to AST nodes
void ast_node_set_type_info(ASTNode* node, TypeInfo* type);
void ast_node_set_symbol_info(ASTNode* node, Symbol* symbol);
void ast_node_set_semantic_flags(ASTNode* node, SemanticFlags flags);

// Query semantic information
TypeInfo* ast_node_get_type_info(ASTNode* node);
Symbol* ast_node_get_symbol_info(ASTNode* node);
bool ast_node_has_semantic_error(ASTNode* node);
``` 