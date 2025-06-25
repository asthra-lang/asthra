/*
 * Concurrency Tiers Integration Tests - Real-World Scenarios
 *
 * Tests for real-world applications of the three-tier concurrency system.
 * Includes web servers, data processing pipelines, and production-like
 * workload scenarios.
 *
 * Phase 8: Testing and Validation
 * Focus: Real-world scenarios and production patterns
 */

#include "test_concurrency_tiers_common.h"

// ============================================================================
// Real-World Scenario Tests
// ============================================================================

void test_web_server_scenario(void) {
    printf("Testing web server concurrency scenario...\n");

    const char *web_server_code =
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "// Tier 1: Simple request handling\n"
        "fn handle_request_simple(request: HttpRequest) -> Result<HttpResponse, string> {\n"
        "    let response_handle = spawn_with_handle process_request(request);\n"
        "    let response = await response_handle?;\n"
        "    Result.Ok(response)\n"
        "}\n"
        "\n"
        "// Tier 2: Advanced server with connection pooling\n"
        "#[non_deterministic]\n"
        "fn run_web_server(port: u16) -> Result<(), string> {\n"
        "    let (request_sender, request_receiver) = channels.channel_pair<HttpRequest>(100)?;\n"
        "    let worker_pool = patterns.WorkerPool::<HttpResponse>::new(8)?;\n"
        "    \n"
        "    // Connection acceptor\n"
        "    let acceptor = spawn_with_handle move || {\n"
        "        loop {\n"
        "            match accept_connection(port) {\n"
        "                Result.Ok(request) => {\n"
        "                    match request_sender.send(request) {\n"
        "                        Result.Ok(_) => {},\n"
        "                        Result.Err(_) => break,\n"
        "                    }\n"
        "                },\n"
        "                Result.Err(_) => break,\n"
        "            }\n"
        "        }\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    // Request processor\n"
        "    let processor = spawn_with_handle move || {\n"
        "        loop {\n"
        "            match request_receiver.recv(void) {\n"
        "                channels.RecvResult.Ok(request) => {\n"
        "                    worker_pool.submit_function(move || {\n"
        "                        process_request_advanced(request)\n"
        "                    })?;\n"
        "                },\n"
        "                channels.RecvResult.Closed() => break,\n"
        "                channels.RecvResult.Error(_) => break,\n"
        "                _ => continue,\n"
        "            }\n"
        "        }\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    await acceptor?;\n"
        "    await processor?;\n"
        "    worker_pool.shutdown()?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "struct HttpRequest {\n"
        "    path: string,\n"
        "    method: string,\n"
        "}\n"
        "\n"
        "struct HttpResponse {\n"
        "    status: i32,\n"
        "    body: string,\n"
        "}\n"
        "\n"
        "fn accept_connection(port: u16) -> Result<HttpRequest, string> {\n"
        "    Result.Ok(HttpRequest { path: \"/\", method: \"GET\" })\n"
        "}\n"
        "\n"
        "fn process_request(request: HttpRequest) -> Result<HttpResponse, string> {\n"
        "    Result.Ok(HttpResponse { status: 200, body: \"Hello, World!\" })\n"
        "}\n"
        "\n"
        "fn process_request_advanced(request: HttpRequest) -> Result<HttpResponse, string> {\n"
        "    Result.Ok(HttpResponse { status: 200, body: \"Advanced Response!\" })\n"
        "}\n";

    ParseResult result = parse_string(web_server_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    // Verify tier usage is correct
    ASTNode *simple_handler = find_function_declaration(result.ast, "handle_request_simple");
    ASSERT_NOT_NULL(simple_handler);
    ASSERT_FALSE(has_annotation(simple_handler, "non_deterministic"));

    ASTNode *server_func = find_function_declaration(result.ast, "run_web_server");
    ASSERT_NOT_NULL(server_func);
    ASSERT_TRUE(has_annotation(server_func, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_data_processing_pipeline(void) {
    printf("Testing data processing pipeline scenario...\n");

    const char *pipeline_code =
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/coordination\";\n"
        "\n"
        "// Tier 1: Simple batch processing\n"
        "fn process_batch_simple(data: Vec<DataItem>) -> Result<Vec<ProcessedItem>, string> {\n"
        "    let mut handles = Vec::new();\n"
        "    \n"
        "    for item in data {\n"
        "        let handle = spawn_with_handle transform_item(item);\n"
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
        "// Tier 2: Advanced streaming pipeline\n"
        "#[non_deterministic]\n"
        "fn run_streaming_pipeline() -> Result<(), string> {\n"
        "    let (input_sender, input_receiver) = channels.channel_pair<DataItem>(100)?;\n"
        "    let (output_sender, output_receiver) = channels.channel_pair<ProcessedItem>(100)?;\n"
        "    \n"
        "    // Stage 1: Data ingestion\n"
        "    let ingestion = spawn_with_handle move || {\n"
        "        for i in 0..1000 {\n"
        "            let item = DataItem { id: i, value: i * 2 };\n"
        "            input_sender.send(item)?;\n"
        "        }\n"
        "        input_sender.close()?;\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    // Stage 2: Parallel processing\n"
        "    let processing = spawn_with_handle move || {\n"
        "        loop {\n"
        "            match input_receiver.recv(void) {\n"
        "                channels.RecvResult.Ok(item) => {\n"
        "                    let processed = transform_item_advanced(item)?;\n"
        "                    output_sender.send(processed)?;\n"
        "                },\n"
        "                channels.RecvResult.Closed() => break,\n"
        "                channels.RecvResult.Error(msg) => return Result.Err(msg),\n"
        "                _ => continue,\n"
        "            }\n"
        "        }\n"
        "        output_sender.close()?;\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    // Stage 3: Data aggregation\n"
        "    let aggregation = spawn_with_handle move || {\n"
        "        let mut count = 0;\n"
        "        loop {\n"
        "            match output_receiver.recv(void) {\n"
        "                channels.RecvResult.Ok(_) => count += 1,\n"
        "                channels.RecvResult.Closed() => break,\n"
        "                channels.RecvResult.Error(msg) => return Result.Err(msg),\n"
        "                _ => continue,\n"
        "            }\n"
        "        }\n"
        "        println(\"Processed {} items\", count);\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    await ingestion?;\n"
        "    await processing?;\n"
        "    await aggregation?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "struct DataItem {\n"
        "    id: i32,\n"
        "    value: i32,\n"
        "}\n"
        "\n"
        "struct ProcessedItem {\n"
        "    id: i32,\n"
        "    processed_value: i32,\n"
        "}\n"
        "\n"
        "fn transform_item(item: DataItem) -> Result<ProcessedItem, string> {\n"
        "    Result.Ok(ProcessedItem {\n"
        "        id: item.id,\n"
        "        processed_value: item.value * 2,\n"
        "    })\n"
        "}\n"
        "\n"
        "fn transform_item_advanced(item: DataItem) -> Result<ProcessedItem, string> {\n"
        "    Result.Ok(ProcessedItem {\n"
        "        id: item.id,\n"
        "        processed_value: item.value * 3 + 1,\n"
        "    })\n"
        "}\n";

    ParseResult result = parse_string(pipeline_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    // Verify tier usage
    ASTNode *simple_batch = find_function_declaration(result.ast, "process_batch_simple");
    ASSERT_NOT_NULL(simple_batch);
    ASSERT_FALSE(has_annotation(simple_batch, "non_deterministic"));

    ASTNode *streaming_pipeline = find_function_declaration(result.ast, "run_streaming_pipeline");
    ASSERT_NOT_NULL(streaming_pipeline);
    ASSERT_TRUE(has_annotation(streaming_pipeline, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_real_world_scenario_tests(void) {
    printf("\n--- Real-World Scenario Tests ---\n");
    test_web_server_scenario();
    test_data_processing_pipeline();
}
