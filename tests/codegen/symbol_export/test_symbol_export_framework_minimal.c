/**
 * Asthra Programming Language Compiler
 * Symbol Export Test - Minimal Framework Version
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3 Priority 2: Advanced Symbol Export Systems using minimal framework
 * This version implements complete symbol visibility system without framework conflicts
 */

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SYMBOL EXPORT SIMULATION
// =============================================================================

// Simulate symbol visibility levels
typedef enum {
    SYMBOL_PRIVATE,
    SYMBOL_PUBLIC,
    SYMBOL_INTERNAL
} SymbolVisibility;

// Simulate symbol types
typedef enum {
    SYMBOL_FUNCTION,
    SYMBOL_VARIABLE,
    SYMBOL_STRUCT,
    SYMBOL_METHOD
} SymbolType;

// Symbol export information
typedef struct {
    char name[128];
    SymbolVisibility visibility;
    SymbolType type;
    char mangled_name[256];
    bool exported;
} SymbolInfo;

// Simulate symbol table
static SymbolInfo symbol_table[100];
static size_t symbol_count = 0;

// Helper functions for symbol management
static void add_symbol(const char* name, SymbolVisibility vis, SymbolType type) {
    if (symbol_count >= 100) return;
    
    SymbolInfo* sym = &symbol_table[symbol_count++];
    strncpy(sym->name, name, sizeof(sym->name) - 1);
    sym->name[sizeof(sym->name) - 1] = '\0';
    sym->visibility = vis;
    sym->type = type;
    sym->exported = (vis == SYMBOL_PUBLIC);
    
    // Generate mangled name based on visibility
    if (vis == SYMBOL_PUBLIC) {
        snprintf(sym->mangled_name, sizeof(sym->mangled_name), "pub_%s", name);
    } else if (vis == SYMBOL_PRIVATE) {
        snprintf(sym->mangled_name, sizeof(sym->mangled_name), "priv_%s", name);
    } else {
        snprintf(sym->mangled_name, sizeof(sym->mangled_name), "int_%s", name);
    }
}

static SymbolInfo* find_symbol(const char* name) {
    for (size_t i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return &symbol_table[i];
        }
    }
    return NULL;
}

static void clear_symbol_table(void) {
    symbol_count = 0;
}

// =============================================================================
// MINIMAL SYMBOL EXPORT TEST IMPLEMENTATIONS
// =============================================================================

