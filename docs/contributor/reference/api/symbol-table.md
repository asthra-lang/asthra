# Symbol Table API

### Symbol Table Implementation

**Symbol Types** (`src/semantic/semantic_symbols.h`):
```c
typedef enum {
    SYMBOL_VARIABLE,       // Local and global variables
    SYMBOL_FUNCTION,       // Function declarations
    SYMBOL_STRUCT,         // Struct type definitions
    SYMBOL_ENUM,           // Enum type definitions
    SYMBOL_FIELD,          // Struct/enum fields
    SYMBOL_PARAMETER,      // Function parameters
    SYMBOL_TYPE_PARAMETER, // Generic type parameters
    SYMBOL_IMPORT,         // Imported symbols
    SYMBOL_PACKAGE        // Package declarations
} SymbolType;
```

**Symbol Information**:
```c
typedef struct Symbol {
    SymbolType type;
    char* name;                    // Symbol name
    TypeInfo* type_info;           // Associated type
    ASTNode* declaration;          // Declaration AST node
    VisibilityModifier visibility; // pub/priv visibility
    bool is_mutable;               // Mutability flag
    bool is_used;                  // Usage tracking
    size_t declaration_line;       // Source location
    size_size_t declaration_column;
    
    // Symbol-specific data
    union {
        struct {
            bool is_initialized;
            ASTNode* initializer;
        } variable;
        
        struct {
            Symbol** parameters;
            size_t parameter_count;
            bool is_generic;
            Symbol** type_parameters;
            size_t type_parameter_count;
        } function;
        
        struct {
            Symbol** fields;
            size_t field_count;
            bool is_generic;
        } struct_symbol;
        
        struct {
            Symbol** variants;
            size_t variant_count;
            bool is_generic;
        } enum_symbol;
    } data;
} Symbol;
```

**Symbol Table Structure**:
```c
typedef struct SymbolTable {
    struct SymbolTable* parent;    // Parent scope
    Symbol** symbols;              // Symbol array
    size_t symbol_count;
    size_t symbol_capacity;
    char* scope_name;              // Scope identifier
    ScopeType scope_type;          // Function, block, etc.
    
    // Hash table for fast lookup
    SymbolHashEntry** hash_table;
    size_t hash_table_size;
} SymbolTable;
```

### Scope Resolution

**Scope Management**:
```c
// Scope lifecycle
SymbolTable* symbol_table_create(SymbolTable* parent, const char* scope_name);
void symbol_table_destroy(SymbolTable* table);

// Symbol operations
bool symbol_table_add(SymbolTable* table, Symbol* symbol);
Symbol* symbol_table_lookup(SymbolTable* table, const char* name);
bool symbol_table_lookup_local(SymbolTable* table, const char* name);
Symbol* symbol_table_lookup_recursive(SymbolTable* table, const char* name);

// Scope navigation
SymbolTable* symbol_table_enter_scope(SymbolTable* current, const char* name);
SymbolTable* symbol_table_exit_scope(SymbolTable* current);
```

**Symbol Resolution Algorithm**:
```c
Symbol* resolve_identifier(ASTNode* identifier, SymbolTable* symbols) {
    const char* name = identifier->data.identifier.name;
    
    // Look up symbol in current scope and parent scopes
    Symbol* symbol = symbol_table_lookup_recursive(symbols, name);
    
    if (!symbol) {
        report_undefined_symbol_error(identifier, name);
        return NULL;
    }
    
    // Check visibility
    if (!is_symbol_accessible(symbol, symbols)) {
        report_inaccessible_symbol_error(identifier, symbol);
        return NULL;
    }
    
    // Mark symbol as used
    symbol->is_used = true;
    
    // Attach symbol information to AST node
    ast_node_set_symbol_info(identifier, symbol);
    
    return symbol;
}
```

### Import Resolution

**Import System** (`src/semantic/semantic_imports.c`):
```c
typedef struct ImportInfo {
    char* module_path;             // Import path
    char* alias;                   // Optional alias
    SymbolTable* imported_symbols; // Imported symbol table
    bool is_wildcard;              // import module::*
} ImportInfo;

// Import processing
bool process_import_declaration(ASTNode* import_decl, SymbolTable* symbols);
SymbolTable* load_module_symbols(const char* module_path);
bool resolve_imported_symbol(const char* name, ImportInfo* import, Symbol** result);
``` 