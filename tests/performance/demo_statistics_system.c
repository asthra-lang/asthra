/**
 * Demonstration of Enhanced Statistics Tracking System
 *
 * This file demonstrates the comprehensive statistics system implemented
 * in Phase 4 TODO cleanup, showcasing performance barriers, memory tracking,
 * and atomic statistics collection.
 */

#include "../../runtime/asthra_memory.h"
#include "../../runtime/asthra_statistics.h"
#include "../framework/test_framework.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    printf("=== Enhanced Statistics System Demonstration ===\n\n");

    // Initialize the statistics system
    asthra_statistics_init();
    printf("1. Statistics system initialized\n");

    // Demonstrate memory allocation tracking
    printf("\n2. Memory Allocation Tracking:\n");
    void *ptr1 = malloc(1024);
    asthra_atomic_stats_update_allocation(1024);
    printf("   Allocated 1024 bytes\n");

    void *ptr2 = malloc(2048);
    asthra_atomic_stats_update_allocation(2048);
    printf("   Allocated 2048 bytes\n");

    void *ptr3 = malloc(512);
    asthra_atomic_stats_update_allocation(512);
    printf("   Allocated 512 bytes\n");

    // Demonstrate deallocation tracking
    printf("\n3. Memory Deallocation Tracking:\n");
    free(ptr1);
    asthra_atomic_stats_update_deallocation(1024);
    printf("   Deallocated 1024 bytes\n");

    // Simulate some performance operations
    printf("\n4. Performance Statistics:\n");
    asthra_atomic_stats_update_compilation(5000000, 2, 150); // 5ms, 2 files, 150 lines
    printf("   Recorded compilation: 5ms, 2 files, 150 lines\n");

    asthra_atomic_stats_update_semantic_analysis(3000000, 45); // 3ms, 45 symbols
    printf("   Recorded semantic analysis: 3ms, 45 symbols\n");

    asthra_atomic_stats_update_code_generation(2000000); // 2ms
    printf("   Recorded code generation: 2ms\n");

    // Simulate GC operations
    printf("\n5. Garbage Collection Statistics:\n");
    uint64_t gc_start = asthra_atomic_stats_gc_start_timer();
    usleep(1000); // Simulate 1ms GC
    asthra_atomic_stats_gc_end_timer(gc_start);
    asthra_atomic_stats_record_gc_objects_collected(25, 4096);
    printf("   Recorded GC collection: ~1ms, 25 objects, 4096 bytes\n");

    // Simulate concurrency operations
    printf("\n6. Concurrency Statistics:\n");
    asthra_atomic_stats_record_thread_created();
    asthra_atomic_stats_record_sync_operation();
    asthra_atomic_stats_record_barrier_operation();
    asthra_atomic_stats_record_thread_destroyed();
    printf("   Recorded thread lifecycle and synchronization operations\n");

    // Generate and display report
    printf("\n7. Statistics Report (Human-Readable):\n");
    printf("----------------------------------------\n");
    asthra_statistics_print_report(stdout, false);

    printf("\n8. Statistics Report (JSON Format):\n");
    printf("------------------------------------\n");
    char *json_stats = asthra_statistics_export_json();
    if (json_stats) {
        printf("%s\n", json_stats);
        free(json_stats);
    }

    // Test snapshot functionality
    printf("\n9. Statistics Snapshot:\n");
    AsthraGlobalStatistics snapshot = asthra_statistics_get_snapshot();
    printf("   Memory allocations: %" PRIu64 "\n", snapshot.memory.total_allocations);
    printf("   Memory deallocations: %" PRIu64 "\n", snapshot.memory.total_deallocations);
    printf("   Current memory usage: %" PRIu64 " bytes\n", snapshot.memory.current_memory_usage);
    printf("   Peak memory usage: %" PRIu64 " bytes\n", snapshot.memory.peak_memory_usage);
    printf("   GC collections: %" PRIu64 "\n", snapshot.gc.collections_performed);
    printf("   Files compiled: %" PRIu32 "\n", snapshot.performance.files_compiled);
    printf("   Threads created: %" PRIu32 "\n", snapshot.concurrency.threads_created);

    // Test reset functionality
    printf("\n10. Testing Statistics Reset:\n");
    asthra_statistics_reset();
    AsthraGlobalStatistics reset_snapshot = asthra_statistics_get_snapshot();
    printf("   After reset - Memory allocations: %" PRIu64 "\n",
           reset_snapshot.memory.total_allocations);
    printf("   After reset - GC collections: %" PRIu64 "\n",
           reset_snapshot.gc.collections_performed);

    // Test file export
    printf("\n11. Exporting Statistics to File:\n");
    bool json_saved = asthra_statistics_save_to_file("/tmp/asthra_stats.json", true);
    bool text_saved = asthra_statistics_save_to_file("/tmp/asthra_stats.txt", false);
    printf("   JSON export: %s\n", json_saved ? "Success" : "Failed");
    printf("   Text export: %s\n", text_saved ? "Success" : "Failed");

    // Cleanup remaining memory
    free(ptr2);
    free(ptr3);

    // Shutdown statistics system
    asthra_statistics_shutdown();
    printf("\n12. Statistics system shutdown complete\n");

    printf("\n=== Features Implemented in TODO Cleanup ===\n");
    printf("✅ Thread-safe atomic statistics collection\n");
    printf("✅ Memory allocation/deallocation tracking\n");
    printf("✅ Garbage collection performance monitoring\n");
    printf("✅ Compilation and semantic analysis timing\n");
    printf("✅ Concurrency operations tracking\n");
    printf("✅ Performance barrier implementation (fixed)\n");
    printf("✅ Real-time statistics snapshots\n");
    printf("✅ JSON and human-readable reporting\n");
    printf("✅ File export capabilities\n");
    printf("✅ Peak memory usage tracking\n");
    printf("✅ System uptime monitoring\n");
    printf("✅ Statistics enable/disable controls\n");

    return 0;
}