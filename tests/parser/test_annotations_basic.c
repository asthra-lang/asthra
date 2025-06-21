/*
 * Annotation Tests - Basic Parsing
 * Tests for fundamental annotation parsing functionality
 * 
 * Covers:
 * - Single #[non_deterministic] annotation parsing
 * - Multiple annotations on functions
 * - Annotations with parameters and values
 * - Basic annotation structure validation
 */

#include "test_annotations_common.h"

// ============================================================================
// Basic Annotation Parsing Tests
// ============================================================================

void test_non_deterministic_annotation_parsing(void) {
    printf("Testing #[non_deterministic] annotation parsing...\n");
    
    const char* code = 
        "#[non_deterministic]\n"
        "fn annotated_function() -> Result<(), string> {\n"
        "    Result.Ok(())\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    ASSERT_NOT_NULL(result.ast);
    
    ASTNode* function_node = find_function_declaration(result.ast, "annotated_function");
    ASSERT_NOT_NULL(function_node);
    ASSERT_TRUE(function_node->annotation_count > 0);
    
    bool has_non_deterministic = false;
    for (int i = 0; i < function_node->annotation_count; i++) {
        if (strcmp(function_node->annotations[i].name, "non_deterministic") == 0) {
            has_non_deterministic = true;
            break;
        }
    }
    ASSERT_TRUE(has_non_deterministic);
    
    cleanup_parse_result(&result);
}

void test_multiple_annotations(void) {
    printf("Testing multiple annotations on function...\n");
    
    const char* code = 
        "#[non_deterministic]\n"
        "#[inline]\n"
        "#[deprecated(\"Use new_function instead\")]\n"
        "fn multi_annotated_function() -> Result<i32, string> {\n"
        "    Result.Ok(42)\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    ASSERT_NOT_NULL(result.ast);
    
    ASTNode* function_node = find_function_declaration(result.ast, "multi_annotated_function");
    ASSERT_NOT_NULL(function_node);
    ASSERT_EQUAL(function_node->annotation_count, 3);
    
    // Check that all annotations are parsed correctly
    bool found_non_deterministic = false;
    bool found_inline = false;
    bool found_deprecated = false;
    
    for (int i = 0; i < function_node->annotation_count; i++) {
        if (strcmp(function_node->annotations[i].name, "non_deterministic") == 0) {
            found_non_deterministic = true;
        } else if (strcmp(function_node->annotations[i].name, "inline") == 0) {
            found_inline = true;
        } else if (strcmp(function_node->annotations[i].name, "deprecated") == 0) {
            found_deprecated = true;
            ASSERT_STR_EQUAL(function_node->annotations[i].value, "Use new_function instead");
        }
    }
    
    ASSERT_TRUE(found_non_deterministic);
    ASSERT_TRUE(found_inline);
    ASSERT_TRUE(found_deprecated);
    
    cleanup_parse_result(&result);
}

void test_annotation_with_parameters(void) {
    printf("Testing annotations with parameters...\n");
    
    const char* code = 
        "#[non_deterministic]\n"
        "#[timeout(5000)]\n"
        "#[retry(max_attempts = 3, backoff = \"exponential\")]\n"
        "fn parameterized_function() -> Result<string, string> {\n"
        "    Result.Ok(\"success\")\n"
        "}\n";
    
    ParseResult result = parse_string(code);
    ASSERT_TRUE(result.success);
    ASSERT_NOT_NULL(result.ast);
    
    ASTNode* function_node = find_function_declaration(result.ast, "parameterized_function");
    ASSERT_NOT_NULL(function_node);
    ASSERT_TRUE(function_node->annotation_count >= 3);
    
    // Verify annotations with parameters are parsed correctly
    for (int i = 0; i < function_node->annotation_count; i++) {
        if (strcmp(function_node->annotations[i].name, "timeout") == 0) {
            ASSERT_STR_EQUAL(function_node->annotations[i].value, "5000");
        } else if (strcmp(function_node->annotations[i].name, "retry") == 0) {
            ASSERT_NOT_NULL(function_node->annotations[i].value);
            // Should contain parameter string
            ASSERT_TRUE(strstr(function_node->annotations[i].value, "max_attempts") != NULL);
        }
    }
    
    cleanup_parse_result(&result);
}

// ============================================================================
// Module Test Runner
// ============================================================================

void run_basic_annotation_tests(void) {
    printf("--- Basic Annotation Parsing Tests ---\n");
    
    test_non_deterministic_annotation_parsing();
    test_multiple_annotations();
    test_annotation_with_parameters();
    
    printf("Basic annotation parsing tests completed.\n");
} 
