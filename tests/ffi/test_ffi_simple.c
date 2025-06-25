/**
 * Simplified FFI Assembly Generator Test
 *
 * This test focuses on the core functionality without requiring
 * complex AST dependencies that may not be fully implemented yet.
 */

#include "../framework/test_framework.h"
#include "ffi_assembly_generator.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// BASIC FUNCTIONALITY TESTS
// =============================================================================

static AsthraTestResult test_ffi_generator_creation(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator->base_generator,
                                    "Base generator not initialized")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Test configuration defaults
    if (!ASTHRA_TEST_ASSERT_BOOL(context, generator->config.enable_bounds_checking,
                                 "Bounds checking should be enabled by default")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT_BOOL(context, generator->config.enable_security_features,
                                 "Security features should be enabled by default")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT_BOOL(context, generator->config.enable_concurrency,
                                 "Concurrency should be enabled by default")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Test runtime function names
    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator->runtime_functions.gc_alloc,
                                    "GC alloc function name not set")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator->runtime_functions.string_concat,
                                    "String concat function name not set")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator->runtime_functions.spawn_task,
                                    "Spawn task function name not set")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_ffi_generator_configuration(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test configuration changes
    generator->config.enable_bounds_checking = false;
    generator->config.pic_mode = true;
    generator->config.max_variadic_args = 16;

    if (!ASTHRA_TEST_ASSERT_BOOL(context, !generator->config.enable_bounds_checking,
                                 "Bounds checking configuration failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT_BOOL(context, generator->config.pic_mode,
                                 "PIC mode configuration failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, generator->config.max_variadic_args, 16,
                                   "Variadic args configuration failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_ffi_generator_statistics(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test initial statistics
    size_t ffi_calls, pattern_matches, string_ops, slice_ops, security_ops, spawn_stmts;
    ffi_get_generation_statistics(generator, &ffi_calls, &pattern_matches, &string_ops, &slice_ops,
                                  &security_ops, &spawn_stmts);

    if (!asthra_test_assert_int_eq(context, ffi_calls, 0, "Initial FFI calls should be 0")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, pattern_matches, 0,
                                   "Initial pattern matches should be 0")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, string_ops, 0,
                                   "Initial string operations should be 0")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, slice_ops, 0, "Initial slice operations should be 0")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, security_ops, 0,
                                   "Initial security operations should be 0")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, spawn_stmts, 0,
                                   "Initial spawn statements should be 0")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_string_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test string concatenation
    bool result = ffi_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI,
                                                    ASTHRA_REG_RAX);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "String concatenation generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Check statistics
    size_t string_ops = 0;
    ffi_get_generation_statistics(generator, NULL, NULL, &string_ops, NULL, NULL, NULL);
    if (!ASTHRA_TEST_ASSERT_GT(context, string_ops, 0, "String operation statistics not updated")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_slice_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test slice length access
    bool result = ffi_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RAX);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Slice length access generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Test slice bounds checking
    char bounds_error[] = "bounds_error";
    result =
        ffi_generate_slice_bounds_check(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, bounds_error);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Slice bounds check generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Test slice to FFI conversion
    result = ffi_generate_slice_to_ffi(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RDX);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Slice to FFI conversion failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Check statistics
    size_t slice_ops = 0;
    ffi_get_generation_statistics(generator, NULL, NULL, NULL, &slice_ops, NULL, NULL);
    if (!ASTHRA_TEST_ASSERT_GT(context, slice_ops, 0, "Slice operation statistics not updated")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_security_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test volatile memory access
    bool result = ffi_generate_volatile_memory_access(generator, ASTHRA_REG_RDI, 8, true);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Volatile memory read generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    result = ffi_generate_volatile_memory_access(generator, ASTHRA_REG_RDI, 8, false);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Volatile memory write generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Test secure memory zeroing
    result = ffi_generate_secure_zero(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Secure memory zeroing generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_concurrency_operations(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test task creation
    Register arg_regs[] = {ASTHRA_REG_RDI, ASTHRA_REG_RSI};
    char function_name[] = "test_function";
    bool result = ffi_generate_task_creation(generator, function_name, arg_regs, 2, ASTHRA_REG_RAX);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Task creation generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_assembly_validation(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Generate some operations first
    ffi_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RAX);
    ffi_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RAX);

    // Validate generated assembly
    bool result = ffi_validate_generated_assembly(generator);
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "Generated assembly validation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_nasm_output(AsthraTestContext *context) {
    asthra_test_context_start(context);

    FFIAssemblyGenerator *generator =
        ffi_assembly_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);

    if (!ASTHRA_TEST_ASSERT_POINTER(context, generator,
                                    "Failed to create FFI assembly generator")) {
        return ASTHRA_TEST_FAIL;
    }

    // Generate some operations
    ffi_generate_string_concatenation(generator, ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RAX);
    ffi_generate_slice_length_access(generator, ASTHRA_REG_RDI, ASTHRA_REG_RBX);

    // Test NASM output
    char output_buffer[4096];
    bool result = ffi_print_nasm_assembly(generator, output_buffer, sizeof(output_buffer));
    if (!ASTHRA_TEST_ASSERT_BOOL(context, result, "NASM assembly output generation failed")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    // Basic validation of output
    if (!ASTHRA_TEST_ASSERT_GT(context, strlen(output_buffer), 0, "NASM output is empty")) {
        ffi_assembly_generator_destroy(generator);
        return ASTHRA_TEST_FAIL;
    }

    printf("Generated NASM assembly (first 200 chars):\n%.200s...\n", output_buffer);

    ffi_assembly_generator_destroy(generator);

    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST EXECUTION
// =============================================================================

int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create_lightweight("FFI Assembly Generator Tests");

    asthra_test_suite_add_test(suite, "ffi_generator_creation", "Test FFI generator creation",
                               test_ffi_generator_creation);
    asthra_test_suite_add_test(suite, "ffi_generator_configuration",
                               "Test FFI generator configuration",
                               test_ffi_generator_configuration);
    asthra_test_suite_add_test(suite, "ffi_generator_statistics", "Test FFI generator statistics",
                               test_ffi_generator_statistics);
    asthra_test_suite_add_test(suite, "string_operations", "Test string operations",
                               test_string_operations);
    asthra_test_suite_add_test(suite, "slice_operations", "Test slice operations",
                               test_slice_operations);
    asthra_test_suite_add_test(suite, "security_operations", "Test security operations",
                               test_security_operations);
    asthra_test_suite_add_test(suite, "concurrency_operations", "Test concurrency operations",
                               test_concurrency_operations);
    asthra_test_suite_add_test(suite, "assembly_validation", "Test assembly validation",
                               test_assembly_validation);
    asthra_test_suite_add_test(suite, "nasm_output", "Test NASM output generation",
                               test_nasm_output);

    return asthra_test_suite_run_and_exit(suite);
}
