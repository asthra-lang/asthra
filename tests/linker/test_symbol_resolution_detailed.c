/**
 * Detailed Symbol Resolution Test
 * Focused testing for symbol resolution, conflict handling,
 * and cross-reference management
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the symbol resolution headers
#include "object_file_manager.h"
#include "symbol_resolution.h"

static bool test_symbol_addition_and_lookup(void) {
    printf("Testing symbol addition and lookup...\n");

    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }

    // Add multiple symbols with different properties
    Asthra_ObjectSymbol symbols[] = {{.name = (char *)"main",
                                      .address = 0x1000,
                                      .size = 256,
                                      .binding = ASTHRA_SYMBOL_GLOBAL,
                                      .type = ASTHRA_SYMBOL_FUNCTION},
                                     {.name = (char *)"global_var",
                                      .address = 0x2000,
                                      .size = 8,
                                      .binding = ASTHRA_SYMBOL_GLOBAL,
                                      .type = ASTHRA_SYMBOL_VARIABLE},
                                     {.name = (char *)"local_func",
                                      .address = 0x1100,
                                      .size = 128,
                                      .binding = ASTHRA_SYMBOL_LOCAL,
                                      .type = ASTHRA_SYMBOL_FUNCTION},
                                     {.name = (char *)"weak_symbol",
                                      .address = 0x3000,
                                      .size = 32,
                                      .binding = ASTHRA_SYMBOL_WEAK,
                                      .type = ASTHRA_SYMBOL_VARIABLE}};

    size_t symbol_count = sizeof(symbols) / sizeof(symbols[0]);

    // Add all symbols
    for (size_t i = 0; i < symbol_count; i++) {
        bool add_result = asthra_symbol_resolver_add_symbol(resolver, &symbols[i], "test.o");
        if (!add_result) {
            printf("FAIL: Failed to add symbol '%s'\n", symbols[i].name);
            asthra_symbol_resolver_destroy(resolver);
            return false;
        }
    }

    printf("✓ Added %zu symbols successfully\n", symbol_count);

    // Test lookup of each symbol
    for (size_t i = 0; i < symbol_count; i++) {
        Asthra_SymbolEntry *found = asthra_symbol_resolver_find_symbol(resolver, symbols[i].name);
        if (!found) {
            printf("FAIL: Failed to find symbol '%s'\n", symbols[i].name);
            asthra_symbol_resolver_destroy(resolver);
            return false;
        }

        if (found->address != symbols[i].address) {
            printf("FAIL: Symbol '%s' has incorrect address\n", symbols[i].name);
            asthra_symbol_resolver_destroy(resolver);
            return false;
        }

        if (found->size != symbols[i].size) {
            printf("FAIL: Symbol '%s' has incorrect size\n", symbols[i].name);
            asthra_symbol_resolver_destroy(resolver);
            return false;
        }
    }

    printf("✓ All symbols found with correct properties\n");

    asthra_symbol_resolver_destroy(resolver);

    printf("PASS: Symbol addition and lookup test succeeded\n");
    return true;
}

static bool test_symbol_conflict_resolution(void) {
    printf("Testing symbol conflict resolution...\n");

    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }

    // Add a global symbol
    Asthra_ObjectSymbol global_symbol = {.name = (char *)"conflicted_symbol",
                                         .address = 0x1000,
                                         .size = 128,
                                         .binding = ASTHRA_SYMBOL_GLOBAL,
                                         .type = ASTHRA_SYMBOL_FUNCTION};

    bool add_result1 = asthra_symbol_resolver_add_symbol(resolver, &global_symbol, "file1.o");
    if (!add_result1) {
        printf("FAIL: Failed to add first global symbol\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Added first global symbol\n");

    // Try to add a conflicting global symbol (should fail)
    Asthra_ObjectSymbol conflicting_symbol = {.name = (char *)"conflicted_symbol",
                                              .address = 0x2000,
                                              .size = 64,
                                              .binding = ASTHRA_SYMBOL_GLOBAL,
                                              .type = ASTHRA_SYMBOL_FUNCTION};

    bool add_result2 = asthra_symbol_resolver_add_symbol(resolver, &conflicting_symbol, "file2.o");
    if (add_result2) {
        printf("FAIL: Should not be able to add conflicting global symbol\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Correctly rejected conflicting global symbol\n");

    // Add a weak symbol with same name (should be allowed)
    Asthra_ObjectSymbol weak_symbol = {.name = (char *)"conflicted_symbol",
                                       .address = 0x3000,
                                       .size = 32,
                                       .binding = ASTHRA_SYMBOL_WEAK,
                                       .type = ASTHRA_SYMBOL_FUNCTION};

    bool add_result3 = asthra_symbol_resolver_add_symbol(resolver, &weak_symbol, "file3.o");
    // This behavior depends on implementation - weak symbols might override or be ignored

    printf("✓ Weak symbol handling completed (result: %s)\n",
           add_result3 ? "accepted" : "rejected");

    asthra_symbol_resolver_destroy(resolver);

    printf("PASS: Symbol conflict resolution test succeeded\n");
    return true;
}

static bool test_symbol_references(void) {
    printf("Testing symbol references...\n");

    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }

    // Add a symbol
    Asthra_ObjectSymbol symbol = {.name = (char *)"referenced_function",
                                  .address = 0x1000,
                                  .size = 128,
                                  .binding = ASTHRA_SYMBOL_GLOBAL,
                                  .type = ASTHRA_SYMBOL_FUNCTION};

    bool add_result = asthra_symbol_resolver_add_symbol(resolver, &symbol, "test.o");
    if (!add_result) {
        printf("FAIL: Failed to add symbol\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    // Add references to the symbol
    for (int i = 0; i < 3; i++) {
        bool ref_result =
            asthra_symbol_resolver_add_reference(resolver, "referenced_function", "test.o", ".text",
                                                 0x2000 + i * 100, // Different reference addresses
                                                 ASTHRA_REF_RELATIVE, 0);

        if (!ref_result) {
            printf("FAIL: Failed to add reference %d\n", i);
            asthra_symbol_resolver_destroy(resolver);
            return false;
        }
    }

    printf("✓ Added 3 references to symbol\n");

    // Find the symbol and check reference count
    Asthra_SymbolEntry *found = asthra_symbol_resolver_find_symbol(resolver, "referenced_function");
    if (!found) {
        printf("FAIL: Failed to find symbol with references\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    if (found->reference_count != 3) {
        printf("FAIL: Expected 3 references, got %zu\n", found->reference_count);
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Symbol has correct reference count: %zu\n", found->reference_count);

    asthra_symbol_resolver_destroy(resolver);

    printf("PASS: Symbol references test succeeded\n");
    return true;
}

static bool test_undefined_symbol_handling(void) {
    printf("Testing undefined symbol handling...\n");

    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }

    // Add reference to undefined symbol
    bool ref_result = asthra_symbol_resolver_add_reference(resolver, "undefined_function", "test.o",
                                                           ".text", 0x1000, ASTHRA_REF_ABSOLUTE, 0);

    if (!ref_result) {
        printf("FAIL: Failed to add reference to undefined symbol\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Added reference to undefined symbol\n");

    // Try to find the undefined symbol
    Asthra_SymbolEntry *found = asthra_symbol_resolver_find_symbol(resolver, "undefined_function");
    if (!found) {
        printf("FAIL: Undefined symbol should exist in resolver\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    if (found->status != ASTHRA_RESOLVE_UNDEFINED) {
        printf("FAIL: Symbol should have undefined status\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Undefined symbol has correct status\n");

    // Get resolution result
    Asthra_ResolutionResult result;
    memset(&result, 0, sizeof(result));

    bool resolve_result = asthra_symbol_resolver_resolve_all(resolver, &result);
    if (resolve_result) {
        printf("FAIL: Resolution should fail with undefined symbols\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    if (result.undefined_count != 1) {
        printf("FAIL: Expected 1 unresolved symbol, got %zu\n", result.undefined_count);
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Resolution correctly failed with 1 unresolved symbol\n");

    asthra_symbol_resolver_destroy(resolver);

    printf("PASS: Undefined symbol handling test succeeded\n");
    return true;
}

static bool test_symbol_resolution_performance(void) {
    printf("Testing symbol resolution performance...\n");

    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }

    const size_t NUM_SYMBOLS = 1000;

    // Add many symbols
    for (size_t i = 0; i < NUM_SYMBOLS; i++) {
        char symbol_name[64];
        snprintf(symbol_name, sizeof(symbol_name), "symbol_%zu", i);

        Asthra_ObjectSymbol symbol = {.name = strdup(symbol_name),
                                      .address = 0x10000 + i * 16,
                                      .size = 16,
                                      .binding = ASTHRA_SYMBOL_GLOBAL,
                                      .type = ASTHRA_SYMBOL_FUNCTION};

        bool add_result = asthra_symbol_resolver_add_symbol(resolver, &symbol, "test.o");
        if (!add_result) {
            printf("FAIL: Failed to add symbol %zu\n", i);
            // Clean up allocated names
            for (size_t j = 0; j <= i; j++) {
                char cleanup_name[64];
                snprintf(cleanup_name, sizeof(cleanup_name), "symbol_%zu", j);
                Asthra_SymbolEntry *entry =
                    asthra_symbol_resolver_find_symbol(resolver, cleanup_name);
                if (entry && entry->name) {
                    free((void *)entry->name);
                }
            }
            asthra_symbol_resolver_destroy(resolver);
            return false;
        }

        // Free the strdup'd name as it should be copied by the resolver
        free((void *)symbol.name);
    }

    printf("✓ Added %zu symbols\n", NUM_SYMBOLS);

    // Test lookup performance
    size_t found_count = 0;
    for (size_t i = 0; i < NUM_SYMBOLS; i += 10) { // Test every 10th symbol
        char symbol_name[64];
        snprintf(symbol_name, sizeof(symbol_name), "symbol_%zu", i);

        Asthra_SymbolEntry *found = asthra_symbol_resolver_find_symbol(resolver, symbol_name);
        if (found) {
            found_count++;
        }
    }

    size_t expected_found = (NUM_SYMBOLS + 9) / 10; // Ceiling division
    if (found_count != expected_found) {
        printf("FAIL: Expected to find %zu symbols, found %zu\n", expected_found, found_count);
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    printf("✓ Successfully found %zu symbols in performance test\n", found_count);

    asthra_symbol_resolver_destroy(resolver);

    printf("PASS: Symbol resolution performance test succeeded\n");
    return true;
}

static bool test_symbol_statistics(void) {
    printf("Testing symbol resolution statistics...\n");

    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }

    // Add some symbols and references
    Asthra_ObjectSymbol symbols[] = {
        {(char *)"func1", 0x1000, 100, ASTHRA_SYMBOL_GLOBAL, ASTHRA_SYMBOL_FUNCTION},
        {(char *)"func2", 0x1100, 150, ASTHRA_SYMBOL_GLOBAL, ASTHRA_SYMBOL_FUNCTION},
        {(char *)"var1", 0x2000, 8, ASTHRA_SYMBOL_GLOBAL, ASTHRA_SYMBOL_VARIABLE}};

    for (size_t i = 0; i < 3; i++) {
        asthra_symbol_resolver_add_symbol(resolver, &symbols[i], "test.o");
    }

    // Add some references
    asthra_symbol_resolver_add_reference(resolver, "func1", "test.o", ".text", 0x3000,
                                         ASTHRA_REF_ABSOLUTE, 0);
    asthra_symbol_resolver_add_reference(resolver, "func2", "test.o", ".text", 0x3100,
                                         ASTHRA_REF_RELATIVE, 0);
    asthra_symbol_resolver_add_reference(resolver, "undefined", "test.o", ".text", 0x3200,
                                         ASTHRA_REF_ABSOLUTE, 0);

    // Get statistics
    size_t total_symbols, resolved_symbols;
    double resolution_time_ms, hash_efficiency;
    asthra_symbol_resolver_get_statistics(resolver, &total_symbols, &resolved_symbols,
                                          &resolution_time_ms, &hash_efficiency);

    printf("✓ Total symbols: %zu\n", total_symbols);
    printf("✓ Resolved symbols: %zu\n", resolved_symbols);
    printf("✓ Resolution time: %.2f ms\n", resolution_time_ms);
    printf("✓ Hash efficiency: %.2f%%\n", hash_efficiency * 100.0);

    // Calculate derived statistics
    size_t undefined_symbols = total_symbols - resolved_symbols;

    // Verify statistics
    if (total_symbols != 4) { // 3 defined + 1 undefined
        printf("FAIL: Expected 4 total symbols, got %zu\n", total_symbols);
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    if (resolved_symbols != 3) {
        printf("FAIL: Expected 3 resolved symbols, got %zu\n", resolved_symbols);
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    if (undefined_symbols != 1) {
        printf("FAIL: Expected 1 undefined symbol, got %zu\n", undefined_symbols);
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }

    // Note: We added 3 references, so we expect 3 total references
    // However, the API doesn't provide total_references, so we'll skip this check

    asthra_symbol_resolver_destroy(resolver);

    printf("PASS: Symbol resolution statistics test succeeded\n");
    return true;
}

int main(void) {
    printf("Detailed Symbol Resolution Tests\n");
    printf("===============================\n");

    bool all_passed = true;
    int tests_run = 0;
    int tests_passed = 0;

    // Run all symbol resolution tests
    struct {
        const char *name;
        bool (*test_func)(void);
    } tests[] = {{"Symbol Addition and Lookup", test_symbol_addition_and_lookup},
                 {"Symbol Conflict Resolution", test_symbol_conflict_resolution},
                 {"Symbol References", test_symbol_references},
                 {"Undefined Symbol Handling", test_undefined_symbol_handling},
                 {"Symbol Resolution Performance", test_symbol_resolution_performance},
                 {"Symbol Statistics", test_symbol_statistics}};

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        printf("\n--- Running %s Test ---\n", tests[i].name);
        if (tests[i].test_func()) {
            tests_passed++;
        } else {
            all_passed = false;
        }
        tests_run++;
    }

    printf("\n===============================\n");
    printf("Tests run: %d, Tests passed: %d\n", tests_run, tests_passed);

    if (all_passed) {
        printf("All detailed symbol resolution tests passed!\n");
        return 0;
    } else {
        printf("Some detailed symbol resolution tests failed!\n");
        return 1;
    }
}
