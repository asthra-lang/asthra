#include "test_complex_struct_operations_common.h"

// Test metadata for all tests
static AsthraTestMetadata test_metadata[] = {
    {"test_nested_struct_operations", __FILE__, __LINE__, "Test nested struct creation and access",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_struct_inheritance_simulation", __FILE__, __LINE__,
     "Test struct inheritance-like behavior", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false,
     NULL},
    {"test_struct_method_dispatch", __FILE__, __LINE__, "Test method dispatch on structs",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test result tracking
typedef struct {
    const char *test_name;
    AsthraTestResult result;
    const char *description;
    AsthraTestSeverity severity;
} TestResult;

// Main test runner
int main(void) {
    printf("=== Complex Struct Operations Tests (Comprehensive Suite) ===\n");
    printf("Testing advanced struct functionality including nesting, inheritance simulation,\n");
    printf("method dispatch, and complex memory layouts.\n\n");

    AsthraTestContext context = {0};
    TestResult results[3];
    int passed = 0, total = 0;
    int high_priority_passed = 0, high_priority_total = 0;
    int medium_priority_passed = 0, medium_priority_total = 0;

    // Test 1: Nested Struct Operations
    printf("1. Running Nested Struct Operations Test...\n");
    results[0] =
        (TestResult){.test_name = "Nested Struct Operations",
                     .result = test_nested_struct_operations(&context),
                     .description = "Tests creation and access of nested struct hierarchies",
                     .severity = ASTHRA_TEST_SEVERITY_HIGH};

    if (results[0].result == ASTHRA_TEST_PASS) {
        printf("   ✅ PASS: Nested struct creation, field access, and hierarchy validation\n");
        passed++;
        high_priority_passed++;
    } else {
        printf("   ❌ FAIL: Issues with nested struct operations\n");
    }
    total++;
    high_priority_total++;

    // Test 2: Struct Inheritance Simulation
    printf("\n2. Running Struct Inheritance Simulation Test...\n");
    results[1] =
        (TestResult){.test_name = "Struct Inheritance Simulation",
                     .result = test_struct_inheritance_simulation(&context),
                     .description = "Tests inheritance-like behavior using struct composition",
                     .severity = ASTHRA_TEST_SEVERITY_HIGH};

    if (results[1].result == ASTHRA_TEST_PASS) {
        printf("   ✅ PASS: Base struct embedding, polymorphic access, and virtual methods\n");
        passed++;
        high_priority_passed++;
    } else {
        printf("   ❌ FAIL: Issues with inheritance simulation\n");
    }
    total++;
    high_priority_total++;

    // Test 3: Struct Method Dispatch
    printf("\n3. Running Struct Method Dispatch Test...\n");
    results[2] = (TestResult){.test_name = "Struct Method Dispatch",
                              .result = test_struct_method_dispatch(&context),
                              .description = "Tests virtual method dispatch through vtables",
                              .severity = ASTHRA_TEST_SEVERITY_MEDIUM};

    if (results[2].result == ASTHRA_TEST_PASS) {
        printf(
            "   ✅ PASS: Virtual method tables, dispatch simulation, and signature validation\n");
        passed++;
        medium_priority_passed++;
    } else {
        printf("   ❌ FAIL: Issues with method dispatch\n");
    }
    total++;
    medium_priority_total++;

    // Comprehensive Results Summary
    printf("\n"
           "="
           "="
           "="
           " COMPREHENSIVE TEST RESULTS "
           "="
           "="
           "="
           "\n");
    printf("Overall Results: %d/%d tests passed (%.1f%%)\n", passed, total,
           (passed * 100.0) / total);

    printf("\nBy Priority Level:\n");
    printf("  High Priority:   %d/%d passed (%.1f%%) - Core struct functionality\n",
           high_priority_passed, high_priority_total,
           (high_priority_passed * 100.0) / high_priority_total);
    printf("  Medium Priority: %d/%d passed (%.1f%%) - Advanced struct features\n",
           medium_priority_passed, medium_priority_total,
           (medium_priority_passed * 100.0) / medium_priority_total);

    printf("\nDetailed Test Results:\n");
    for (int i = 0; i < 3; i++) {
        const char *status = (results[i].result == ASTHRA_TEST_PASS) ? "PASS" : "FAIL";
        const char *priority =
            (results[i].severity == ASTHRA_TEST_SEVERITY_HIGH) ? "HIGH" : "MEDIUM";
        printf("  [%s] %s (%s): %s\n", status, results[i].test_name, priority,
               results[i].description);
    }

    // Feature Coverage Analysis
    printf("\nFeature Coverage Analysis:\n");
    printf("  ✓ Nested struct hierarchies (Point → Rectangle → Shape)\n");
    printf("  ✓ Struct composition for inheritance simulation\n");
    printf("  ✓ Virtual method tables and dispatch simulation\n");
    printf("  ✓ Memory layout and alignment validation\n");
    printf("  ✓ Packed struct behavior (no padding)\n");
    printf("  ✓ Union-like struct behavior (overlapping fields)\n");
    printf("  ✓ Field access through offsets and pointers\n");
    printf("  ✓ Method signature validation\n");

    // Performance and Quality Metrics
    printf("\nQuality Metrics:\n");
    if (passed == total) {
        printf("  🎯 Perfect Score: All tests passed\n");
        printf("  🚀 Production Ready: Complex struct operations fully validated\n");
    } else if (high_priority_passed == high_priority_total) {
        printf("  ⚠️  Core Functionality: All high-priority tests passed\n");
        printf("  🔧 Needs Work: Some advanced features need attention\n");
    } else {
        printf("  ❌ Critical Issues: Core struct functionality has problems\n");
        printf("  🛠️  Requires Fixes: Address failing tests before production\n");
    }

    printf("\nTest Suite Capabilities:\n");
    printf("  • Comprehensive struct operation validation\n");
    printf("  • Memory layout and alignment testing\n");
    printf("  • Inheritance pattern simulation\n");
    printf("  • Virtual method dispatch verification\n");
    printf("  • Cross-platform struct behavior validation\n");

    return (passed == total) ? 0 : 1;
}
