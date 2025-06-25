/*
 * Annotation Tests - Error Detection
 * Tests for annotation error detection and validation
 *
 * Covers:
 * - Invalid annotation syntax detection
 * - Unknown annotation handling
 * - Annotation placement restrictions
 * - Parser error recovery
 */

#include "test_annotations_common.h"

// ============================================================================
// Error Detection Tests
// ============================================================================

void test_invalid_annotation_syntax(void) {
    printf("Testing invalid annotation syntax detection...\n");

    const char *invalid_code1 = "[non_deterministic]  // Missing #\n"
                                "fn invalid_syntax1(void) {}\n";

    ParseResult result1 = parse_string(invalid_code1);
    ASSERT_FALSE(result1.success); // Should fail to parse
    cleanup_parse_result(&result1);

    const char *invalid_code2 = "#non_deterministic  // Missing brackets\n"
                                "fn invalid_syntax2(void) {}\n";

    ParseResult result2 = parse_string(invalid_code2);
    ASSERT_FALSE(result2.success); // Should fail to parse
    cleanup_parse_result(&result2);

    const char *invalid_code3 = "#[non-deterministic]  // Invalid hyphen\n"
                                "fn invalid_syntax3(void) {}\n";

    ParseResult result3 = parse_string(invalid_code3);
    ASSERT_FALSE(result3.success); // Should fail to parse
    cleanup_parse_result(&result3);
}

void test_unknown_annotation_handling(void) {
    printf("Testing unknown annotation handling...\n");

    const char *code_with_unknown = "#[unknown_annotation]\n"
                                    "#[non_deterministic]\n"
                                    "fn function_with_unknown() -> Result<(), string> {\n"
                                    "    Result.Ok(())\n"
                                    "}\n";

    ParseResult result = parse_string(code_with_unknown);
    ASSERT_TRUE(result.success); // Should parse successfully

    SemanticAnalysisResult semantic_result = analyze_semantics(result.ast);
    // Behavior depends on implementation: might warn or ignore unknown annotations
    // But should not fail due to known non_deterministic annotation

    ASTNode *function_node = find_function_declaration(result.ast, "function_with_unknown");
    ASSERT_NOT_NULL(function_node);
    ASSERT_TRUE(has_annotation(function_node, "non_deterministic"));

    cleanup_parse_result(&result);
    cleanup_semantic_result(&semantic_result);
}

void test_annotation_placement_restrictions(void) {
    printf("Testing annotation placement restrictions...\n");

    // Annotations should only be valid on certain constructs
    const char *invalid_placement =
        "fn test_function(void) {\n"
        "    #[non_deterministic]  // Invalid - cannot annotate statements\n"
        "    let x = 42;\n"
        "}\n";

    ParseResult result = parse_string(invalid_placement);
    ASSERT_FALSE(result.success); // Should fail - annotations only on declarations
    cleanup_parse_result(&result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_error_annotation_tests(void) {
    printf("--- Error Detection Tests ---\n");

    test_invalid_annotation_syntax();
    test_unknown_annotation_handling();
    test_annotation_placement_restrictions();

    printf("Error detection tests completed.\n");
}