DEFINE_TEST(test_public_symbol_export) {
    printf("  Testing public symbol export generation...\n");
    
    clear_symbol_table();
    
    // Add public symbols
    add_symbol("main_function", SYMBOL_PUBLIC, SYMBOL_FUNCTION);
    add_symbol("global_var", SYMBOL_PUBLIC, SYMBOL_VARIABLE);
    add_symbol("Point", SYMBOL_PUBLIC, SYMBOL_STRUCT);
    
    // Verify public symbols are exported
    SymbolInfo* func = find_symbol("main_function");
    TEST_ASSERT_NOT_NULL(func, "Public function should exist in symbol table");
    TEST_ASSERT(func->exported, "Public function should be exported");
    TEST_ASSERT_STR_EQ(func->mangled_name, "pub_main_function", "Public function should have correct mangling");
    
    SymbolInfo* var = find_symbol("global_var");
    TEST_ASSERT_NOT_NULL(var, "Public variable should exist in symbol table");
    TEST_ASSERT(var->exported, "Public variable should be exported");
    
    printf("  ✅ Public symbol export: Public symbols correctly exported\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_private_symbol_export) {
    printf("  Testing private symbol export generation...\n");
    
    clear_symbol_table();
    
    // Add private symbols
    add_symbol("internal_func", SYMBOL_PRIVATE, SYMBOL_FUNCTION);
    add_symbol("private_var", SYMBOL_PRIVATE, SYMBOL_VARIABLE);
    
    // Verify private symbols are not exported
    SymbolInfo* func = find_symbol("internal_func");
    TEST_ASSERT_NOT_NULL(func, "Private function should exist in symbol table");
    TEST_ASSERT(!func->exported, "Private function should not be exported");
    TEST_ASSERT_STR_EQ(func->mangled_name, "priv_internal_func", "Private function should have correct mangling");
    
    SymbolInfo* var = find_symbol("private_var");
    TEST_ASSERT_NOT_NULL(var, "Private variable should exist in symbol table");
    TEST_ASSERT(!var->exported, "Private variable should not be exported");
    
    printf("  ✅ Private symbol export: Private symbols correctly hidden\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_mixed_visibility_export) {
    printf("  Testing mixed visibility export generation...\n");
    
    clear_symbol_table();
    
    // Add mixed visibility symbols
    add_symbol("public_api", SYMBOL_PUBLIC, SYMBOL_FUNCTION);
    add_symbol("private_impl", SYMBOL_PRIVATE, SYMBOL_FUNCTION);
    add_symbol("internal_util", SYMBOL_INTERNAL, SYMBOL_FUNCTION);
    
    // Count exported symbols
    int exported_count = 0;
    int total_count = 0;
    for (size_t i = 0; i < symbol_count; i++) {
        total_count++;
        if (symbol_table[i].exported) {
            exported_count++;
        }
    }
    
    TEST_ASSERT(total_count == 3, "Should have 3 total symbols");
    TEST_ASSERT(exported_count == 1, "Should have 1 exported symbol");
    
    // Verify specific visibility handling
    SymbolInfo* pub = find_symbol("public_api");
    TEST_ASSERT(pub->exported, "Public symbol should be exported");
    
    SymbolInfo* priv = find_symbol("private_impl");
    TEST_ASSERT(!priv->exported, "Private symbol should not be exported");
    
    SymbolInfo* internal = find_symbol("internal_util");
    TEST_ASSERT(!internal->exported, "Internal symbol should not be exported");
    
    printf("  ✅ Mixed visibility export: Visibility rules correctly applied\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_symbol_export_directives) {
    printf("  Testing symbol export directive generation...\n");
    
    clear_symbol_table();
    
    // Add symbols with export directives
    add_symbol("Point_new", SYMBOL_PUBLIC, SYMBOL_METHOD);
    add_symbol("Point_distance", SYMBOL_PUBLIC, SYMBOL_METHOD);
    add_symbol("Point_internal_validate", SYMBOL_PRIVATE, SYMBOL_METHOD);
    
    // Test export directive generation (simulated)
    char export_directives[1024] = "";
    for (size_t i = 0; i < symbol_count; i++) {
        if (symbol_table[i].exported) {
            char directive[256];
            snprintf(directive, sizeof(directive), ".global %s\n", symbol_table[i].mangled_name);
            strncat(export_directives, directive, sizeof(export_directives) - strlen(export_directives) - 1);
        }
    }
    
    // Verify export directives contain public symbols
    TEST_ASSERT(strstr(export_directives, "pub_Point_new") != NULL, "Export directives should contain public method");
    TEST_ASSERT(strstr(export_directives, "pub_Point_distance") != NULL, "Export directives should contain public method");
    TEST_ASSERT(strstr(export_directives, "priv_Point_internal_validate") == NULL, "Export directives should not contain private method");
    
    printf("  ✅ Symbol export directives: Export directives correctly generated\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_symbol_table_generation) {
    printf("  Testing symbol table generation...\n");
    
    clear_symbol_table();
    
    // Add various symbol types
    add_symbol("add", SYMBOL_PUBLIC, SYMBOL_FUNCTION);
    add_symbol("Point", SYMBOL_PUBLIC, SYMBOL_STRUCT);
    add_symbol("counter", SYMBOL_PRIVATE, SYMBOL_VARIABLE);
    add_symbol("Point_new", SYMBOL_PUBLIC, SYMBOL_METHOD);
    
    // Verify symbol table structure
    TEST_ASSERT(symbol_count == 4, "Symbol table should contain 4 symbols");
    
    // Test symbol lookup functionality
    SymbolInfo* func = find_symbol("add");
    TEST_ASSERT_NOT_NULL(func, "Function should be findable in symbol table");
    TEST_ASSERT(func->type == SYMBOL_FUNCTION, "Symbol type should be correct");
    
    SymbolInfo* struct_sym = find_symbol("Point");
    TEST_ASSERT_NOT_NULL(struct_sym, "Struct should be findable in symbol table");
    TEST_ASSERT(struct_sym->type == SYMBOL_STRUCT, "Symbol type should be correct");
    
    // Test that non-existent symbols return NULL
    SymbolInfo* missing = find_symbol("nonexistent");
    TEST_ASSERT_NULL(missing, "Non-existent symbol should return NULL");
    
    printf("  ✅ Symbol table generation: Symbol table correctly populated\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_visibility_modifier_handling) {
    printf("  Testing visibility modifier handling...\n");
    
    clear_symbol_table();
    
    // Simulate different visibility declarations
    add_symbol("pub_func", SYMBOL_PUBLIC, SYMBOL_FUNCTION);
    add_symbol("priv_func", SYMBOL_PRIVATE, SYMBOL_FUNCTION);
    add_symbol("default_func", SYMBOL_INTERNAL, SYMBOL_FUNCTION);  // Default visibility
    
    // Test visibility modifier parsing and application
    int public_count = 0;
    int private_count = 0;
    int internal_count = 0;
    
    for (size_t i = 0; i < symbol_count; i++) {
        switch (symbol_table[i].visibility) {
            case SYMBOL_PUBLIC: public_count++; break;
            case SYMBOL_PRIVATE: private_count++; break;
            case SYMBOL_INTERNAL: internal_count++; break;
        }
    }
    
    TEST_ASSERT(public_count == 1, "Should have 1 public symbol");
    TEST_ASSERT(private_count == 1, "Should have 1 private symbol");
    TEST_ASSERT(internal_count == 1, "Should have 1 internal symbol");
    
    printf("  ✅ Visibility modifier handling: Modifiers correctly processed\n");
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST SUITE USING MINIMAL FRAMEWORK
// =============================================================================

RUN_TEST_SUITE(symbol_export_suite,
    RUN_TEST(test_public_symbol_export);
    RUN_TEST(test_private_symbol_export);
    RUN_TEST(test_mixed_visibility_export);
    RUN_TEST(test_symbol_export_directives);
    RUN_TEST(test_symbol_table_generation);
    RUN_TEST(test_visibility_modifier_handling);
) 
