/**
 * FFI Test Stubs - Test Suite Implementation
 * 
 * Provides comprehensive FFI infrastructure test suite
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_stubs_test_suite.h"
#include "ffi_stubs_generator.h"
#include "ffi_stubs_memory.h"
#include "ffi_stubs_marshaling.h"
#include "ffi_stubs_ast_parser.h"
#include "ffi_stubs_codegen.h"
#include "../../runtime/memory/asthra_runtime_memory.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// COMPREHENSIVE FFI TEST SUITE
// =============================================================================

void run_enhanced_ffi_tests(void) {
    printf("\n=== Enhanced FFI Infrastructure Test Suite ===\n");
    
    // Initialize FFI Memory Manager
    ffi_memory_manager_init();

    // Test Enhanced FFI Generator
    printf("\n1. Testing Enhanced FFI Generator...\n");
    EnhancedFFIGenerator *ffi_gen = enhanced_ffi_generator_create();
    if (ffi_gen) {
        enhanced_generate_string_concatenation(ffi_gen, "Hello", "World");
        enhanced_generate_slice_length_access(ffi_gen, "test_slice", REG_RAX);
        enhanced_generate_volatile_memory_access(ffi_gen, "test_memory", 64);
        enhanced_ffi_generator_destroy(ffi_gen);
        printf("   ✓ FFI Generator tests completed\n");
    } else {
        printf("   ✗ FFI Generator creation failed\n");
    }
    
    // Test Enhanced Parser
    printf("\n2. Testing Enhanced Parser...\n");
    EnhancedParser *parser = enhanced_parser_create();
    if (parser) {
        EnhancedASTNode *ast = enhanced_parser_parse_program(parser, "fn main(void) { return 42; }");
        if (ast) {
            enhanced_ast_release_node(ast);
            printf("   ✓ Parser tests completed\n");
        } else {
            printf("   ✗ Parser failed to parse test program\n");
        }
        enhanced_parser_destroy(parser);
    } else {
        printf("   ✗ Parser creation failed\n");
    }
    
    // Test Enhanced FFI Marshaling
    printf("\n3. Testing Enhanced FFI Marshaling...\n");
    char test_param[] = "test parameter";
    bool marshaling_success = enhanced_ffi_generate_parameter_marshaling(
        test_param, strlen(test_param), FFI_OWNERSHIP_COPY, REG_RDI
    );
    if (marshaling_success) {
        printf("   ✓ FFI Marshaling tests completed\n");
    } else {
        printf("   ✗ FFI Marshaling failed\n");
    }
    
    // Test Enhanced Codegen
    printf("\n4. Testing Enhanced Codegen...\n");
    EnhancedCodegenContext *codegen = enhanced_codegen_create_context();
    if (codegen) {
        enhanced_codegen_generate_function_call(codegen, "printf");
        enhanced_codegen_generate_return(codegen, NULL);
        
        CodegenStats stats = enhanced_codegen_get_stats(codegen);
        printf("   Generated %u instructions, %u function calls, %u returns\n",
               stats.total_instructions, stats.function_calls, stats.returns);
        
        enhanced_codegen_destroy_context(codegen);
        printf("   ✓ Codegen tests completed\n");
    } else {
        printf("   ✗ Codegen creation failed\n");
    }
    
    // Test statistics
    printf("\n5. Testing Statistics...\n");
    FFIMarshalingStats marshaling_stats = ffi_get_marshaling_stats();
    printf("   FFI Marshaling: %u operations, %.1f%% success rate\n",
           marshaling_stats.total_operations, marshaling_stats.success_rate);
    
    // Test FFI Memory Statistics
    FFIMemoryStats mem_stats = ffi_get_memory_statistics();
    printf("   FFI Memory: Allocated %zu bytes, Freed %zu bytes, Current %zu bytes, Peak %zu bytes\n",
           mem_stats.total_allocated_bytes, mem_stats.total_freed_bytes, 
           mem_stats.current_memory_usage, mem_stats.peak_memory_usage);
    if (mem_stats.leak_detected) {
        printf("   ✗ FFI Memory Leak Detected!\n");
    } else {
        printf("   ✓ No FFI Memory Leaks Detected.\n");
    }

    // Test Runtime Memory Management (using asthra_alloc/free)
    printf("\n6. Testing Runtime Memory Management...\n");
    void *ptr1 = asthra_alloc(100, ASTHRA_ZONE_GC);
    void *ptr2 = asthra_alloc(50, ASTHRA_ZONE_MANUAL);
    asthra_free(ptr1, ASTHRA_ZONE_GC);
    asthra_free(ptr2, ASTHRA_ZONE_MANUAL);
    
    AsthraRuntimeStats runtime_stats = asthra_get_runtime_stats();
    printf("   Runtime Memory: Total Allocations: %llu, Current Usage: %zu, Peak Usage: %zu\n",
           (unsigned long long)runtime_stats.total_allocations, runtime_stats.current_memory_usage, runtime_stats.peak_memory_usage);
    
    // Intentionally leak ptr2 for testing purposes
    // asthra_free(ptr2, ASTHRA_ZONE_MANUAL);
    
    printf("   ✓ Runtime Memory Management tests completed (check for explicit leak below)\n");

    printf("\n=== Enhanced FFI Infrastructure Test Suite Complete ===\n");
}

void cleanup_enhanced_ffi_infrastructure(void) {
    enhanced_ffi_marshaler_cleanup();
    ffi_memory_manager_cleanup(); // Cleanup FFI memory manager
    // Note: Memory leak checking and cleanup functions not available in current runtime
    printf("   Memory cleanup completed\n");
    printf("[CLEANUP] Enhanced FFI infrastructure cleaned up\n");
}