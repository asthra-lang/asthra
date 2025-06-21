#ifndef ASTHRA_AI_SEMANTIC_API_H
#define ASTHRA_AI_SEMANTIC_API_H

#include "../analysis/semantic_core.h"
#include "../parser/ast.h"
#include "semantic_cache.h"
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations
typedef struct SemanticAnalyzer SemanticAnalyzer;
typedef struct SymbolTable SymbolTable;
typedef struct ASTNode ASTNode;

// AI-friendly data structures for semantic information
typedef struct {
    char *name;
    char *type_name;
    char *kind;              // "variable", "function", "struct", "enum"
    bool is_public;
    bool is_mutable;
    char *documentation;
    SourceLocation location;
} AISymbolInfo;

typedef struct {
    char *name;
    char *type_name;
    bool is_public;
    bool is_mutable;
    char *default_value;
} AIFieldInfo;

typedef struct {
    char *struct_name;
    AIFieldInfo *fields;
    size_t field_count;
    char **method_names;
    size_t method_count;
    bool is_generic;
    char **type_parameters;
    size_t type_param_count;
} AIStructInfo;

// Forward declaration - full definition in type_queries.h
typedef struct AITypeInfo AITypeInfo;

typedef struct {
    char *file_path;
    size_t line;
    size_t column;
    char *context_code;      // Surrounding code for context
} AICodeLocation;

// Main API structure with performance caching
typedef struct {
    SemanticAnalyzer *analyzer;
    SymbolTable *cached_symbols;
    SemanticCache *performance_cache;  // Week 7: Performance optimization cache
    char *source_file_path;
    bool is_initialized;
    pthread_mutex_t api_mutex;
    
    // Performance monitoring (Week 7)
    size_t total_queries;
    double total_query_time_ms;
    size_t cache_hits;
    size_t cache_misses;
} AsthraSemanticsAPI;

// Core API lifecycle functions
AsthraSemanticsAPI *asthra_ai_create_api(const char *source_file);
bool asthra_ai_initialize_from_source(AsthraSemanticsAPI *api, const char *source_code);
void asthra_ai_destroy_api(AsthraSemanticsAPI *api);

// Symbol information queries
AISymbolInfo *asthra_ai_get_symbol_info(AsthraSemanticsAPI *api, const char *symbol_name);
AIStructInfo *asthra_ai_get_struct_info(AsthraSemanticsAPI *api, const char *struct_name);
char **asthra_ai_get_available_methods(AsthraSemanticsAPI *api, const char *type_name, size_t *count);
AIFieldInfo **asthra_ai_get_struct_fields(AsthraSemanticsAPI *api, const char *struct_name, size_t *count);
char **asthra_ai_get_enum_variants(AsthraSemanticsAPI *api, const char *enum_name, size_t *count);

// Type system queries
char *asthra_ai_infer_expression_type(AsthraSemanticsAPI *api, const char *expression);
bool asthra_ai_check_type_compatibility(AsthraSemanticsAPI *api, const char *expected, const char *actual);
char **asthra_ai_get_compatible_types(AsthraSemanticsAPI *api, const char *context, size_t *count);
AITypeInfo *asthra_ai_get_type_info(AsthraSemanticsAPI *api, const char *type_name);

// AST navigation and search
ASTNode *asthra_ai_find_declaration(AsthraSemanticsAPI *api, const char *symbol_name);
// Moved to ast_navigation.h as asthra_ai_find_symbol_usages
char *asthra_ai_get_symbol_documentation(AsthraSemanticsAPI *api, const char *symbol_name);
AICodeLocation **asthra_ai_get_symbol_locations(AsthraSemanticsAPI *api, const char *symbol_name, size_t *count);

// Memory management for AI data structures
void asthra_ai_free_symbol_info(AISymbolInfo *info);
void asthra_ai_free_struct_info(AIStructInfo *info);
void asthra_ai_free_field_info_array(AIFieldInfo **fields, size_t count);
void asthra_ai_free_string_array(char **strings, size_t count);
void asthra_ai_free_type_info(AITypeInfo *info);
void asthra_ai_free_code_location_array(AICodeLocation **locations, size_t count);

// Performance monitoring and optimization (Week 7)
typedef struct {
    double average_query_time_ms;
    double cache_hit_rate;
    size_t total_queries;
    size_t memory_usage_bytes;
    size_t cache_entries;
} AIPerformanceStats;

AIPerformanceStats asthra_ai_get_performance_stats(AsthraSemanticsAPI *api);
void asthra_ai_reset_performance_stats(AsthraSemanticsAPI *api);
bool asthra_ai_configure_cache(AsthraSemanticsAPI *api, const CacheConfig *config);
void asthra_ai_clear_cache(AsthraSemanticsAPI *api);

#endif // ASTHRA_AI_SEMANTIC_API_H
