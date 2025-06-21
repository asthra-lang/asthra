/**
 * Minimal FFI Assembly Generator Test - Common Definitions
 * 
 * This header contains shared definitions and structures for the
 * minimal FFI assembly generator test suite.
 */

#ifndef TEST_FFI_MINIMAL_COMMON_H
#define TEST_FFI_MINIMAL_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

// =============================================================================
// TYPE DEFINITIONS
// =============================================================================

typedef enum {
    TARGET_ARCH_X86_64,
    TARGET_ARCH_COUNT
} TargetArchitecture;

typedef enum {
    CALLING_CONV_SYSTEM_V_AMD64,
    CALLING_CONV_COUNT
} CallingConvention;

typedef enum {
    ASTHRA_REG_RAX = 0, ASTHRA_REG_RCX, ASTHRA_REG_RDX, ASTHRA_REG_RBX, ASTHRA_REG_RSP, ASTHRA_REG_RBP, ASTHRA_REG_RSI, ASTHRA_REG_RDI,
    ASTHRA_REG_R8, ASTHRA_REG_R9, ASTHRA_REG_R10, ASTHRA_REG_R11, ASTHRA_REG_R12, ASTHRA_REG_R13, ASTHRA_REG_R14, ASTHRA_REG_R15,
    ASTHRA_REG_XMM0, ASTHRA_REG_XMM1, ASTHRA_REG_XMM2, ASTHRA_REG_XMM3, ASTHRA_REG_XMM4, ASTHRA_REG_XMM5, ASTHRA_REG_XMM6, ASTHRA_REG_XMM7,
    ASTHRA_REG_COUNT,
    ASTHRA_REG_NONE = -1
} Register;

// Minimal FFI Assembly Generator structure
typedef struct {
    TargetArchitecture target_arch;
    CallingConvention calling_conv;
    
    struct {
        bool enable_bounds_checking;
        bool enable_security_features;
        bool enable_concurrency;
        bool optimize_string_operations;
        bool optimize_pattern_matching;
        size_t max_variadic_args;
        bool pic_mode;
    } config;
    
    struct {
        char *gc_alloc;
        char *gc_free;
        char *slice_bounds_check;
        char *string_concat;
        char *string_interpolate;
        char *result_create_ok;
        char *result_create_err;
        char *spawn_task;
        char *secure_zero;
    } runtime_functions;
    
    atomic_uint_fast32_t ffi_calls_generated;
    atomic_uint_fast32_t pattern_matches_generated;
    atomic_uint_fast32_t string_operations_generated;
    atomic_uint_fast32_t slice_operations_generated;
    atomic_uint_fast32_t security_operations_generated;
    atomic_uint_fast32_t spawn_statements_generated;
} MinimalFFIAssemblyGenerator;

// =============================================================================
// TEST FRAMEWORK MACROS
// =============================================================================

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s - %s\n", __func__, message); \
            return false; \
        } \
    } while(0)

#define TEST_SUCCESS() \
    do { \
        printf("PASS: %s\n", __func__); \
        return true; \
    } while(0)

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Core generator functions
MinimalFFIAssemblyGenerator *minimal_ffi_generator_create(TargetArchitecture arch, CallingConvention conv);
void minimal_ffi_generator_destroy(MinimalFFIAssemblyGenerator *generator);

// String operations
bool minimal_generate_string_concatenation(MinimalFFIAssemblyGenerator *generator, Register left_reg, Register right_reg, Register result_reg);

// Slice operations
bool minimal_generate_slice_length_access(MinimalFFIAssemblyGenerator *generator, Register slice_reg, Register result_reg);
bool minimal_generate_slice_bounds_check(MinimalFFIAssemblyGenerator *generator, Register slice_reg, Register index_reg, const char *error_label);
bool minimal_generate_slice_to_ffi(MinimalFFIAssemblyGenerator *generator, Register slice_reg, Register ptr_reg, Register len_reg);

// Security operations
bool minimal_generate_volatile_memory_access(MinimalFFIAssemblyGenerator *generator, Register memory_reg, size_t size, bool is_read);
bool minimal_generate_secure_zero(MinimalFFIAssemblyGenerator *generator, Register memory_reg, Register size_reg);

// Concurrency operations
bool minimal_generate_task_creation(MinimalFFIAssemblyGenerator *generator, const char *function_name, Register *arg_regs, size_t arg_count, Register handle_reg);

// Assembly validation and output
bool minimal_validate_generated_assembly(MinimalFFIAssemblyGenerator *generator);
bool minimal_print_nasm_assembly(MinimalFFIAssemblyGenerator *generator, char *output_buffer, size_t buffer_size);

// Statistics
void minimal_get_generation_statistics(MinimalFFIAssemblyGenerator *generator,
                                     size_t *ffi_calls, size_t *pattern_matches,
                                     size_t *string_ops, size_t *slice_ops,
                                     size_t *security_ops, size_t *spawn_stmts);

// Test functions
bool test_ffi_generator_creation(void);
bool test_ffi_generator_configuration(void);
bool test_string_operations(void);
bool test_slice_operations(void);
bool test_security_operations(void);
bool test_concurrency_operations(void);
bool test_assembly_validation(void);
bool test_nasm_output(void);
bool test_generation_statistics(void);

// Test utilities
extern size_t tests_run;
extern size_t tests_passed;
typedef bool (*test_function_t)(void);
void run_test(test_function_t test_func);

#endif // TEST_FFI_MINIMAL_COMMON_H 
