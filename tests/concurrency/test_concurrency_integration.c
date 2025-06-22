/**
 * Asthra Programming Language - Concurrency Integration Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Integration tests for the three-tier concurrency system:
 * - Tier progression patterns (1→2)
 * - Real-world usage scenarios
 * - Complex coordination patterns
 * - Performance and scalability patterns
 */

#include "test_advanced_concurrency_common.h"

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

void test_tier_progression_pattern(void) {
    printf("Testing integration: Tier progression pattern...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "// Simple function using only Tier 1 features\n"
        "pub fn simple_parallel() -> Result<(), string> {\n"
        "    let handle1 = spawn_with_handle compute_task(1);\n"
        "    let handle2 = spawn_with_handle compute_task(2);\n"
        "    \n"
        "    let result1 = await handle1;\n"
        "    let result2 = await handle2;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "// Advanced function using Tier 2 features\n"
        "#[non_deterministic]\n"
        "pub fn coordinated_processing() -> Result<(), string> {\n"
        "    let (producer_ch, consumer_ch) = channels.channel_pair<i32>(10)?;\n"
        "    \n"
        "    // Producer task\n"
        "    spawn producer_task(producer_ch);\n"
        "    \n"
        "    // Consumer task\n"
        "    spawn consumer_task(consumer_ch);\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn compute_task(n: i32) -> Result<i32, string> {\n"
        "    Result.Ok(n * 2)\n"
        "}\n"
        "\n"
        "priv fn producer_task(ch: ChannelHandle<i32>) -> Result<(), string> {\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn consumer_task(ch: ChannelHandle<i32>) -> Result<(), string> {\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    // First function should not need annotation
    ASTNode* simple_func = find_function_declaration(result.ast, "simple_parallel");
    ASSERT_NOT_NULL(simple_func);
    ASSERT_FALSE(has_annotation(simple_func, "non_deterministic"));
    
    // Second function should have annotation
    ASTNode* coordinated_func = find_function_declaration(result.ast, "coordinated_processing");
    ASSERT_NOT_NULL(coordinated_func);
    ASSERT_TRUE(has_annotation(coordinated_func, "non_deterministic"));
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_real_world_pattern(void) {
    printf("Testing integration: Real-world usage pattern...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn web_server_pattern(void) -> Result<(), string> {\n"
        "    let request_ch = channels.channel<Request>(100)?;\n"
        "    let response_ch = channels.channel<Response>(100)?;\n"
        "    \n"
        "    // Tier 1: Simple task spawning\n"
        "    spawn listen_for_requests(request_ch);\n"
        "    \n"
        "    // Tier 2: Advanced coordination\n"
        "    let pool = patterns.worker_pool<Request>(4)?;\n"
        "    patterns.submit_task(pool, request_ch)?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv struct Request { id: i32 }\n"
        "priv struct Response { data: string }\n"
        "\n"
        "priv fn listen_for_requests(ch: ChannelHandle<Request>) -> Result<(), string> {\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_producer_consumer_pattern(void) {
    printf("Testing integration: Producer-consumer pattern...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn producer_consumer_demo(void) -> Result<(), string> {\n"
        "    let ch = channels.channel<WorkItem>(50)?;\n"
        "    \n"
        "    // Start multiple producers\n"
        "    spawn producer(ch, 1, 100);\n"
        "    spawn producer(ch, 101, 200);\n"
        "    spawn producer(ch, 201, 300);\n"
        "    \n"
        "    // Start multiple consumers\n"
        "    let consumer1 = spawn_with_handle consumer(ch, \"Consumer1\");\n"
        "    let consumer2 = spawn_with_handle consumer(ch, \"Consumer2\");\n"
        "    \n"
        "    // Wait for consumers to finish\n"
        "    await consumer1;\n"
        "    await consumer2;\n"
        "    \n"
        "    channels.close(ch)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv struct WorkItem {\n"
        "    id: i32,\n"
        "    data: string\n"
        "}\n"
        "\n"
        "priv fn producer(ch: ChannelHandle<WorkItem>, start: i32, end: i32) -> Result<(), string> {\n"
        "    for i in start..end {\n"
        "        let item = WorkItem { id: i, data: format!(\"Item {}\", i) };\n"
        "        channels.send(ch, item)?;\n"
        "    }\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn consumer(ch: ChannelHandle<WorkItem>, name: string) -> Result<(), string> {\n"
        "    loop {\n"
        "        match channels.try_recv(ch)? {\n"
        "            Option.Some(item) => {\n"
        "                // Process item\n"
        "                println!(\"{} processed item {}\", name, item.id);\n"
        "            },\n"
        "            Option.None => break\n"
        "        }\n"
        "    }\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_map_reduce_pattern(void) {
    printf("Testing integration: Map-reduce pattern...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn map_reduce_demo(data: []i32) -> Result<i32, string> {\n"
        "    let map_results = channels.channel<i32>(data.len())?;\n"
        "    \n"
        "    // Map phase: spawn workers for chunks\n"
        "    let chunk_size = data.len() / 4;\n"
        "    for i in 0..4 {\n"
        "        let start = i * chunk_size;\n"
        "        let end = if i == 3 { data.len() } else { (i + 1) * chunk_size };\n"
        "        let chunk = data[start..end];\n"
        "        \n"
        "        spawn map_worker(chunk, map_results);\n"
        "    }\n"
        "    \n"
        "    // Reduce phase: collect and sum results\n"
        "    let reduce_handle = spawn_with_handle reduce_worker(map_results, 4);\n"
        "    let final_result = await reduce_handle;\n"
        "    \n"
        "    final_result\n"
        "}\n"
        "\n"
        "priv fn map_worker(chunk: []i32, results: ChannelHandle<i32>) -> Result<(), string> {\n"
        "    let sum = 0;\n"
        "    for value in chunk {\n"
        "        sum += value * value; // Square each value\n"
        "    }\n"
        "    channels.send(results, sum)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn reduce_worker(results: ChannelHandle<i32>, expected_count: i32) -> Result<i32, string> {\n"
        "    let total = 0;\n"
        "    for _ in 0..expected_count {\n"
        "        let partial_sum = channels.recv(results)?;\n"
        "        total += partial_sum;\n"
        "    }\n"
        "    Result.Ok(total)\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_pipeline_pattern(void) {
    printf("Testing integration: Pipeline processing pattern...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn pipeline_demo() -> Result<(), string> {\n"
        "    // Create pipeline stages\n"
        "    let stage1_out = channels.channel<RawData>(10)?;\n"
        "    let stage2_out = channels.channel<ProcessedData>(10)?;\n"
        "    let stage3_out = channels.channel<FinalData>(10)?;\n"
        "    \n"
        "    // Start pipeline stages\n"
        "    spawn data_generator(stage1_out);\n"
        "    spawn data_processor(stage1_out, stage2_out);\n"
        "    spawn data_finalizer(stage2_out, stage3_out);\n"
        "    \n"
        "    // Output consumer\n"
        "    let consumer_handle = spawn_with_handle output_consumer(stage3_out);\n"
        "    await consumer_handle;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv struct RawData { value: i32 }\n"
        "priv struct ProcessedData { value: i32, processed: bool }\n"
        "priv struct FinalData { value: i32, timestamp: i64 }\n"
        "\n"
        "priv fn data_generator(out: ChannelHandle<RawData>) -> Result<(), string> {\n"
        "    for i in 0..100 {\n"
        "        channels.send(out, RawData { value: i })?;\n"
        "    }\n"
        "    channels.close(out)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn data_processor(input: ChannelHandle<RawData>, output: ChannelHandle<ProcessedData>) -> Result<(), string> {\n"
        "    loop {\n"
        "        match channels.try_recv(input)? {\n"
        "            Some(raw) => {\n"
        "                let processed = ProcessedData { value: raw.value * 2, processed: true };\n"
        "                channels.send(output, processed)?;\n"
        "            },\n"
        "            None => break\n"
        "        }\n"
        "    }\n"
        "    channels.close(output)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn data_finalizer(input: ChannelHandle<ProcessedData>, output: ChannelHandle<FinalData>) -> Result<(), string> {\n"
        "    loop {\n"
        "        match channels.try_recv(input)? {\n"
        "            Option.Some(processed) => {\n"
        "                let final_data = FinalData { value: processed.value, timestamp: get_timestamp() };\n"
        "                channels.send(output, final_data)?;\n"
        "            },\n"
        "            Option.None => break\n"
        "        }\n"
        "    }\n"
        "    channels.close(output)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn output_consumer(input: ChannelHandle<FinalData>) -> Result<(), string> {\n"
        "    loop {\n"
        "        match channels.try_recv(input)? {\n"
        "            Option.Some(final_data) => {\n"
        "                println!(\"Final: {} at {}\", final_data.value, final_data.timestamp);\n"
        "            },\n"
        "            Option.None => break\n"
        "        }\n"
        "    }\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn get_timestamp() -> i64 { 0 }\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_coordination_pattern(void) {
    printf("Testing integration: Complex coordination pattern...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/coordination\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn coordination_demo() -> Result<(), string> {\n"
        "    let barrier = coordination.barrier(3)?;\n"
        "    let semaphore = coordination.semaphore(2)?;\n"
        "    let results_ch = channels.channel<string>(10)?;\n"
        "    \n"
        "    // Start coordinated workers\n"
        "    spawn coordinated_worker(barrier, semaphore, results_ch, \"Worker1\");\n"
        "    spawn coordinated_worker(barrier, semaphore, results_ch, \"Worker2\");\n"
        "    spawn coordinated_worker(barrier, semaphore, results_ch, \"Worker3\");\n"
        "    \n"
        "    // Collect results\n"
        "    let collector_handle = spawn_with_handle result_collector(results_ch, 3);\n"
        "    await collector_handle;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn coordinated_worker(\n"
        "    barrier: BarrierHandle,\n"
        "    semaphore: SemaphoreHandle,\n"
        "    results: ChannelHandle<string>,\n"
        "    name: string\n"
        ") -> Result<(), string> {\n"
        "    // Phase 1: Initialization\n"
        "    coordination.acquire_semaphore(semaphore)?;\n"
        "    \n"
        "    // Do some work\n"
        "    let result = format!(\"{} completed phase 1\", name);\n"
        "    \n"
        "    coordination.release_semaphore(semaphore)?;\n"
        "    \n"
        "    // Synchronization point\n"
        "    coordination.wait_barrier(barrier)?;\n"
        "    \n"
        "    // Phase 2: Coordinated work\n"
        "    channels.send(results, result)?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn result_collector(results: ChannelHandle<string>, count: i32) -> Result<(), string> {\n"
        "    for _ in 0..count {\n"
        "        let result = match channels.recv(results) {\n"
        "            Result.Ok(r) => r,\n"
        "            Result.Err(e) => return Result.Err(e)\n"
        "        };\n"
        "        println!(\"Collected: {}\", result);\n"
        "    }\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_error_propagation_pattern(void) {
    printf("Testing integration: Error propagation across tiers...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "// Tier 1: Error handling with spawn/await\n"
        "pub fn tier1_error_handling() -> Result<i32, string> {\n"
        "    let handle1 = spawn_with_handle fallible_computation(true);\n"
        "    let handle2 = spawn_with_handle fallible_computation(false);\n"
        "    \n"
        "    // Handle errors from concurrent tasks\n"
        "    let result1 = await handle1;\n"
        "    match result1 {\n"
        "        Ok(value) => {\n"
        "            let result2 = await handle2;\n"
        "            match result2 {\n"
        "                Ok(value2) => Result.Ok(value + value2),\n"
        "                Err(e) => Result.Err(e)\n"
        "            }\n"
        "        },\n"
        "        Err(e) => Result.Err(e)\n"
        "    }\n"
        "}\n"
        "\n"
        "// Tier 2: Error handling with channels\n"
        "#[non_deterministic]\n"
        "pub fn tier2_error_handling() -> Result<(), string> {\n"
        "    let results_ch = channels.channel<Result<i32, string>>(10)?;\n"
        "    \n"
        "    // Start workers that may fail\n"
        "    spawn error_prone_worker(results_ch, true);\n"
        "    spawn error_prone_worker(results_ch, false);\n"
        "    \n"
        "    // Collect and handle errors\n"
        "    let collector_handle = spawn_with_handle error_collector(results_ch, 2);\n"
        "    await collector_handle;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn fallible_computation(succeed: bool) -> Result<i32, string> {\n"
        "    if succeed {\n"
        "        Result.Ok(42)\n"
        "    } else {\n"
        "        Result.Err(\"Computation failed\")\n"
        "    }\n"
        "}\n"
        "\n"
        "priv fn error_prone_worker(results: ChannelHandle<Result<i32, string>>, succeed: bool) -> Result<(), string> {\n"
        "    let result = fallible_computation(succeed);\n"
        "    channels.send(results, result)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn error_collector(results: ChannelHandle<Result<i32, string>>, count: i32) -> Result<(), string> {\n"
        "    for _ in 0..count {\n"
        "        let result = match channels.recv(results) {\n"
        "            Result.Ok(r) => r,\n"
        "            Result.Err(e) => return Result.Err(e)\n"
        "        };\n"
        "        match result {\n"
        "            Ok(value) => println!(\"Success: {}\", value),\n"
        "            Err(e) => println!(\"Error: {}\", e)\n"
        "        }\n"
        "    }\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// =============================================================================
// TEST RUNNER FOR INTEGRATION
// =============================================================================

void run_integration_tests(void) {
    printf("🔄 INTEGRATION TESTS\n");
    printf("-------------------\n");
    
    test_tier_progression_pattern();
    test_real_world_pattern();
    test_producer_consumer_pattern();
    test_map_reduce_pattern();
    test_pipeline_pattern();
    test_coordination_pattern();
    test_error_propagation_pattern();
    
    printf("\n");
} 
