/**
 * Minimal FFI Assembly Generator Test - Test Functions
 * 
 * This file contains all the individual test functions for the
 * minimal FFI assembly generator test suite.
 */

#include "test_ffi_minimal_common.h"

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

bool test_ffi_generator_creation(void) {
    printf("Testing FFI generator creation...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    TEST_ASSERT(generator->target_arch == TARGET_ARCH_X86_64, "Target architecture not set correctly");
    TEST_ASSERT(generator->calling_conv == CALLING_CONV_SYSTEM_V_AMD64, "Calling convention not set correctly");
    
    // Test configuration defaults
    TEST_ASSERT(generator->config.enable_bounds_checking == true, "Bounds checking should be enabled by default");
    TEST_ASSERT(generator->config.enable_security_features == true, "Security features should be enabled by default");
    TEST_ASSERT(generator->config.enable_concurrency == true, "Concurrency should be enabled by default");
    
    // Test runtime function names
    TEST_ASSERT(generator->runtime_functions.gc_alloc != NULL, "GC alloc function name not set");
    TEST_ASSERT(generator->runtime_functions.string_concat != NULL, "String concat function name not set");
    TEST_ASSERT(generator->runtime_functions.spawn_task != NULL, "Spawn task function name not set");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_ffi_generator_configuration(void) {
    printf("Testing FFI generator configuration...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Test configuration changes
    generator->config.enable_bounds_checking = false;
    generator->config.pic_mode = true;
    generator->config.max_variadic_args = 16;
    
    TEST_ASSERT(generator->config.enable_bounds_checking == false, "Bounds checking configuration failed");
    TEST_ASSERT(generator->config.pic_mode == true, "PIC mode configuration failed");
    TEST_ASSERT(generator->config.max_variadic_args == 16, "Variadic args configuration failed");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_string_operations(void) {
    printf("Testing string operations...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Test string concatenation
    bool result = minimal_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RAX);
    TEST_ASSERT(result == true, "String concatenation generation failed");
    
    // Check statistics
    size_t string_ops = 0;
    minimal_get_generation_statistics(generator, NULL, NULL, &string_ops, NULL, NULL, NULL);
    TEST_ASSERT(string_ops > 0, "String operation statistics not updated");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_slice_operations(void) {
    printf("Testing slice operations...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Test slice length access
    bool result = minimal_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RAX);
    TEST_ASSERT(result == true, "Slice length access generation failed");
    
    // Test slice bounds checking
    result = minimal_generate_slice_bounds_check(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, "bounds_error");
    TEST_ASSERT(result == true, "Slice bounds check generation failed");
    
    // Test slice to FFI conversion
    result = minimal_generate_slice_to_ffi(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RDX);
    TEST_ASSERT(result == true, "Slice to FFI conversion failed");
    
    // Check statistics
    size_t slice_ops = 0;
    minimal_get_generation_statistics(generator, NULL, NULL, NULL, &slice_ops, NULL, NULL);
    TEST_ASSERT(slice_ops > 0, "Slice operation statistics not updated");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_security_operations(void) {
    printf("Testing security operations...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Test volatile memory access
    bool result = minimal_generate_volatile_memory_access(generator, ASTHRA_REG_RDI, 8, true);
    TEST_ASSERT(result == true, "Volatile memory read generation failed");
    
    result = minimal_generate_volatile_memory_access(generator, ASTHRA_REG_RDI, 8, false);
    TEST_ASSERT(result == true, "Volatile memory write generation failed");
    
    // Test secure memory zeroing
    result = minimal_generate_secure_zero(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI);
    TEST_ASSERT(result == true, "Secure memory zeroing generation failed");
    
    // Check statistics
    size_t security_ops = 0;
    minimal_get_generation_statistics(generator, NULL, NULL, NULL, NULL, &security_ops, NULL);
    TEST_ASSERT(security_ops > 0, "Security operation statistics not updated");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_concurrency_operations(void) {
    printf("Testing concurrency operations...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Test task creation
    Register arg_regs[] = {ASTHRA_REG_RDI, ASTHRA_REG_RSI};
    bool result = minimal_generate_task_creation(generator, "test_function", 
                                                arg_regs, 2, ASTHRA_REG_RAX);
    TEST_ASSERT(result == true, "Task creation generation failed");
    
    // Check statistics
    size_t spawn_stmts = 0;
    minimal_get_generation_statistics(generator, NULL, NULL, NULL, NULL, NULL, &spawn_stmts);
    TEST_ASSERT(spawn_stmts > 0, "Spawn statement statistics not updated");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_assembly_validation(void) {
    printf("Testing assembly validation...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Generate some operations first
    minimal_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RAX);
    minimal_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RAX);
    
    // Validate generated assembly
    bool result = minimal_validate_generated_assembly(generator);
    TEST_ASSERT(result == true, "Generated assembly validation failed");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_nasm_output(void) {
    printf("Testing NASM assembly output...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Generate some operations
    minimal_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RAX);
    minimal_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RBX);
    
    // Test NASM output
    char output_buffer[4096];
    bool result = minimal_print_nasm_assembly(generator, output_buffer, sizeof(output_buffer));
    TEST_ASSERT(result == true, "NASM assembly output generation failed");
    
    // Basic validation of output
    TEST_ASSERT(strlen(output_buffer) > 0, "NASM output is empty");
    
    printf("Generated NASM assembly:\n%s\n", output_buffer);
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

bool test_generation_statistics(void) {
    printf("Testing generation statistics...\n");
    
    MinimalFFIAssemblyGenerator *generator = minimal_ffi_generator_create(
        TARGET_ARCH_X86_64, 
        CALLING_CONV_SYSTEM_V_AMD64
    );
    
    TEST_ASSERT(generator != NULL, "Failed to create FFI assembly generator");
    
    // Generate various operations
    minimal_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RAX);
    minimal_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RBX);
    minimal_generate_secure_zero(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI);
    
    Register arg_regs[] = {ASTHRA_REG_RDI, ASTHRA_REG_RSI};
    minimal_generate_task_creation(generator, "worker", arg_regs, 2, ASTHRA_REG_RAX);
    
    // Get statistics
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    minimal_get_generation_statistics(generator, &ffi_calls, &pattern_matches,
                                    &string_ops, &slice_ops, &security_ops, &spawn_stmts);
    
    printf("Generation Statistics:\n");
    printf("  FFI calls: %zu\n", ffi_calls);
    printf("  Pattern matches: %zu\n", pattern_matches);
    printf("  String operations: %zu\n", string_ops);
    printf("  Slice operations: %zu\n", slice_ops);
    printf("  Security operations: %zu\n", security_ops);
    printf("  Spawn statements: %zu\n", spawn_stmts);
    
    TEST_ASSERT(string_ops == 1, "Expected 1 string operation");
    TEST_ASSERT(slice_ops == 1, "Expected 1 slice operation");
    TEST_ASSERT(security_ops == 1, "Expected 1 security operation");
    TEST_ASSERT(spawn_stmts == 1, "Expected 1 spawn statement");
    
    minimal_ffi_generator_destroy(generator);
    
    TEST_SUCCESS();
}

// =============================================================================
// TEST UTILITIES
// =============================================================================

size_t tests_run = 0;
size_t tests_passed = 0;

void run_test(test_function_t test_func) {
    tests_run++;
    if (test_func()) {
        tests_passed++;
    }
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("Minimal FFI Assembly Generator Test Suite\n");
    printf("==========================================\n\n");
    
    run_test(test_ffi_generator_creation);
    run_test(test_ffi_generator_configuration);
    run_test(test_string_operations);
    run_test(test_slice_operations);
    run_test(test_security_operations);
    run_test(test_concurrency_operations);
    run_test(test_assembly_validation);
    run_test(test_nasm_output);
    run_test(test_generation_statistics);
    
    printf("\n==========================================\n");
    printf("Test Results:\n");
    printf("  Tests run: %zu\n", tests_run);
    printf("  Tests passed: %zu\n", tests_passed);
    printf("  Tests failed: %zu\n", tests_run - tests_passed);
    printf("  Success rate: %.1f%%\n", tests_run > 0 ? (100.0 * tests_passed / tests_run) : 0.0);
    
    return (tests_passed == tests_run) ? 0 : 1;
} 
