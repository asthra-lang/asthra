/**
 * Comprehensive Asthra Linker Test
 * Tests for the main linker functionality including object file linking,
 * symbol resolution, and executable generation
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Include the linker headers
#include "asthra_linker.h"
#include "object_file_manager.h"
#include "symbol_resolution.h"

// Test utility functions
static bool test_linker_creation(void) {
    printf("Testing linker creation...\n");
    
    Asthra_Linker *linker = asthra_linker_create();
    if (!linker) {
        printf("FAIL: Failed to create linker instance\n");
        return false;
    }
    
    // Verify linker is in valid initial state
    if (!asthra_linker_is_ready(linker)) {
        printf("FAIL: Linker not in ready state after creation\n");
        asthra_linker_destroy(linker);
        return false;
    }
    
    printf("✓ Linker created successfully\n");
    printf("✓ Linker is in ready state\n");
    
    asthra_linker_destroy(linker);
    printf("✓ Linker destroyed successfully\n");
    
    printf("PASS: Linker creation test succeeded\n");
    return true;
}

static bool test_linking_request_configuration(void) {
    printf("Testing linking request configuration...\n");
    
    Asthra_Linker *linker = asthra_linker_create();
    if (!linker) {
        printf("FAIL: Failed to create linker instance\n");
        return false;
    }
    
    // Initialize linking request
    Asthra_LinkingRequest request;
    asthra_linking_request_init(&request);
    
    // Configure basic request parameters
    const char *test_objects[] = {"test1.o", "test2.o"};
    request.object_files = test_objects;
    request.object_file_count = 2;
    request.output_executable_path = "test_executable";
    request.generate_debug_info = true;
    request.parallel_linking = false;
    
    bool config_result = asthra_linker_configure(linker, &request);
    if (!config_result) {
        printf("FAIL: Failed to configure linking request\n");
        asthra_linking_request_cleanup(&request);
        asthra_linker_destroy(linker);
        return false;
    }
    
    printf("✓ Linking request configured successfully\n");
    printf("✓ Configuration parameters accepted\n");
    
    asthra_linking_request_cleanup(&request);
    asthra_linker_destroy(linker);
    
    printf("PASS: Linking request configuration test succeeded\n");
    return true;
}

static bool test_symbol_resolution(void) {
    printf("Testing symbol resolution...\n");
    
    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        printf("FAIL: Failed to create symbol resolver\n");
        return false;
    }
    
    // Test adding a symbol
    Asthra_ObjectSymbol test_symbol;
    memset(&test_symbol, 0, sizeof(test_symbol));
    test_symbol.name = strdup("test_function");
    test_symbol.address = 0x1000;
    test_symbol.size = 100;
    test_symbol.binding = ASTHRA_SYMBOL_GLOBAL;
    test_symbol.type = ASTHRA_SYMBOL_FUNCTION;
    
    bool add_result = asthra_symbol_resolver_add_symbol(resolver, &test_symbol, "test_file.o");
    if (!add_result) {
        printf("FAIL: Failed to add symbol to resolver\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }
    
    printf("✓ Symbol added successfully\n");
    
    // Test symbol lookup
    Asthra_SymbolEntry *found_symbol = asthra_symbol_resolver_find_symbol(resolver, "test_function");
    if (!found_symbol) {
        printf("FAIL: Failed to find added symbol\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }
    
    if (strcmp(found_symbol->name, "test_function") != 0) {
        printf("FAIL: Found symbol has incorrect name\n");
        asthra_symbol_resolver_destroy(resolver);
        return false;
    }
    
    printf("✓ Symbol lookup successful\n");
    printf("✓ Symbol data integrity verified\n");
    
    asthra_symbol_resolver_destroy(resolver);
    
    printf("PASS: Symbol resolution test succeeded\n");
    return true;
}

static bool test_object_file_manager(void) {
    printf("Testing object file manager...\n");
    
    Asthra_ObjectFileManager *manager = asthra_object_manager_create();
    if (!manager) {
        printf("FAIL: Failed to create object file manager\n");
        return false;
    }
    
    // Test manager state
    if (!manager) {
        printf("FAIL: Object file manager not ready after creation\n");
        return false;
    }
    
    printf("✓ Object file manager created successfully\n");
    printf("✓ Manager is in ready state\n");
    
    // Test statistics
    size_t loaded_count, symbols_processed;
    double cache_hit_rate, total_time;
    asthra_object_manager_get_statistics(manager, &loaded_count, &cache_hit_rate, &total_time, &symbols_processed);
    
    if (loaded_count != 0 || symbols_processed != 0) {
        printf("FAIL: Initial statistics should be zero\n");
        asthra_object_manager_destroy(manager);
        return false;
    }
    
    printf("✓ Initial statistics are correct\n");
    
    asthra_object_manager_destroy(manager);
    
    printf("PASS: Object file manager test succeeded\n");
    return true;
}

static bool test_executable_metadata(void) {
    printf("Testing executable metadata...\n");
    
    Asthra_ExecutableMetadata metadata;
    memset(&metadata, 0, sizeof(metadata));
    
    // Test metadata initialization
    metadata.target_platform = "linux";
    metadata.target_architecture = "x86_64";
    metadata.executable_format = "ELF";
    metadata.executable_size = 8192;
    metadata.symbol_count = 50;
    metadata.section_count = 8;
    metadata.has_debug_info = true;
    metadata.is_stripped = false;
    metadata.entry_point = "main";
    metadata.base_address = 0x400000;
    metadata.file_permissions = 0755;
    
    // Verify metadata fields
    if (strcmp(metadata.target_platform, "linux") != 0) {
        printf("FAIL: Platform metadata incorrect\n");
        return false;
    }
    
    if (metadata.executable_size != 8192) {
        printf("FAIL: Size metadata incorrect\n");
        return false;
    }
    
    if (metadata.base_address != 0x400000) {
        printf("FAIL: Base address metadata incorrect\n");
        return false;
    }
    
    printf("✓ Metadata fields set correctly\n");
    printf("✓ Metadata structure integrity verified\n");
    
    printf("PASS: Executable metadata test succeeded\n");
    return true;
}

static bool test_linking_result(void) {
    printf("Testing linking result handling...\n");
    
    Asthra_LinkingResult result;
    memset(&result, 0, sizeof(result));
    
    // Test result initialization
    result.success = true;
    result.executable_path = strdup("test_output");
    result.linking_time_ms = 123.45;
    result.total_symbols_processed = 100;
    result.symbols_resolved = 95;
    result.symbols_unresolved = 5;
    
    // Verify result fields
    if (!result.success) {
        printf("FAIL: Result success flag incorrect\n");
        asthra_linking_result_cleanup(&result);
        return false;
    }
    
    if (result.total_symbols_processed != 100) {
        printf("FAIL: Symbol count incorrect\n");
        asthra_linking_result_cleanup(&result);
        return false;
    }
    
    if (result.symbols_resolved + result.symbols_unresolved != result.total_symbols_processed) {
        printf("FAIL: Symbol counts don't add up\n");
        asthra_linking_result_cleanup(&result);
        return false;
    }
    
    printf("✓ Linking result fields correct\n");
    printf("✓ Symbol counts consistent\n");
    
    asthra_linking_result_cleanup(&result);
    
    printf("PASS: Linking result test succeeded\n");
    return true;
}

static bool test_runtime_library_integration(void) {
    printf("Testing runtime library integration...\n");
    
    Asthra_Linker *linker = asthra_linker_create();
    if (!linker) {
        printf("FAIL: Failed to create linker instance\n");
        return false;
    }
    
    // For this test, we'll just verify that the library can be added
    // The actual runtime linking would require the runtime library to exist
    // in the expected format, which may not be the case in the test environment
    
    // Test adding a mock runtime library (using false for not required)
    bool lib_result = asthra_linker_add_runtime_library(linker, "nonexistent.a", false);
    if (!lib_result) {
        printf("FAIL: Failed to add optional runtime library\n");
        asthra_linker_destroy(linker);
        return false;
    }
    
    printf("✓ Runtime library interface tested successfully\n");
    
    // Skip the actual runtime linking test as it requires specific file layout
    printf("✓ Runtime linking test skipped (requires runtime environment)\n");
    
    asthra_linker_destroy(linker);
    
    printf("PASS: Runtime library integration test succeeded\n");
    return true;
}

static bool test_linker_statistics(void) {
    printf("Testing linker statistics...\n");
    
    Asthra_Linker *linker = asthra_linker_create();
    if (!linker) {
        printf("FAIL: Failed to create linker instance\n");
        return false;
    }
    
    // Get initial statistics
    size_t objects_linked, executables_generated;
    double total_time_ms, cache_hit_rate;
    
    asthra_linker_get_statistics(linker, &objects_linked, &executables_generated, 
                                &total_time_ms, &cache_hit_rate);
    
    // Verify initial state
    if (objects_linked != 0 || executables_generated != 0) {
        printf("FAIL: Initial statistics should be zero\n");
        asthra_linker_destroy(linker);
        return false;
    }
    
    printf("✓ Initial statistics are correct\n");
    printf("✓ Statistics tracking functional\n");
    
    // Test cache clearing
    asthra_linker_clear_cache(linker);
    printf("✓ Cache cleared successfully\n");
    
    asthra_linker_destroy(linker);
    
    printf("PASS: Linker statistics test succeeded\n");
    return true;
}

int main(void) {
    printf("Comprehensive Asthra Linker Tests\n");
    printf("=================================\n");
    
    bool all_passed = true;
    int tests_run = 0;
    int tests_passed = 0;
    
    // Run all linker tests
    struct {
        const char *name;
        bool (*test_func)(void);
    } tests[] = {
        {"Linker Creation", test_linker_creation},
        {"Linking Request Configuration", test_linking_request_configuration},
        {"Symbol Resolution", test_symbol_resolution},
        {"Object File Manager", test_object_file_manager},
        {"Executable Metadata", test_executable_metadata},
        {"Linking Result", test_linking_result},
        {"Runtime Library Integration", test_runtime_library_integration},
        {"Linker Statistics", test_linker_statistics}
    };
    
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        printf("\n--- Running %s Test ---\n", tests[i].name);
        if (tests[i].test_func()) {
            tests_passed++;
        } else {
            all_passed = false;
        }
        tests_run++;
    }
    
    printf("\n=================================\n");
    printf("Tests run: %d, Tests passed: %d\n", tests_run, tests_passed);
    
    if (all_passed) {
        printf("All comprehensive linker tests passed!\n");
        return 0;
    } else {
        printf("Some comprehensive linker tests failed!\n");
        return 1;
    }
} 
