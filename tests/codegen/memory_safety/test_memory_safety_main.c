#include "test_memory_safety_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_ownership_tracking", __FILE__, __LINE__, "Test ownership and borrowing tracking",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_lifetime_management", __FILE__, __LINE__, "Test lifetime analysis and validation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_memory_leak_detection", __FILE__, __LINE__, "Test memory leak detection and prevention",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_bounds_checking", __FILE__, __LINE__, "Test array bounds and buffer overflow protection",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test function structure
typedef struct {
    const char *name;
    AsthraTestResult (*function)(AsthraTestContext *);
    const char *description;
    bool is_critical;
} MemorySafetyTest;

// Test suite definition
static MemorySafetyTest memory_safety_tests[] = {
    {"Ownership Tracking", test_ownership_tracking,
     "Tests ownership transfer, borrowing validation, and reference counting", true},
    {"Lifetime Management", test_lifetime_management,
     "Tests scope-based lifetime analysis and automatic cleanup", true},
    {"Memory Leak Detection", test_memory_leak_detection,
     "Tests leak prevention and statistics tracking", false},
    {"Bounds Checking", test_bounds_checking,
     "Tests array bounds validation and buffer overflow protection", false}};

#define NUM_MEMORY_SAFETY_TESTS (sizeof(memory_safety_tests) / sizeof(memory_safety_tests[0]))

// Test execution and reporting
static void print_test_header(void) {
    printf("=== Memory Safety Systems Tests (Minimal Framework) ===\n");
    printf("Testing comprehensive memory safety features for Asthra:\n");
    printf("- Ownership tracking and borrowing validation\n");
    printf("- Lifetime analysis and scope management\n");
    printf("- Memory leak detection and prevention\n");
    printf("- Array bounds checking and buffer overflow protection\n\n");
}

static void print_test_summary(int passed, int total, int critical_passed, int critical_total) {
    printf("\n=== Test Results Summary ===\n");
    printf("Total Tests:    %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    printf("Critical Tests: %d/%d (%.1f%%)\n", critical_passed, critical_total,
           (critical_passed * 100.0) / critical_total);

    if (passed == total) {
        printf("✅ All memory safety tests PASSED\n");
    } else if (critical_passed == critical_total) {
        printf("⚠️  Critical tests PASSED, some optional tests failed\n");
    } else {
        printf("❌ CRITICAL memory safety tests FAILED\n");
    }

    printf("\nMemory Safety Coverage:\n");
    printf("- Ownership System:    %s\n", (critical_passed >= 1) ? "✅ VALIDATED" : "❌ FAILED");
    printf("- Lifetime Management: %s\n", (critical_passed >= 2) ? "✅ VALIDATED" : "❌ FAILED");
    printf("- Leak Detection:      %s\n", (passed >= 3) ? "✅ VALIDATED" : "⚠️  PARTIAL");
    printf("- Bounds Checking:     %s\n", (passed >= 4) ? "✅ VALIDATED" : "⚠️  PARTIAL");
}

static void print_test_details(void) {
    printf("\n=== Test Categories ===\n");

    for (size_t i = 0; i < NUM_MEMORY_SAFETY_TESTS; i++) {
        MemorySafetyTest *test = &memory_safety_tests[i];
        printf("%zu. %s%s\n", i + 1, test->name, test->is_critical ? " (CRITICAL)" : "");
        printf("   %s\n", test->description);
    }
    printf("\n");
}

static bool run_individual_test(MemorySafetyTest *test, AsthraTestContext *context) {
    printf("Running %s... ", test->name);
    fflush(stdout);

    AsthraTestResult result = test->function(context);

    if (result == ASTHRA_TEST_PASS) {
        printf("✅ PASS\n");
        return true;
    } else {
        printf("❌ FAIL\n");
        if (test->is_critical) {
            printf("   ⚠️  CRITICAL TEST FAILURE - Memory safety may be compromised\n");
        }
        return false;
    }
}

int main(void) {
    print_test_header();
    print_test_details();

    AsthraTestContext context = {0};
    int passed = 0;
    int critical_passed = 0;
    int critical_total = 0;

    // Count critical tests
    for (size_t i = 0; i < NUM_MEMORY_SAFETY_TESTS; i++) {
        if (memory_safety_tests[i].is_critical) {
            critical_total++;
        }
    }

    printf("=== Executing Tests ===\n");

    // Run all tests
    for (size_t i = 0; i < NUM_MEMORY_SAFETY_TESTS; i++) {
        MemorySafetyTest *test = &memory_safety_tests[i];

        if (run_individual_test(test, &context)) {
            passed++;
            if (test->is_critical) {
                critical_passed++;
            }
        }
    }

    print_test_summary(passed, NUM_MEMORY_SAFETY_TESTS, critical_passed, critical_total);

    // Return appropriate exit code
    if (critical_passed == critical_total) {
        return 0; // Success if all critical tests pass
    } else {
        return 1; // Failure if any critical test fails
    }
}
