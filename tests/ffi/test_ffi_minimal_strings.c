/**
 * Minimal FFI Assembly Generator Test - String Operations
 * 
 * This file contains string operation assembly generation functions
 * for the minimal FFI test suite.
 */

#include "test_ffi_minimal_common.h"

// =============================================================================
// STRING OPERATIONS IMPLEMENTATION
// =============================================================================

bool minimal_generate_string_concatenation(MinimalFFIAssemblyGenerator *generator, Register left_reg, Register right_reg, Register result_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->string_operations_generated, 1);
    
    printf("  Generated string concatenation: concat(r%d, r%d) -> r%d\n", left_reg, right_reg, result_reg);
    printf("  Assembly: call %s\n", generator->runtime_functions.string_concat);
    
    return true;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("Minimal FFI String Operations Test\n");
    printf("===================================\n");
    printf("This file contains string operation functions for the FFI generator.\n");
    printf("Run test_ffi_minimal_tests for the complete test suite.\n");
    return 0;
} 
