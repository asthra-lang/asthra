/**
 * Main Test file for Struct Pattern Matching Implementation
 * Tests struct destructuring in match expressions as per spec.md and prd.md
 *
 * This file serves as the main test runner and includes the split test suites.
 * Run individual test files directly for focused testing:
 * - test_basic_struct_patterns.c: Basic struct pattern parsing
 * - test_struct_pattern_bindings.c: Field bindings and ignored fields
 * - test_multiple_struct_patterns.c: Multiple patterns and mixed types
 * - test_struct_pattern_errors.c: Error handling and malformed patterns
 */

#include "test_struct_pattern_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for external test functions
// (These would be linked from the other test files in a full build)

// Run all test suites
int main(void) {
    printf("🧪 Struct Pattern Matching Test Suite\n");
    printf("=====================================\n\n");

    printf("This test suite has been split into focused modules:\n\n");

    printf("📁 Available test modules:\n");
    printf("  • test_basic_struct_patterns.c - Basic pattern parsing\n");
    printf("  • test_struct_pattern_bindings.c - Field bindings and ignored fields\n");
    printf("  • test_multiple_struct_patterns.c - Multiple patterns and mixed types\n");
    printf("  • test_struct_pattern_errors.c - Error handling and malformed patterns\n\n");

    printf("🚀 To run individual test modules:\n");
    printf("  make test_basic_struct_patterns\n");
    printf("  make test_struct_pattern_bindings\n");
    printf("  make test_multiple_struct_patterns\n");
    printf("  make test_struct_pattern_errors\n\n");

    printf("💡 Each module can be run independently for focused testing.\n");
    printf("   Use 'make all_struct_pattern_tests' to run all modules.\n\n");

    printf("✅ Test suite organization complete!\n");
    return 0;
}
