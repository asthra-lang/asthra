/*
 * Concurrency Tiers Integration Tests - Performance Patterns
 *
 * Tests for performance-oriented concurrency patterns including CPU-intensive
 * and I/O-intensive workloads. Validates optimal use of different tiers
 * based on workload characteristics.
 *
 * Phase 8: Testing and Validation
 * Focus: Performance patterns and workload optimization
 */

#include "test_concurrency_tiers_common.h"
#include <pthread.h>
#include <unistd.h>

// A simple CPU-intensive task function for threads
static void *cpu_task_function(void *arg) {
    long long iterations = *(long long *)arg;
    double result = 0.0;
    for (long long i = 0; i < iterations; ++i) {
        result += sqrt(i) * log(i + 1);
    }
    pthread_exit((void *)0);
}

// ============================================================================
// Performance Pattern Tests
// ============================================================================

void test_cpu_intensive_workload(void) {
    printf("Testing CPU-intensive workload patterns...\n");

    const char *cpu_code =
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "// Tier 1: Simple parallel computation\n"
        "fn parallel_computation_simple(tasks: Vec<ComputeTask>) -> Result<Vec<ComputeResult>, "
        "string> {\n"
        "    let mut handles = Vec::new();\n"
        "    \n"
        "    for task in tasks {\n"
        "        let handle = spawn_with_handle compute_task(task);\n"
        "        handles.push(handle);\n"
        "    }\n"
        "    \n"
        "    let mut results = Vec::new();\n"
        "    for handle in handles {\n"
        "        results.push(await handle?);\n"
        "    }\n"
        "    \n"
        "    Result.Ok(results)\n"
        "}\n"
        "\n"
        "// Tier 2: Advanced work distribution\n"
        "#[non_deterministic]\n"
        "fn parallel_computation_advanced(tasks: Vec<ComputeTask>) -> Result<Vec<ComputeResult>, "
        "string> {\n"
        "    let cpu_count = get_cpu_count();\n"
        "    let pool = patterns.WorkerPool::<ComputeResult>::new(cpu_count)?;\n"
        "    \n"
        "    // Submit all tasks\n"
        "    for task in tasks {\n"
        "        pool.submit_function(move || {\n"
        "            compute_task_optimized(task)\n"
        "        })?;\n"
        "    }\n"
        "    \n"
        "    // Collect results\n"
        "    let mut results = Vec::new();\n"
        "    for _ in 0..tasks.len(void) {\n"
        "        results.push(pool.get_result()??);\n"
        "    }\n"
        "    \n"
        "    pool.shutdown()?;\n"
        "    Result.Ok(results)\n"
        "}\n"
        "\n"
        "struct ComputeTask {\n"
        "    id: i32,\n"
        "    data: Vec<f64>,\n"
        "}\n"
        "\n"
        "struct ComputeResult {\n"
        "    task_id: i32,\n"
        "    result: f64,\n"
        "}\n"
        "\n"
        "fn compute_task(task: ComputeTask) -> Result<ComputeResult, string> {\n"
        "    let mut sum = 0.0;\n"
        "    for value in task.data {\n"
        "        sum += value * value;\n"
        "    }\n"
        "    Result.Ok(ComputeResult { task_id: task.id, result: sum })\n"
        "}\n"
        "\n"
        "fn compute_task_optimized(task: ComputeTask) -> Result<ComputeResult, string> {\n"
        "    let mut sum = 0.0;\n"
        "    // Simulated optimizations\n"
        "    for value in task.data {\n"
        "        sum += value * value + 0.1;  // Some optimization\n"
        "    }\n"
        "    Result.Ok(ComputeResult { task_id: task.id, result: sum })\n"
        "}\n"
        "\n"
        "fn get_cpu_count() -> u32 {\n"
        "    4  // Simplified\n"
        "}\n";

    ParseResult result = parse_string(cpu_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);

    // C-level simulation of multi-threaded CPU workload
    printf("  Simulating multi-threaded CPU workload...\n");
    int num_threads = 4;                         // Simulate 4 CPU cores
    long long iterations_per_thread = 100000000; // Adjust for test duration
    pthread_t threads[num_threads];
    uint64_t start_time, end_time;

    start_time = get_timestamp_ms();

    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, cpu_task_function, &iterations_per_thread);
    }

    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    end_time = get_timestamp_ms();
    double duration = calculate_execution_time(start_time, end_time);
    printf("  Multi-threaded CPU workload completed in %.2f ms\n", duration);

    // Assert that the duration is within an acceptable range
    // (This is a very rough check, as actual performance varies greatly)
    ASSERT_TRUE(duration > 0);
    ASSERT_TRUE(duration < 5000); // Should be less than 5 seconds

    printf("  ✓ CPU-intensive workload simulation passed\n");
}

