/*
 * Annotation Tests - Complex Scenarios
 * Tests for advanced annotation usage patterns
 *
 * Covers:
 * - Nested function call annotation propagation
 * - Mixed Tier 1/Tier 2 function interactions
 * - Complex annotation inheritance patterns
 * - Multi-level function call validation
 */

#include "test_annotations_common.h"

// ============================================================================
// Complex Annotation Scenarios
// ============================================================================

void test_nested_function_annotations(void) {
    printf("Testing nested function call annotation propagation...\n");

    const char *code = "import \"stdlib/concurrent/coordination\";\n"
                       "\n"
                       "#[non_deterministic]\n"
                       "fn coordinator() -> Result<(), string> {\n"
                       "    // Calls another Tier 2 function\n"
                       "    select_coordinator()?;\n"
                       "    Result.Ok(())\n"
                       "}\n"
                       "\n"
                       "#[non_deterministic]\n"
                       "fn select_coordinator() -> Result<(), string> {\n"
                       "    coordination.select([\n"
                       "        coordination.timeout_case(1000, || handle_timeout())\n"
                       "    ])?;\n"
                       "    Result.Ok(())\n"
                       "}\n"
                       "\n"
                       "fn handle_timeout() -> Result<(), string> {\n"
                       "    Result.Ok(())\n"
                       "}\n";

    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);

    // Both coordinator functions should have annotations
    ASTNode *coordinator = find_function_declaration(result.ast, "coordinator");
    ASSERT_NOT_NULL(coordinator);
    ASSERT_TRUE(has_annotation(coordinator, "non_deterministic"));

    ASTNode *select_coordinator = find_function_declaration(result.ast, "select_coordinator");
    ASSERT_NOT_NULL(select_coordinator);
    ASSERT_TRUE(has_annotation(select_coordinator, "non_deterministic"));

    // Helper function doesn't need annotation
    ASTNode *handle_timeout = find_function_declaration(result.ast, "handle_timeout");
    ASSERT_NOT_NULL(handle_timeout);
    ASSERT_FALSE(has_annotation(handle_timeout, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_mixed_tier_function_calls(void) {
    printf("Testing mixed Tier 1/Tier 2 function calls...\n");

    const char *code = "import \"stdlib/concurrent/patterns\";\n"
                       "\n"
                       "#[non_deterministic]\n"
                       "fn mixed_processing() -> Result<Vec<i32>, string> {\n"
                       "    // Tier 1: Basic parallel processing\n"
                       "    let basic_handle = spawn_with_handle basic_computation();\n"
                       "    \n"
                       "    // Tier 2: Advanced worker pool\n"
                       "    let pool = patterns.WorkerPool::<i32>::new(4)?;\n"
                       "    \n"
                       "    for i in 0..10 {\n"
                       "        pool.submit_function(move || advanced_computation(i))?;\n"
                       "    }\n"
                       "    \n"
                       "    // Wait for basic result\n"
                       "    let basic_result = await basic_handle?;\n"
                       "    \n"
                       "    // Collect advanced results\n"
                       "    let mut advanced_results = Vec::new();\n"
                       "    for _ in 0..10 {\n"
                       "        advanced_results.push(pool.get_result()??);\n"
                       "    }\n"
                       "    \n"
                       "    pool.shutdown()?;\n"
                       "    \n"
                       "    advanced_results.push(basic_result);\n"
                       "    Result.Ok(advanced_results)\n"
                       "}\n"
                       "\n"
                       "fn basic_computation() -> Result<i32, string> {\n"
                       "    // Pure Tier 1 function\n"
                       "    Result.Ok(42)\n"
                       "}\n"
                       "\n"
                       "fn advanced_computation(n: i32) -> Result<i32, string> {\n"
                       "    // Called from Tier 2 context but itself is Tier 1\n"
                       "    Result.Ok(n * n + 1)\n"
                       "}\n";

    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);

    // Main function needs annotation due to Tier 2 usage
    ASTNode *mixed_func = find_function_declaration(result.ast, "mixed_processing");
    ASSERT_NOT_NULL(mixed_func);
    ASSERT_TRUE(has_annotation(mixed_func, "non_deterministic"));

    // Helper functions don't need annotations
    ASTNode *basic_func = find_function_declaration(result.ast, "basic_computation");
    ASSERT_NOT_NULL(basic_func);
    ASSERT_FALSE(has_annotation(basic_func, "non_deterministic"));

    ASTNode *advanced_func = find_function_declaration(result.ast, "advanced_computation");
    ASSERT_NOT_NULL(advanced_func);
    ASSERT_FALSE(has_annotation(advanced_func, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_complex_annotation_tests(void) {
    printf("--- Complex Annotation Scenarios ---\n");

    test_nested_function_annotations();
    test_mixed_tier_function_calls();

    printf("Complex annotation scenario tests completed.\n");
}
