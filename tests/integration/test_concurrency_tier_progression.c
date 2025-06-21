/*
 * Concurrency Tiers Integration Tests - Tier Progression
 * 
 * Tests for progression patterns from Tier 1 (deterministic) to Tier 2 (non-deterministic)
 * concurrency features. Validates the transition from simple spawn/await to advanced
 * channel-based communication patterns.
 * 
 * Phase 8: Testing and Validation
 * Focus: Tier interactions and progression patterns
 */

#include "test_concurrency_tiers_common.h"

// ============================================================================
// Tier 1 → Tier 2 Progression Tests
// ============================================================================

void test_simple_to_advanced_progression(void) {
    printf("Testing simple to advanced progression pattern...\n");
    
    // Tier 1: Simple parallel processing
    const char* tier1_code = 
        "fn simple_parallel(data: Vec<i32>) -> Result<Vec<i32>, string> {\n"
        "    let mut handles = Vec::new();\n"
        "    \n"
        "    for item in data {\n"
        "        let handle = spawn_with_handle double_value(item);\n"
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
        "fn double_value(x: i32) -> Result<i32, string> {\n"
        "    Result.Ok(x * 2)\n"
        "}\n";
    
    ParseResult tier1_result = parse_string(tier1_code);
    ASSERT_TRUE(tier1_result.success);
    
    SemanticAnalysisResult tier1_semantic = analyze_semantics(tier1_result.ast);
    ASSERT_TRUE(tier1_semantic.success);
    ASSERT_EQUAL(tier1_semantic.error_count, 0);
    
    // Verify it's recognized as Tier 1 (deterministic)
    ASTNode* simple_func = find_function_declaration(tier1_result.ast, "simple_parallel");
    ASSERT_NOT_NULL(simple_func);
    ASSERT_FALSE(has_annotation(simple_func, "non_deterministic"));
    
    cleanup_parse_result(&tier1_result);
    cleanup_semantic_result(&tier1_semantic);
    
    // Tier 2: Advanced with channels
    const char* tier2_code = 
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "fn advanced_parallel(data: Vec<i32>) -> Result<Vec<i32>, string> {\n"
        "    let (sender, receiver) = channels.channel_pair<i32>(10)?;\n"
        "    \n"
        "    // Producer\n"
        "    let producer = spawn_with_handle move || {\n"
        "        for item in data {\n"
        "            sender.send(item * 2)?;\n"
        "        }\n"
        "        sender.close()?;\n"
        "        Result.Ok(())\n"
        "    };\n"
        "    \n"
        "    // Consumer\n"
        "    let consumer = spawn_with_handle move || {\n"
        "        let mut results = Vec::new();\n"
        "        loop {\n"
        "            match receiver.recv(void) {\n"
        "                channels.RecvResult.Ok(value) => results.push(value),\n"
        "                channels.RecvResult.Closed() => break,\n"
        "                channels.RecvResult.Error(msg) => return Result.Err(msg),\n"
        "                _ => continue,\n"
        "            }\n"
        "        }\n"
        "        Result.Ok(results)\n"
        "    };\n"
        "    \n"
        "    await producer?;\n"
        "    let results = await consumer?;\n"
        "    Result.Ok(results)\n"
        "}\n";
    
    ParseResult tier2_result = parse_string(tier2_code);
    ASSERT_TRUE(tier2_result.success);
    
    SemanticAnalysisResult tier2_semantic = analyze_semantics(tier2_result.ast);
    ASSERT_TRUE(tier2_semantic.success);
    ASSERT_EQUAL(tier2_semantic.error_count, 0);
    
    // Verify it's recognized as Tier 2 (requires annotation)
    ASTNode* advanced_func = find_function_declaration(tier2_result.ast, "advanced_parallel");
    ASSERT_NOT_NULL(advanced_func);
    ASSERT_TRUE(has_annotation(advanced_func, "non_deterministic"));
    
    cleanup_parse_result(&tier2_result);
    cleanup_semantic_result(&tier2_semantic);
}

void test_mixed_tier_usage(void) {
    printf("Testing mixed tier usage in single program...\n");
    
    const char* mixed_code = 
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "// Tier 1 function\n"
        "fn basic_processing(data: Vec<i32>) -> Result<Vec<i32>, string> {\n"
        "    let mut handles = Vec::new();\n"
        "    \n"
        "    for item in data {\n"
        "        let handle = spawn_with_handle process_item(item);\n"
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
        "// Tier 2 function that calls Tier 1\n"
        "#[non_deterministic]\n"
        "fn advanced_processing(data: Vec<i32>) -> Result<Vec<i32>, string> {\n"
        "    // First do basic processing\n"
        "    let basic_results = basic_processing(data)?;\n"
        "    \n"
        "    // Then use advanced patterns\n"
        "    let pool = patterns.WorkerPool::<i32>::new(4)?;\n"
        "    \n"
        "    for item in basic_results {\n"
        "        pool.submit_function(move || {\n"
        "            advanced_process_item(item)\n"
        "        })?;\n"
        "    }\n"
        "    \n"
        "    let mut final_results = Vec::new();\n"
        "    for _ in 0..basic_results.len(void) {\n"
        "        final_results.push(pool.get_result()" "?" "?" ");\n"
        "    }\n"
        "    \n"
        "    pool.shutdown()?;\n"
        "    Result.Ok(final_results)\n"
        "}\n"
        "\n"
        "fn process_item(x: i32) -> Result<i32, string> {\n"
        "    Result.Ok(x * 2)\n"
        "}\n"
        "\n"
        "fn advanced_process_item(x: i32) -> Result<i32, string> {\n"
        "    Result.Ok(x + 1)\n"
        "}\n";
    
    ParseResult result = parse_string(mixed_code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);
    
    // Verify tier classifications
    ASTNode* basic_func = find_function_declaration(result.ast, "basic_processing");
    ASSERT_NOT_NULL(basic_func);
    ASSERT_FALSE(has_annotation(basic_func, "non_deterministic"));
    
    ASTNode* advanced_func = find_function_declaration(result.ast, "advanced_processing");
    ASSERT_NOT_NULL(advanced_func);
    ASSERT_TRUE(has_annotation(advanced_func, "non_deterministic"));
    
    ASTNode* helper1 = find_function_declaration(result.ast, "process_item");
    ASSERT_NOT_NULL(helper1);
    ASSERT_FALSE(has_annotation(helper1, "non_deterministic"));
    
    ASTNode* helper2 = find_function_declaration(result.ast, "advanced_process_item");
    ASSERT_NOT_NULL(helper2);
    ASSERT_FALSE(has_annotation(helper2, "non_deterministic"));
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_tier_progression_tests(void) {
    printf("--- Tier 1 → Tier 2 Progression Tests ---\n");
    test_simple_to_advanced_progression();
    test_mixed_tier_usage();
} 
