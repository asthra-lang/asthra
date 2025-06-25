/*
 * Concurrency Tiers Integration Tests - Migration Patterns
 *
 * Tests for migration patterns from other programming languages to Asthra's
 * three-tier concurrency system. Demonstrates how common concurrency patterns
 * from languages like Go can be adapted to Asthra.
 *
 * Phase 8: Testing and Validation
 * Focus: Migration patterns and language interoperability
 */

#include "test_concurrency_tiers_common.h"

// ============================================================================
// Migration Pattern Tests
// ============================================================================

void test_go_to_asthra_migration(void) {
    printf("Testing Go to Asthra migration patterns...\n");

    // Simulate Go-style concurrent code migrated to Asthra
    const char *migrated_code =
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "// Originally: func worker(jobs <-chan Job, results chan<- Result)\n"
        "#[non_deterministic]\n"
        "fn worker(jobs: channels.Receiver<Job>, results: channels.Sender<Result>) -> Result<(), "
        "string> {\n"
        "    loop {\n"
        "        match jobs.recv(void) {\n"
        "            channels.RecvResult.Ok(job) => {\n"
        "                let result = process_job(job)?;\n"
        "                results.send(result)?;\n"
        "            },\n"
        "            channels.RecvResult.Closed() => break,\n"
        "            channels.RecvResult.Error(msg) => return Result.Err(msg),\n"
        "            _ => continue,\n"
        "        }\n"
        "    }\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "// Originally: func main() with goroutines\n"
        "#[non_deterministic]\n"
        "fn run_workers() -> Result<(), string> {\n"
        "    let (job_sender, job_receiver) = channels.channel_pair<Job>(100)?;\n"
        "    let (result_sender, result_receiver) = channels.channel_pair<Result>(100)?;\n"
        "    \n"
        "    // Start workers (originally: go worker(jobs, results))\n"
        "    let mut worker_handles = Vec::new();\n"
        "    for i in 0..3 {\n"
        "        let jobs = job_receiver.clone();\n"
        "        let results = result_sender.clone();\n"
        "        \n"
        "        let handle = spawn_with_handle move || {\n"
        "            worker(jobs, results)\n"
        "        };\n"
        "        worker_handles.push(handle);\n"
        "    }\n"
        "    \n"
        "    // Send jobs\n"
        "    for i in 0..10 {\n"
        "        let job = Job { id: i, data: i * 2 };\n"
        "        job_sender.send(job)?;\n"
        "    }\n"
        "    job_sender.close()?;\n"
        "    \n"
        "    // Collect results\n"
        "    for i in 0..10 {\n"
        "        match result_receiver.recv(void) {\n"
        "            channels.RecvResult.Ok(result) => {\n"
        "                println(\"Result {}: {}\", result.job_id, result.value);\n"
        "            },\n"
        "            _ => break,\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    // Wait for workers\n"
        "    for handle in worker_handles {\n"
        "        await handle?;\n"
        "    }\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "struct Job {\n"
        "    id: i32,\n"
        "    data: i32,\n"
        "}\n"
        "\n"
        "struct Result {\n"
        "    job_id: i32,\n"
        "    value: i32,\n"
        "}\n"
        "\n"
        "fn process_job(job: Job) -> Result<Result, string> {\n"
        "    Result.Ok(Result {\n"
        "        job_id: job.id,\n"
        "        value: job.data * 2,\n"
        "    })\n"
        "}\n";

    ParseResult result = parse_string(migrated_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    // Verify all channel-using functions have proper annotations
    ASTNode *worker_func = find_function_declaration(result.ast, "worker");
    ASSERT_NOT_NULL(worker_func);
    ASSERT_TRUE(has_annotation(worker_func, "non_deterministic"));

    ASTNode *run_func = find_function_declaration(result.ast, "run_workers");
    ASSERT_NOT_NULL(run_func);
    ASSERT_TRUE(has_annotation(run_func, "non_deterministic"));

    // Helper function shouldn't need annotation
    ASTNode *process_func = find_function_declaration(result.ast, "process_job");
    ASSERT_NOT_NULL(process_func);
    ASSERT_FALSE(has_annotation(process_func, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_java_to_asthra_migration(void) {
    printf("Testing Java to Asthra migration patterns...\n");

    // Simulate Java ExecutorService pattern migrated to Asthra
    const char *java_migrated_code =
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "// Originally: ExecutorService executor = Executors.newFixedThreadPool(4);\n"
        "#[non_deterministic]\n"
        "fn create_executor_service() -> Result<patterns.WorkerPool<TaskResult>, string> {\n"
        "    patterns.WorkerPool::<TaskResult>::new(4)\n"
        "}\n"
        "\n"
        "// Originally: Future<Result> submit(Callable<Result> task)\n"
        "#[non_deterministic]\n"
        "fn submit_task(pool: patterns.WorkerPool<TaskResult>, task: Task) -> Result<(), string> "
        "{\n"
        "    pool.submit_function(move || {\n"
        "        execute_task(task)\n"
        "    })\n"
        "}\n"
        "\n"
        "// Originally: List<Future<Result>> futures = new ArrayList<>();\n"
        "#[non_deterministic]\n"
        "fn process_tasks_batch(tasks: Vec<Task>) -> Result<Vec<TaskResult>, string> {\n"
        "    let pool = create_executor_service()?;\n"
        "    \n"
        "    // Submit all tasks\n"
        "    for task in tasks {\n"
        "        submit_task(pool, task)?;\n"
        "    }\n"
        "    \n"
        "    // Collect results\n"
        "    let mut results = Vec::new();\n"
        "    for _ in 0..tasks.len(void) {\n"
        "        results.push(pool.get_result()"
        "?"
        "?"
        ");\n"
        "    }\n"
        "    \n"
        "    pool.shutdown()?;\n"
        "    Result.Ok(results)\n"
        "}\n"
        "\n"
        "struct Task {\n"
        "    id: i32,\n"
        "    work_data: string,\n"
        "}\n"
        "\n"
        "struct TaskResult {\n"
        "    task_id: i32,\n"
        "    output: string,\n"
        "}\n"
        "\n"
        "fn execute_task(task: Task) -> Result<TaskResult, string> {\n"
        "    Result.Ok(TaskResult {\n"
        "        task_id: task.id,\n"
        "        output: task.work_data + \" processed\",\n"
        "    })\n"
        "}\n";

    ParseResult result = parse_string(java_migrated_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    // Verify all pool-using functions have proper annotations
    ASTNode *create_func = find_function_declaration(result.ast, "create_executor_service");
    ASSERT_NOT_NULL(create_func);
    ASSERT_TRUE(has_annotation(create_func, "non_deterministic"));

    ASTNode *submit_func = find_function_declaration(result.ast, "submit_task");
    ASSERT_NOT_NULL(submit_func);
    ASSERT_TRUE(has_annotation(submit_func, "non_deterministic"));

    ASTNode *batch_func = find_function_declaration(result.ast, "process_tasks_batch");
    ASSERT_NOT_NULL(batch_func);
    ASSERT_TRUE(has_annotation(batch_func, "non_deterministic"));

    // Helper function shouldn't need annotation
    ASTNode *execute_func = find_function_declaration(result.ast, "execute_task");
    ASSERT_NOT_NULL(execute_func);
    ASSERT_FALSE(has_annotation(execute_func, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_migration_pattern_tests(void) {
    printf("\n--- Migration Pattern Tests ---\n");
    test_go_to_asthra_migration();
    test_java_to_asthra_migration();
}
