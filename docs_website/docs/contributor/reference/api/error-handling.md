# Error Handling API

### Error Categories

**Semantic Error Types**:
```c
typedef enum {
    SEMANTIC_ERROR_TYPE_MISMATCH,
    SEMANTIC_ERROR_UNDEFINED_SYMBOL,
    SEMANTIC_ERROR_REDEFINED_SYMBOL,
    SEMANTIC_ERROR_INVALID_ASSIGNMENT,
    SEMANTIC_ERROR_INVALID_OPERATION,
    SEMANTIC_ERROR_ANNOTATION_CONFLICT,
    SEMANTIC_ERROR_TIER_VIOLATION,
    SEMANTIC_ERROR_GENERIC_CONSTRAINT,
    SEMANTIC_ERROR_IMPORT_ERROR,
    SEMANTIC_ERROR_VISIBILITY_VIOLATION
} SemanticErrorType;
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

**Error Message Quality**:
```c
// Example error message generation
void report_type_mismatch_error(ErrorReporter* reporter, ASTNode* node,
                               TypeInfo* expected, TypeInfo* actual) {
    char* message = format_string(
        "Type mismatch: expected '%s', found '%s'",
        type_info_to_string(expected),
        type_info_to_string(actual)
    );
    
    char* suggestion = NULL;
    if (can_suggest_conversion(expected, actual)) {
        suggestion = format_string(
            "Try explicit conversion: %s(%s)",
            type_info_to_string(expected),
            ast_node_to_string(node)
        );
    }
    
    add_error(reporter, SEMANTIC_ERROR_TYPE_MISMATCH, node,
              message, suggestion);
}
``` 