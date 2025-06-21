/*
 * Annotation Tests - Validation
 * Tests for annotation semantic validation and enforcement
 * 
 * Covers:
 * - #[non_deterministic] requirement for Tier 2 features
 * - Annotation sufficiency for Tier 2 operations
 * - Tier 1 features not requiring annotations
 * - Semantic analysis integration
 */

#include "test_annotations_common.h"

// ============================================================================
// Annotation Validation Tests
// ============================================================================

void test_non_deterministic_required_for_tier2(void) {
    printf("Testing #[non_deterministic] requirement for Tier 2 features...\n");
    
    // Code using Tier 2 features without annotation should fail validation
    const char* code_without_annotation = 
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "fn missing_annotation() -> Result<(), string> {\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    channels.send(ch, 42)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code_without_annotation);
    ASSERT_TRUE(result.success);  // Should parse fine
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_FALSE(semantic_result.success);  // Should fail semantic analysis
    ASSERT_TRUE(semantic_result.error_count > 0);
    
    // Should have error mentioning missing non_deterministic annotation
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

void test_non_deterministic_sufficient_for_tier2(void) {
    printf("Testing #[non_deterministic] sufficient for Tier 2 features...\n");
    
    // Code using Tier 2 features WITH annotation should pass validation
    const char* code_with_annotation = 
        "import \"stdlib/concurrent/channels\";\n"
        "\n"
        "#[non_deterministic]\n"
        "fn correct_annotation() -> Result<(), string> {\n"
        "    let ch = channels.channel<i32>(10)?;\n"
        "    channels.send(ch, 42)?;\n"
        "    let value = channels.recv(ch)?;\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code_with_annotation);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);  // Should pass with annotation
    ASSERT_EQUAL(semantic_result.error_count, 0);
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_tier1_no_annotation_required(void) {
    printf("Testing Tier 1 features don't require annotation...\n");
    
    const char* tier1_code = 
        "fn tier1_function() -> Result<i32, string> {\n"
        "    let handle1 = spawn_with_handle compute(10);\n"
        "    let handle2 = spawn_with_handle compute(20);\n"
        "    \n"
        "    let result1 = await handle1?;\n"
        "    let result2 = await handle2?;\n"
        "    \n"
        "    Result.Ok(result1 + result2)\n"
        "}\n"
        "\n"
        "fn compute(n: i32) -> Result<i32, string> {\n"
        "    Result.Ok(n * n)\n"
        "}\n";
    
    ParseResult result = parse_string(tier1_code);
    ASSERT_TRUE(result.success);
    
    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    ASSERT_TRUE(semantic_result.success);  // Should pass without annotation
    ASSERT_EQUAL(semantic_result.error_count, 0);
    
    // Verify functions don't have non_deterministic annotation
    ASTNode* tier1_func = find_function_declaration(result.ast, "tier1_function");
    ASSERT_NOT_NULL(tier1_func);
    ASSERT_FALSE(has_annotation(tier1_func, "non_deterministic"));
    
    ASTNode* compute_func = find_function_declaration(result.ast, "compute");
    ASSERT_NOT_NULL(compute_func);
    ASSERT_FALSE(has_annotation(compute_func, "non_deterministic"));
    
    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_validation_annotation_tests(void) {
    printf("--- Annotation Validation Tests ---\n");
    
    test_non_deterministic_required_for_tier2();
    test_non_deterministic_sufficient_for_tier2();
    test_tier1_no_annotation_required();
    
    printf("Annotation validation tests completed.\n");
} 
