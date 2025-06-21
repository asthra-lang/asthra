/**
 * Enhanced FFI Test Infrastructure  
 * Main file that includes all modular FFI test components
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <assert.h>

// Include all modular FFI stubs components
#include "ffi_stubs_types.h"
#include "ffi_stubs_generator.h"
#include "ffi_stubs_memory.h"
#include "ffi_stubs_marshaling.h"
#include "ffi_stubs_ast_parser.h"
#include "ffi_stubs_codegen.h"
#include "ffi_stubs_test_suite.h"

// =============================================================================
// MAIN FUNCTION (for standalone compilation)
// =============================================================================

int main(void) {
    printf("Enhanced FFI Stubs Test - Comprehensive Infrastructure Testing\n");
    
    // Run comprehensive tests
    run_enhanced_ffi_tests();
    
    // Cleanup
    cleanup_enhanced_ffi_infrastructure();
    
    printf("\nAll enhanced FFI stub tests completed successfully!\n");
    return 0;
}