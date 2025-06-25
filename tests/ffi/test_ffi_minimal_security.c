/**
 * Minimal FFI Assembly Generator Test - Security Operations
 *
 * This file contains security operation assembly generation functions
 * for the minimal FFI test suite.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Forward declarations for functions provided by test_runner_minimal.c
typedef struct MinimalFFIGenerator MinimalFFIGenerator;
MinimalFFIGenerator *minimal_ffi_generator_create(void);
void minimal_ffi_generator_destroy(MinimalFFIGenerator *gen);
bool minimal_generate_secure_zero(MinimalFFIGenerator *gen, const char *operation);
bool minimal_generate_volatile_memory_access(MinimalFFIGenerator *gen, const char *operation);

// =============================================================================
// SECURITY OPERATIONS TEST
// =============================================================================

bool test_security_operations(void) {
    printf("Testing security operations...\n");

    MinimalFFIGenerator *generator = minimal_ffi_generator_create();
    if (!generator) {
        printf("Failed to initialize generator\n");
        return false;
    }

    // Test secure zero
    bool result = minimal_generate_secure_zero(generator, "test_secure_zero_operation");
    if (!result) {
        printf("Secure zero generation failed\n");
        minimal_ffi_generator_destroy(generator);
        return false;
    }

    // Test volatile memory access
    result = minimal_generate_volatile_memory_access(generator, "test_volatile_memory_access");
    if (!result) {
        printf("Volatile memory access generation failed\n");
        minimal_ffi_generator_destroy(generator);
        return false;
    }

    minimal_ffi_generator_destroy(generator);
    return true;
}

#ifndef SKIP_MAIN
int main(void) {
    printf("Minimal FFI Security Operations Test\n");
    printf("=====================================\n");

    if (test_security_operations()) {
        printf("✅ Security operations test passed\n");
        return 0;
    } else {
        printf("❌ Security operations test failed\n");
        return 1;
    }
}
#endif
