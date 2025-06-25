/*
 * Annotation Tests - Performance and Edge Cases
 * Tests for annotation performance and edge case handling
 *
 * Covers:
 * - Functions with many annotations
 * - Annotations with complex parameters
 * - Performance edge cases
 * - Memory usage validation
 */

#include "test_annotations_common.h"

// ============================================================================
// Performance and Edge Cases
// ============================================================================

void test_many_annotations(void) {
    printf("Testing functions with many annotations...\n");

    const char *code = "#[non_deterministic]\n"
                       "#[inline]\n"
                       "#[doc(\"Legacy function - consider using new implementation\")]\n"
                       "#[test]\n"
                       "#[cold]\n"
                       "#[must_use]\n"
                       "#[cfg(feature = \"advanced\")]\n"
                       "#[doc(\"Complex function with many annotations\")]\n"
                       "fn heavily_annotated_function() -> Result<(), string> {\n"
                       "    Result.Ok(())\n"
                       "}\n";

    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    ASSERT_NOT_NULL(result.ast);

    ASTNode *function_node = find_function_declaration(result.ast, "heavily_annotated_function");
    ASSERT_NOT_NULL(function_node);
    ASSERT_TRUE(function_node->annotation_count >= 8);
    ASSERT_TRUE(has_annotation(function_node, "non_deterministic"));

    cleanup_parse_result(&result);
}

void test_annotation_with_complex_parameters(void) {
    printf("Testing annotations with complex parameters...\n");

    const char *code = "#[non_deterministic]\n"
                       "#[retry(max_attempts = 5, delays = [100, 200, 400, 800, 1600])]\n"
                       "#[circuit_breaker(failure_threshold = 0.5, recovery_timeout = 30000)]\n"
                       "fn complex_annotated_function() -> Result<string, string> {\n"
                       "    Result.Ok(\"success\")\n"
                       "}\n";

    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    ASSERT_NOT_NULL(result.ast);

    ASTNode *function_node = find_function_declaration(result.ast, "complex_annotated_function");
    ASSERT_NOT_NULL(function_node);
    ASSERT_TRUE(has_annotation(function_node, "non_deterministic"));

    // Verify complex parameters are parsed
    for (int i = 0; i < function_node->annotation_count; i++) {
        if (strcmp(function_node->annotations[i].name, "retry") == 0) {
            ASSERT_NOT_NULL(function_node->annotations[i].value);
            ASSERT_TRUE(strstr(function_node->annotations[i].value, "max_attempts") != NULL);
            ASSERT_TRUE(strstr(function_node->annotations[i].value, "delays") != NULL);
        }
    }

    cleanup_parse_result(&result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_performance_annotation_tests(void) {
    printf("--- Performance and Edge Cases ---\n");

    test_many_annotations();
    test_annotation_with_complex_parameters();

    printf("Performance and edge case tests completed.\n");
}
