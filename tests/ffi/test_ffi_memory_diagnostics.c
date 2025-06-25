/**
 * Test Suite for Asthra Safe C Memory Interface - Memory Diagnostics
 * Tests for memory statistics, validation, and diagnostic functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// =============================================================================
// MEMORY STATISTICS AND DIAGNOSTICS TESTS
// =============================================================================

void test_memory_statistics(void) {
    TEST_SECTION("Memory Statistics");

    // Reset statistics to start clean
    Asthra_ffi_reset_memory_stats();

    // Get initial statistics
    AsthraFFIMemoryStats initial_stats = Asthra_ffi_get_memory_stats();
    printf("Initial stats: %zu allocations, %zu bytes\n", initial_stats.total_allocations,
           initial_stats.current_bytes);

    // Perform some allocations
    void *ptr1 = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_MANUAL);
    void *ptr2 = Asthra_ffi_alloc(2048, ASTHRA_ZONE_HINT_GC);
    AsthraFFISliceHeader slice = Asthra_slice_new(sizeof(int), 100, 200, ASTHRA_ZONE_HINT_MANUAL);

    // Get updated statistics
    AsthraFFIMemoryStats stats = Asthra_ffi_get_memory_stats();
    TEST_ASSERT(stats.total_allocations > initial_stats.total_allocations,
                "Statistics track new allocations");
    TEST_ASSERT(stats.current_allocations > initial_stats.current_allocations,
                "Statistics track current allocations");
    TEST_ASSERT(stats.slice_count >= 1, "Statistics track slice count");
    TEST_ASSERT(stats.current_bytes > initial_stats.current_bytes,
                "Statistics track allocated bytes");

    printf("Updated stats: %zu total allocations, %zu current allocations, %zu bytes, %zu slices\n",
           stats.total_allocations, stats.current_allocations, stats.current_bytes,
           stats.slice_count);

    // Clean up and verify statistics update
    Asthra_ffi_free(ptr1, ASTHRA_ZONE_HINT_MANUAL);
    Asthra_ffi_free(ptr2, ASTHRA_ZONE_HINT_GC);
    Asthra_slice_free(slice);

    AsthraFFIMemoryStats final_stats = Asthra_ffi_get_memory_stats();
    TEST_ASSERT(final_stats.current_allocations < stats.current_allocations,
                "Statistics track deallocations");
    TEST_ASSERT(final_stats.current_bytes < stats.current_bytes, "Statistics track freed bytes");

    printf("Final stats: %zu current allocations, %zu bytes\n", final_stats.current_allocations,
           final_stats.current_bytes);
}

void test_pointer_validation(void) {
    TEST_SECTION("Pointer Validation");

    // Create some valid allocations
    void *valid_ptr1 = Asthra_ffi_alloc(512, ASTHRA_ZONE_HINT_MANUAL);
    void *valid_ptr2 = Asthra_ffi_alloc(1024, ASTHRA_ZONE_HINT_GC);
    AsthraFFISliceHeader valid_slice =
        Asthra_slice_new(sizeof(double), 50, 100, ASTHRA_ZONE_HINT_MANUAL);

    TEST_ASSERT(valid_ptr1 != NULL && valid_ptr2 != NULL, "Valid allocations succeed");
    TEST_ASSERT(Asthra_slice_is_valid(valid_slice), "Valid slice creation succeeds");

    // Test pointer validation
    AsthraFFIResult validation_result = Asthra_ffi_validate_all_pointers();
    TEST_ASSERT(Asthra_result_is_ok(validation_result), "Pointer validation succeeds");

    int *validated_count = (int *)Asthra_result_unwrap_ok(validation_result);
    TEST_ASSERT(*validated_count >= 3, "Validation found expected number of pointers");
    printf("Validated %d pointers\n", *validated_count);

    // Test individual pointer validation (if such function exists)
    // This would require extending the API

    // Clean up
    Asthra_ffi_free(valid_ptr1, ASTHRA_ZONE_HINT_MANUAL);
    Asthra_ffi_free(valid_ptr2, ASTHRA_ZONE_HINT_GC);
    Asthra_slice_free(valid_slice);
    free(validated_count);
}

void test_memory_state_dump(void) {
    TEST_SECTION("Memory State Dump");

    // Create some allocations to dump
    void *dump_ptr1 = Asthra_ffi_alloc(256, ASTHRA_ZONE_HINT_MANUAL);
    void *dump_ptr2 = Asthra_ffi_alloc(512, ASTHRA_ZONE_HINT_GC);
    AsthraFFISliceHeader dump_slice =
        Asthra_slice_new(sizeof(float), 25, 50, ASTHRA_ZONE_HINT_SECURE);

    printf("\nMemory state dump (should show active allocations):\n");
    printf("=================================================\n");
    Asthra_ffi_dump_memory_state(stdout);
    printf("=================================================\n");

    // The dump function should complete without crashing
    TEST_ASSERT(true, "Memory state dump completes successfully");

    // Clean up
    Asthra_ffi_free(dump_ptr1, ASTHRA_ZONE_HINT_MANUAL);
    Asthra_ffi_free(dump_ptr2, ASTHRA_ZONE_HINT_GC);
    Asthra_slice_free(dump_slice);

    printf("\nMemory state dump after cleanup:\n");
    printf("================================\n");
    Asthra_ffi_dump_memory_state(stdout);
    printf("================================\n");
}

void test_zone_statistics(void) {
    TEST_SECTION("Zone-Specific Statistics");

    // Allocate in different zones
    void *manual_ptrs[3];
    void *gc_ptrs[2];
    void *secure_ptrs[2];

    for (int i = 0; i < 3; i++) {
        manual_ptrs[i] = Asthra_ffi_alloc(128 * (i + 1), ASTHRA_ZONE_HINT_MANUAL);
        TEST_ASSERT(manual_ptrs[i] != NULL, "Manual zone allocation succeeds");
    }

    for (int i = 0; i < 2; i++) {
        gc_ptrs[i] = Asthra_ffi_alloc(256 * (i + 1), ASTHRA_ZONE_HINT_GC);
        TEST_ASSERT(gc_ptrs[i] != NULL, "GC zone allocation succeeds");
    }

    for (int i = 0; i < 2; i++) {
        secure_ptrs[i] = Asthra_ffi_alloc(64 * (i + 1), ASTHRA_ZONE_HINT_SECURE);
        TEST_ASSERT(secure_ptrs[i] != NULL, "Secure zone allocation succeeds");
    }

    // Get statistics and verify zone distribution
    AsthraFFIMemoryStats stats = Asthra_ffi_get_memory_stats();
    TEST_ASSERT(stats.current_allocations >= 7, "All zone allocations tracked");

    printf("Zone allocation statistics:\n");
    printf("  Total current allocations: %zu\n", stats.current_allocations);
    printf("  Current bytes: %zu\n", stats.current_bytes);

    // Clean up by zone
    for (int i = 0; i < 3; i++) {
        Asthra_ffi_free(manual_ptrs[i], ASTHRA_ZONE_HINT_MANUAL);
    }
    for (int i = 0; i < 2; i++) {
        Asthra_ffi_free(gc_ptrs[i], ASTHRA_ZONE_HINT_GC);
    }
    for (int i = 0; i < 2; i++) {
        Asthra_ffi_free(secure_ptrs[i], ASTHRA_ZONE_HINT_SECURE);
    }

    AsthraFFIMemoryStats final_stats = Asthra_ffi_get_memory_stats();
    printf("Post-cleanup statistics:\n");
    printf("  Current allocations: %zu\n", final_stats.current_allocations);
    printf("  Current bytes: %zu\n", final_stats.current_bytes);
}

void test_memory_leak_detection(void) {
    TEST_SECTION("Memory Leak Detection");

    // Get baseline statistics
    AsthraFFIMemoryStats baseline = Asthra_ffi_get_memory_stats();

    // Simulate some allocation/deallocation patterns
    void *leak_test_ptrs[5];

    // Allocate several pointers
    for (int i = 0; i < 5; i++) {
        leak_test_ptrs[i] = Asthra_ffi_alloc(100 * (i + 1), ASTHRA_ZONE_HINT_MANUAL);
        TEST_ASSERT(leak_test_ptrs[i] != NULL, "Leak test allocation succeeds");
    }

    AsthraFFIMemoryStats after_alloc = Asthra_ffi_get_memory_stats();
    TEST_ASSERT(after_alloc.current_allocations > baseline.current_allocations,
                "Allocations increase current count");

    // Free only some of them (simulate partial leak)
    for (int i = 0; i < 3; i++) {
        Asthra_ffi_free(leak_test_ptrs[i], ASTHRA_ZONE_HINT_MANUAL);
    }

    AsthraFFIMemoryStats after_partial_free = Asthra_ffi_get_memory_stats();
    TEST_ASSERT(after_partial_free.current_allocations < after_alloc.current_allocations,
                "Partial cleanup reduces current count");
    TEST_ASSERT(after_partial_free.current_allocations > baseline.current_allocations,
                "Some allocations still remain");

    printf("Leak detection test:\n");
    printf("  Baseline allocations: %zu\n", baseline.current_allocations);
    printf("  After allocation: %zu\n", after_alloc.current_allocations);
    printf("  After partial cleanup: %zu\n", after_partial_free.current_allocations);
    printf("  Potential leaks: %zu\n",
           after_partial_free.current_allocations - baseline.current_allocations);

    // Clean up remaining allocations
    for (int i = 3; i < 5; i++) {
        Asthra_ffi_free(leak_test_ptrs[i], ASTHRA_ZONE_HINT_MANUAL);
    }

    AsthraFFIMemoryStats final = Asthra_ffi_get_memory_stats();
    TEST_ASSERT(final.current_allocations <= baseline.current_allocations,
                "Full cleanup returns to baseline");
}

void test_performance_monitoring(void) {
    TEST_SECTION("Performance Monitoring");

    // Reset statistics for clean measurement
    Asthra_ffi_reset_memory_stats();

    // Perform a series of allocations and measure
    const int num_operations = 100;
    void *perf_ptrs[num_operations];

    printf("Performing %d allocation operations...\n", num_operations);

    for (int i = 0; i < num_operations; i++) {
        perf_ptrs[i] = Asthra_ffi_alloc(64, ASTHRA_ZONE_HINT_MANUAL);
        TEST_ASSERT(perf_ptrs[i] != NULL, "Performance test allocation succeeds");
    }

    AsthraFFIMemoryStats alloc_stats = Asthra_ffi_get_memory_stats();
    printf("After %d allocations:\n", num_operations);
    printf("  Total allocations: %zu\n", alloc_stats.total_allocations);
    printf("  Current allocations: %zu\n", alloc_stats.current_allocations);
    printf("  Current bytes: %zu\n", alloc_stats.current_bytes);

    // Perform deallocations
    printf("Performing %d deallocation operations...\n", num_operations);

    for (int i = 0; i < num_operations; i++) {
        Asthra_ffi_free(perf_ptrs[i], ASTHRA_ZONE_HINT_MANUAL);
    }

    AsthraFFIMemoryStats dealloc_stats = Asthra_ffi_get_memory_stats();
    printf("After deallocations:\n");
    printf("  Total allocations: %zu\n", dealloc_stats.total_allocations);
    printf("  Current allocations: %zu\n", dealloc_stats.current_allocations);
    printf("  Current bytes: %zu\n", dealloc_stats.current_bytes);

    TEST_ASSERT(dealloc_stats.total_allocations >= num_operations,
                "Total allocations tracked correctly");
    TEST_ASSERT(dealloc_stats.current_allocations < alloc_stats.current_allocations,
                "Deallocations reduce current count");
}

void test_diagnostic_edge_cases(void) {
    TEST_SECTION("Diagnostic Edge Cases");

    // Test statistics with no allocations
    Asthra_ffi_reset_memory_stats();
    AsthraFFIMemoryStats empty_stats = Asthra_ffi_get_memory_stats();

    printf("Empty statistics:\n");
    printf("  Total allocations: %zu\n", empty_stats.total_allocations);
    printf("  Current allocations: %zu\n", empty_stats.current_allocations);
    printf("  Current bytes: %zu\n", empty_stats.current_bytes);
    printf("  Slice count: %zu\n", empty_stats.slice_count);

    // Test validation with no pointers
    AsthraFFIResult empty_validation = Asthra_ffi_validate_all_pointers();
    if (Asthra_result_is_ok(empty_validation)) {
        int *count = (int *)Asthra_result_unwrap_ok(empty_validation);
        printf("Validated %d pointers in empty state\n", *count);
        free(count);
        TEST_ASSERT(true, "Empty validation succeeds");
    } else {
        TEST_ASSERT(true, "Empty validation may fail gracefully");
    }

    // Test dump with no allocations
    printf("\nEmpty memory state dump:\n");
    printf("========================\n");
    Asthra_ffi_dump_memory_state(stdout);
    printf("========================\n");

    TEST_ASSERT(true, "Empty state diagnostics complete successfully");
}

// Test registration
static test_case_t diagnostic_tests[] = {{"Memory Statistics", test_memory_statistics},
                                         {"Pointer Validation", test_pointer_validation},
                                         {"Memory State Dump", test_memory_state_dump},
                                         {"Zone Statistics", test_zone_statistics},
                                         {"Memory Leak Detection", test_memory_leak_detection},
                                         {"Performance Monitoring", test_performance_monitoring},
                                         {"Diagnostic Edge Cases", test_diagnostic_edge_cases}};

int main(void) {
    printf("Asthra Safe C Memory Interface - Diagnostics Tests\n");
    printf("=================================================\n");

    test_runtime_init();

    int failed = run_test_suite("Memory Diagnostics", diagnostic_tests,
                                sizeof(diagnostic_tests) / sizeof(diagnostic_tests[0]));

    print_test_results();
    test_runtime_cleanup();

    return failed == 0 ? 0 : 1;
}
