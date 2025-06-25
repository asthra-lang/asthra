/*
 * Concurrency Tiers Integration Tests - Error Detection
 *
 * Tests for error detection and edge cases in the three-tier concurrency system.
 * Validates proper enforcement of annotation requirements and error handling.
 *
 * Phase 8: Testing and Validation
 * Focus: Error detection and edge cases
 */

#include "test_concurrency_tiers_common.h"

// ============================================================================
// Error Detection and Edge Cases
// ============================================================================

void test_annotation_missing_detection(void) {
    printf("Testing missing annotation detection in complex scenarios...\n");

    const char *invalid_code = "import \"stdlib/concurrent/channels\";\n"
                               "\n"
                               "// Missing annotation - should fail\n"
                               "fn problematic_function() -> Result<(), string> {\n"
                               "    let (sender, receiver) = channels.channel_pair<i32>(10)?;\n"
                               "    Result.Ok(())\n"
                               "}\n"
                               "\n"
                               "// This one has annotation - should pass\n"
                               "#[non_deterministic]\n"
                               "fn correct_function() -> Result<(), string> {\n"
                               "    let (sender, receiver) = channels.channel_pair<i32>(10)?;\n"
                               "    Result.Ok(())\n"
                               "}\n";

    ParseResult result = parse_string(invalid_code);
    ASSERT_TRUE(result.success); // Should parse fine

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_FALSE(semantic_result.success); // Should fail semantic analysis
    ASSERT_TRUE(semantic_result.error_count > 0);

    // Should have error about missing annotation
    bool found_annotation_error = false;
    for (int i = 0; i < semantic_result.error_count; i++) {
        if (strstr(semantic_result.errors[i].message, "non_deterministic") != NULL ||
            strstr(semantic_result.errors[i].message, "annotation") != NULL) {
            found_annotation_error = true;
            break;
        }
    }
    ASSERT_TRUE(found_annotation_error);

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_nested_tier_violations(void) {
    printf("Testing nested tier violations...\n");

    const char *nested_violation_code =
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "// Tier 1 function trying to call Tier 2 without annotation\n"
        "fn tier1_calling_tier2() -> Result<(), string> {\n"
        "    let (sender, receiver) = channels.channel_pair<i32>(10)?;\n"
        "    let pool = patterns.WorkerPool::<i32>::new(4)?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "// Proper Tier 2 function\n"
        "#[non_deterministic]\n"
        "fn proper_tier2() -> Result<(), string> {\n"
        "    let (sender, receiver) = channels.channel_pair<i32>(10)?;\n"
        "    let pool = patterns.WorkerPool::<i32>::new(4)?;\n"
        "    Result.Ok(())\n"
        "}\n";

    ParseResult result = parse_string(nested_violation_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_FALSE(semantic_result.success); // Should fail due to tier violation
    ASSERT_TRUE(semantic_result.error_count > 0);

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_incorrect_annotation_usage(void) {
    printf("Testing incorrect annotation usage...\n");

    const char *incorrect_annotation_code =
        "// Using annotation on Tier 1 function (unnecessary but allowed)\n"
        "#[non_deterministic]\n"
        "fn simple_spawn() -> Result<(), string> {\n"
        "    let handle = spawn_with_handle compute();\n"
        "    let result = await handle?;\n"
        "    Result.Ok(())\n"
        "}\n"
        "\n"
        "// Helper function\n"
        "fn compute() -> Result<i32, string> {\n"
        "    Result.Ok(42)\n"
        "}\n";

    ParseResult result = parse_string(incorrect_annotation_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    // This should pass - unnecessary annotations are allowed
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_complex_tier_interaction_edge_cases(void) {
    printf("Testing complex tier interaction edge cases...\n");

    const char *complex_edge_case_code =
        "import \"stdlib/concurrent/channels\";\n"
        "import \"stdlib/concurrent/patterns\";\n"
        "\n"
        "// Tier 1 function that properly delegates to Tier 2\n"
        "fn delegating_tier1(data: Vec<i32>) -> Result<Vec<i32>, string> {\n"
        "    // Call a properly annotated Tier 2 function\n"
        "    advanced_processing(data)\n"
        "}\n"
        "\n"
        "// Proper Tier 2 function\n"
        "#[non_deterministic]\n"
        "fn advanced_processing(data: Vec<i32>) -> Result<Vec<i32>, string> {\n"
        "    let (sender, receiver) = channels.channel_pair<i32>(100)?;\n"
        "    \n"
        "    // Process data with channels\n"
        "    for item in data {\n"
        "        sender.send(item * 2)?;\n"
        "    }\n"
        "    sender.close()?;\n"
        "    \n"
        "    let mut results = Vec::new();\n"
        "    loop {\n"
        "        match receiver.recv(void) {\n"
        "            channels.RecvResult.Ok(value) => results.push(value),\n"
        "            channels.RecvResult.Closed() => break,\n"
        "            channels.RecvResult.Error(_) => break,\n"
        "            _ => continue,\n"
        "        }\n"
        "    }\n"
        "    \n"
        "    Result.Ok(results)\n"
        "}\n";

    ParseResult result = parse_string(complex_edge_case_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);
    ASSERT_EQUAL(semantic_result.error_count, 0);

    // Verify delegation pattern is properly handled
    ASTNode *delegating_func = find_function_declaration(result.ast, "delegating_tier1");
    ASSERT_NOT_NULL(delegating_func);
    ASSERT_FALSE(has_annotation(delegating_func, "non_deterministic"));

    ASTNode *advanced_func = find_function_declaration(result.ast, "advanced_processing");
    ASSERT_NOT_NULL(advanced_func);
    ASSERT_TRUE(has_annotation(advanced_func, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_annotation_inheritance_scenarios(void) {
    printf("Testing annotation inheritance scenarios...\n");

    const char *inheritance_code =
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "// Struct with methods that require annotations\n"
        "struct ChannelManager {\n"
        "    capacity: u32,\n"
        "}\n"
        "\n"
        "impl ChannelManager {\n"
        "    fn new(capacity: u32) -> ChannelManager {\n"
        "        ChannelManager { capacity: capacity }\n"
        "    }\n"
        "    \n"
        "    // Method that requires annotation\n"
        "    #[non_deterministic]\n"
        "    fn create_channel(self) -> Result<(channels.Sender<i32>, channels.Receiver<i32>), "
        "string> {\n"
        "        channels.channel_pair<i32>(self.capacity)\n"
        "    }\n"
        "    \n"
        "    // Method without annotation (should fail if it uses channels)\n"
        "    fn invalid_create_channel(self) -> Result<(channels.Sender<i32>, "
        "channels.Receiver<i32>), string> {\n"
        "        channels.channel_pair<i32>(self.capacity)\n"
        "    }\n"
        "}\n";

    ParseResult result = parse_string(inheritance_code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_FALSE(semantic_result.success); // Should fail due to invalid method
    ASSERT_TRUE(semantic_result.error_count > 0);

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_error_detection_tests(void) {
    printf("\n--- Error Detection and Edge Cases ---\n");
    test_annotation_missing_detection();
    test_nested_tier_violations();
    test_incorrect_annotation_usage();
    test_complex_tier_interaction_edge_cases();
    test_annotation_inheritance_scenarios();
}
