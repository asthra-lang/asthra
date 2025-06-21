/**
 * Minimal FFI Assembly Generator Test - Slice Operations
 * 
 * This file contains slice operation assembly generation functions
 * for the minimal FFI test suite.
 */

#include "test_ffi_minimal_common.h"

// =============================================================================
// SLICE OPERATIONS IMPLEMENTATION
// =============================================================================

bool minimal_generate_slice_length_access(MinimalFFIAssemblyGenerator *generator, Register slice_reg, Register result_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->slice_operations_generated, 1);
    
    printf("  Generated slice length access: slice[%d].len -> %d\n", slice_reg, result_reg);
    printf("  Assembly: mov r%d, [r%d + 8]\n", result_reg, slice_reg);
    
    return true;
}

bool minimal_generate_slice_bounds_check(MinimalFFIAssemblyGenerator *generator, Register slice_reg, Register index_reg, const char *error_label) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->slice_operations_generated, 1);
    
    printf("  Generated slice bounds check: slice[%d][%d] with error label '%s'\n", slice_reg, index_reg, error_label);
    printf("  Assembly: mov rax, [r%d + 8]; cmp r%d, rax; jae %s\n", slice_reg, index_reg, error_label);
    
    return true;
}

bool minimal_generate_slice_to_ffi(MinimalFFIAssemblyGenerator *generator, Register slice_reg, Register ptr_reg, Register len_reg) {
    if (!generator) return false;
    
    atomic_fetch_add(&generator->slice_operations_generated, 1);
    
    printf("  Generated slice to FFI conversion: slice[%d] -> (ptr=%d, len=%d)\n", slice_reg, ptr_reg, len_reg);
    printf("  Assembly: mov r%d, [r%d]; mov r%d, [r%d + 8]\n", ptr_reg, slice_reg, len_reg, slice_reg);
    
    return true;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

#ifndef SKIP_MAIN
int main(void) {
    printf("Minimal FFI Slice Operations Test\n");
    printf("==================================\n");
    printf("This file contains slice operation functions for the FFI generator.\n");
    printf("Run test_ffi_minimal_tests for the complete test suite.\n");
    return 0;
}
#endif 
