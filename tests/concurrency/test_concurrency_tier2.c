/**
 * Asthra Programming Language - Tier 2 Concurrency Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for Tier 2 concurrency features (stdlib features requiring annotations):
 * - #[non_deterministic] annotation requirement validation
 * - Stdlib concurrency module import validation
 * - Mixed Tier 1/Tier 2 usage patterns
 * - Advanced coordination primitives
 */

#include "test_advanced_concurrency_common.h"

// =============================================================================
// TIER 2 CONCURRENCY TESTS (Annotation Requirements)
// =============================================================================

void test_tier2_annotation_requirement(void) {
    printf("Testing Tier 2: Annotation requirement for stdlib imports...\n");
    
    const char* no_annotation_code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "pub fn missing_annotation(none) -> Result<(), string> {\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(no_annotation_code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    // Should fail semantic analysis due to missing #[non_deterministic] annotation
    ASSERT_FALSE(semantic_result.success);
    ASSERT_TRUE(semantic_result.error_count > 0);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_annotation_sufficient(void) {
    printf("Testing Tier 2: Annotation sufficient for stdlib usage...\n");
    
    const char* with_annotation_code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn with_annotation(none) -> Result<(), string> {\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(with_annotation_code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    // Should pass semantic analysis with #[non_deterministic] annotation
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_mixed_with_tier1(void) {
    printf("Testing Tier 2: Mixed Tier 1 and Tier 2 usage...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn mixed_tiers(none) -> Result<(), string> {\n"
        "    // Tier 1: Basic spawn (no annotation needed)\n"
        "    let handle = spawn_with_handle compute_data(none);\n"
        "    \n"
        "    // Tier 2: Channel operations (annotation required)\n"
        "    let ch = channels.channel<i32>(5)?;\n"
        "    channels.send(ch, 42)?;\n"
        "    \n"
        "    // Tier 1: Await the spawned task\n"
        "    let result = await handle;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn compute_data(none) -> Result<i32, string> {\n"
        "    Result.Ok(100)\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success); // Should pass with proper annotation
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_channel_operations(void) {
    printf("Testing Tier 2: Advanced channel operations...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn channel_operations(none) -> Result<(), string> {\n"
        "    let (sender, receiver) = channels.channel_pair<i32>(10)?;\n"
        "    \n"
        "    // Send operations\n"
        "    sender.send(42)?;\n"
        "    sender.send(43)?;\n"
        "    \n"
        "    // Receive operations\n"
        "    let value1 = receiver.recv()?;\n"
        "    let value2 = receiver.try_recv()?;\n"
        "    \n"
        "    // Close operations\n"
        "    sender.close()?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_coordination_primitives(void) {
    printf("Testing Tier 2: Coordination primitives...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/coordination\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn coordination_test(none) -> Result<(), string> {\n"
        "    let barrier = coordination.barrier(2)?;\n"
        "    let semaphore = coordination.semaphore(1)?;\n"
        "    \n"
        "    coordination.wait_barrier(barrier)?;\n"
        "    coordination.acquire_semaphore(semaphore)?;\n"
        "    coordination.release_semaphore(semaphore)?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_worker_pool_patterns(void) {
    printf("Testing Tier 2: Worker pool patterns...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn worker_pool_test(none) -> Result<(), string> {\n"
        "    // Create worker pool\n"
        "    let pool = patterns.worker_pool<string>(4)?;\n"
        "    \n"
        "    // Submit tasks\n"
        "    for i in 0..10 {\n"
        "        let task = format!(\"Task {}\", i);\n"
        "        patterns.submit_task(pool, task)?;\n"
        "    }\n"
        "    \n"
        "    // Wait for completion\n"
        "    patterns.wait_completion(pool)?;\n"
        "    \n"
        "    // Shutdown pool\n"
        "    patterns.shutdown_pool(pool)?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_multiple_stdlib_imports(void) {
    printf("Testing Tier 2: Multiple stdlib imports...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/coordination\";\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn comprehensive_concurrency(none) -> Result<(), string> {\n"
        "    // Use all three stdlib modules\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    let barrier = coordination.barrier(2)?;\n"
        "    let pool = patterns.worker_pool<string>(2)?;\n"
        "    \n"
        "    // Coordinate between them\n"
        "    channels.send(ch, 42)?;\n"
        "    coordination.wait_barrier(barrier)?;\n"
        "    patterns.submit_task(pool, \"test\")?;\n"
        "    \n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_annotation_inheritance(void) {
    printf("Testing Tier 2: Annotation inheritance patterns...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn parent_function(none) -> Result<(), string> {\n"
        "    // Parent has annotation, can use Tier 2 features\n"
        "    let ch = channels.channel<i32>(5)?;\n"
        "    helper_function(ch)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "priv fn helper_function(ch: ChannelHandle<i32>) -> Result<(), string> {\n"
        "    // Helper function inherits non-deterministic context\n"
        "    channels.send(ch, 42)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier2_missing_import_error(void) {
    printf("Testing Tier 2: Missing import error detection...\n");
    
    const char* code = 
        "package test;\n"
        "\n"
        "// No import statement\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn missing_import(none) -> Result<(), string> {\n"
        "    // This should fail - no channels import\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);  // Should parse fine
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    // Should fail semantic analysis due to missing import
    ASSERT_FALSE(semantic_result.success);
    ASSERT_TRUE(semantic_result.error_count > 0);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// =============================================================================
// TEST RUNNER FOR TIER 2
// =============================================================================

void run_tier2_tests(void) {
    printf("⚠️ TIER 2 CONCURRENCY TESTS (Annotation Requirements)\n");
    printf("------------------------------------------------------\n");
    
    test_tier2_annotation_requirement();
    test_tier2_annotation_sufficient();
    test_tier2_mixed_with_tier1();
    test_tier2_channel_operations();
    test_tier2_coordination_primitives();
    test_tier2_worker_pool_patterns();
    test_tier2_multiple_stdlib_imports();
    test_tier2_annotation_inheritance();
    test_tier2_missing_import_error();
    
    printf("\n");
} 