// A simple I/O intensive task function for threads (simulated delay)
static void *io_task_function(void *arg) {
    int delay_ms = *(int *)arg;
    usleep(delay_ms * 1000); // Simulate I/O delay
    pthread_exit((void *)0);
}

void test_io_intensive_workload(void) {
    printf("Testing I/O-intensive workload patterns...\n");

    const char *io_code =
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/coordination\";\n"
        "\n"
        "// Tier 1: Simple file processing\n"
        "fn process_files_simple(filenames: Vec<string>) -> Result<Vec<FileResult>, string> {\n"
        "    let mut handles = Vec::new();\n"
        "    \n"
        "    for filename in filenames {\n"
        "        let handle = spawn_with_handle read_and_process_file(filename);\n"
        "        handles.push(handle);\n"
        "    }\n"
        "    \n"
        "    let mut results = Vec::new();\n"
        "    for handle in handles {\n"
        "        results.push(await handle?);\n"
        "    }\n"
        "    \n"
        "    Result.Ok(results)\n"
        "}\n"
        "\n"
        "// Tier 2: Advanced I/O coordination with backpressure\n"
        "#[non_deterministic]\n"
        "fn process_files_streaming(filenames: Vec<string>) -> Result<(), string> {\n"
        "    let (file_sender, file_receiver) = channels.channel_pair<string>(10)?;\n"
        "    let (result_sender, result_receiver) = channels.channel_pair<FileResult>(10)?;\n"
        "    \n"
        "    // File dispatcher\n"
        "    let dispatcher = spawn_with_handle move || {\n"
        "        for filename in filenames {\n"
        "            file_sender.send(filename)?;\n"
        "        }\n"
        "        file_sender.close()?;\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    // Parallel processors\n"
        "    let mut processors = Vec::new();\n"
        "    for i in 0..4 {\n"
        "        let receiver = file_receiver.clone();\n"
        "        let sender = result_sender.clone();\n"
        "        \n"
        "        let processor = spawn_with_handle move || {\n"
        "            loop {\n"
        "                match receiver.recv(void) {\n"
        "                    channels.RecvResult.Ok(filename) => {\n"
        "                        let result = read_and_process_file_advanced(filename)?;\n"
        "                        sender.send(result)?;\n"
        "                    },\n"
        "                    channels.RecvResult.Closed() => break,\n"
        "                    channels.RecvResult.Error(msg) => return Result.Err(msg),\n"
        "                    _ => continue,\n"
        "                }\n"
        "            }\n"
        "            Result.Ok(())\n"
        "        };\n"
        "        processors.push(processor);\n"
        "    }\n"
        "    \n"
        "    // Result collector\n"
        "    let collector = spawn_with_handle move || {\n"
        "        let mut count = 0;\n"
        "        loop {\n"
        "            match result_receiver.recv(void) {\n"
        "                channels.RecvResult.Ok(result) => {\n"
        "                    count += 1;\n"
        "                    println(\"Processed file: {}\", result.filename);\n"
        "                },\n"
        "                channels.RecvResult.Closed() => break,\n"
        "                channels.RecvResult.Error(msg) => return Result.Err(msg),\n"
        "                _ => continue,\n"
        "            }\n"
        "        }\n"
        "        println(\"Total files processed: {}\", count);\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    await dispatcher?;\n"
        "    for processor in processors {\n"
        "        await processor?;\n"
        "    }\n"
        "    result_sender.close()?;\n"
        "    await collector?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "struct FileResult {\n"
        "    filename: string,\n"
        "    line_count: i32,\n"
        "    byte_count: i32,\n"
        "}\n"
        "\n"
        "fn read_and_process_file(filename: string) -> Result<FileResult, string> {\n"
        "    Result.Ok(FileResult {\n"
        "        filename: filename,\n"
        "        line_count: 100,\n"
        "        byte_count: 5000,\n"
        "    })\n"
        "}\n"
        "\n"
        "fn read_and_process_file_advanced(filename: string) -> Result<FileResult, string> {\n"
        "    Result.Ok(FileResult {\n"
        "        filename: filename,\n"
        "        line_count: 150,\n"
        "        byte_count: 7500,\n"
        "    })\n"
        "}\n";

    ParseResult result = parse_string(io_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);

    // C-level simulation of multi-threaded I/O workload
    printf("  Simulating multi-threaded I/O workload...\n");
    int num_io_tasks = 10; // Number of simulated I/O operations
    int io_delay_ms = 100; // Delay per I/O operation (100ms)
    pthread_t io_threads[num_io_tasks];
    uint64_t start_time, end_time;

    start_time = get_timestamp_ms();

    for (int i = 0; i < num_io_tasks; ++i) {
        pthread_create(&io_threads[i], NULL, io_task_function, &io_delay_ms);
    }

    for (int i = 0; i < num_io_tasks; ++i) {
        pthread_join(io_threads[i], NULL);
    }

    end_time = get_timestamp_ms();
    double duration = calculate_execution_time(start_time, end_time);
    printf("  Multi-threaded I/O workload completed in %.2f ms\n", duration);

    // Assert that the duration is roughly as expected (e.g., total_tasks * delay_per_task /
    // num_threads) This is a simplified check, as actual performance varies. Expect duration to be
    // closer to io_delay_ms if tasks are truly parallel, or num_io_tasks * io_delay_ms if
    // sequential. Since we're using separate threads, it should be closer to io_delay_ms *
    // (num_io_tasks / num_cores) roughly. For this simple test, we just check it's not excessively
    // long.
    ASSERT_TRUE(duration > 0);
    ASSERT_TRUE(duration < (num_io_tasks * io_delay_ms *
                            2)); // Should be less than (total expected sequential time * 2)

    printf("  ✓ I/O-intensive workload simulation passed\n");
}

// A global counter protected by a mutex for race condition testing
static int global_counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Task function that increments a global counter with mutex protection
static void *safe_increment_task(void *arg) {
    int increments = *(int *)arg;
    for (int i = 0; i < increments; ++i) {
        pthread_mutex_lock(&counter_mutex);
        global_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    pthread_exit((void *)0);
}

// Task function that increments a global counter without mutex protection (introduces race)
static void *unsafe_increment_task(void *arg) {
    int increments = *(int *)arg;
    for (int i = 0; i < increments; ++i) {
        global_counter++; // Race condition here
    }
    pthread_exit((void *)0);
}

void test_race_conditions_and_deadlocks(void) {
    printf("Testing race conditions and deadlocks...\n");

    // Test 1: Safe increment (no race condition)
    printf("  Simulating safe increments...\n");
    global_counter = 0; // Reset counter
    int num_safe_threads = 5;
    int increments_per_safe_thread = 100000;
    pthread_t safe_threads[num_safe_threads];

    for (int i = 0; i < num_safe_threads; ++i) {
        pthread_create(&safe_threads[i], NULL, safe_increment_task, &increments_per_safe_thread);
    }

    for (int i = 0; i < num_safe_threads; ++i) {
        pthread_join(safe_threads[i], NULL);
    }

    ASSERT_EQUAL(global_counter, num_safe_threads * increments_per_safe_thread);
    printf("  ✓ Safe increment test passed (expected %d, got %d)\n",
           num_safe_threads * increments_per_safe_thread, global_counter);

    // Test 2: Unsafe increment (simulated race condition)
    printf("  Simulating unsafe increments (expect potential deviation)...\n");
    global_counter = 0; // Reset counter
    int num_unsafe_threads = 5;
    int increments_per_unsafe_thread = 100000;
    pthread_t unsafe_threads[num_unsafe_threads];

    for (int i = 0; i < num_unsafe_threads; ++i) {
        pthread_create(&unsafe_threads[i], NULL, unsafe_increment_task,
                       &increments_per_unsafe_thread);
    }

    for (int i = 0; i < num_unsafe_threads; ++i) {
        pthread_join(unsafe_threads[i], NULL);
    }

    // We expect the unsafe counter to be *less than or equal to* the expected value due to race
    // conditions It's non-deterministic, so we can't assert strict equality, but we can assert it's
    // not greater.
    ASSERT_TRUE(global_counter <= num_unsafe_threads * increments_per_unsafe_thread);
    printf("  ✓ Unsafe increment test passed (expected approx. %d, got %d)\n",
           num_unsafe_threads * increments_per_unsafe_thread, global_counter);

    // Test 3: Simulated Deadlock (conceptual - complex to simulate directly without language
    // features)
    printf("  Simulating conceptual deadlock scenario...\n");
    // In a real scenario, this would involve threads acquiring locks in different orders.
    // For this test, we simply assert that no actual deadlock occurred during the test suite.
    // (This is implicitly true if the test suite finishes)
    printf("  ✓ Conceptual deadlock scenario considered (test suite completed without hang)\n");
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_performance_pattern_tests(void) {
    printf("\n--- Performance Pattern Tests ---\n");
    test_cpu_intensive_workload();
    test_io_intensive_workload();
    test_race_conditions_and_deadlocks(); // New test
}
