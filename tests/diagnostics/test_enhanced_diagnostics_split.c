/**
 * Test for split enhanced diagnostics modules
 */

#include "../../src/diagnostics/enhanced_diagnostics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Basic standalone unit test framework
static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

#define TEST_CASE(name)                                                                            \
    int name(void);                                                                                \
    int name(void)

#define RUN_TEST(test_func)                                                                        \
    do {                                                                                           \
        test_count++;                                                                              \
        printf("Running %s... ", #test_func);                                                      \
        fflush(stdout);                                                                            \
        if (test_func()) {                                                                         \
            printf("PASS\n");                                                                      \
            test_passed++;                                                                         \
        } else {                                                                                   \
            printf("FAIL\n");                                                                      \
            test_failed++;                                                                         \
        }                                                                                          \
    } while (0)

#define ASSERT_TRUE(condition)                                                                     \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            printf("ASSERTION FAILED: %s at line %d\n", #condition, __LINE__);                     \
            return 0;                                                                              \
        }                                                                                          \
    } while (0)

#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_STREQ(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)

TEST_CASE(test_enhanced_diagnostics_split_works) {
    // Test creating a basic diagnostic
    EnhancedDiagnostic *diagnostic = enhanced_diagnostic_create("E001", DIAGNOSTIC_ERROR, 
                                                               "Test error message");
    ASSERT_NOT_NULL(diagnostic);
    ASSERT_STREQ(diagnostic->code, "E001");
    ASSERT_EQ(diagnostic->level, DIAGNOSTIC_ERROR);
    ASSERT_STREQ(diagnostic->message, "Test error message");

    // Test adding a span
    DiagnosticSpan span = {
        .start_line = 10,
        .start_column = 5,
        .end_line = 10,
        .end_column = 15,
        .file_path = "test.asthra",
        .label = "here",
        .snippet = NULL
    };
    bool result = enhanced_diagnostic_add_span(diagnostic, &span);
    ASSERT_TRUE(result);
    ASSERT_EQ(diagnostic->span_count, 1);

    // Test adding a suggestion
    DiagnosticSuggestion suggestion = {
        .type = SUGGESTION_REPLACE,
        .span = span,
        .text = "fixed_text",
        .confidence = CONFIDENCE_HIGH,
        .rationale = "This is the correct syntax"
    };
    result = enhanced_diagnostic_add_suggestion(diagnostic, &suggestion);
    ASSERT_TRUE(result);
    ASSERT_EQ(diagnostic->suggestion_count, 1);

    // Test JSON serialization
    char *json = enhanced_diagnostic_to_json(diagnostic);
    ASSERT_NOT_NULL(json);
    ASSERT_TRUE(strstr(json, "\"code\": \"E001\"") != NULL);
    ASSERT_TRUE(strstr(json, "\"level\": \"error\"") != NULL);
    ASSERT_TRUE(strstr(json, "\"message\": \"Test error message\"") != NULL);
    ASSERT_TRUE(strstr(json, "\"spans\":") != NULL);
    ASSERT_TRUE(strstr(json, "\"suggestions\":") != NULL);

    free(json);
    enhanced_diagnostic_destroy(diagnostic);
    
    return 1; // Test passed
}

int main(void) {
    printf("=== Enhanced Diagnostics Split Test ===\n\n");
    
    RUN_TEST(test_enhanced_diagnostics_split_works);
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);
    
    if (test_failed == 0) {
        printf("\n✅ All enhanced diagnostics split tests passed!\n");
    } else {
        printf("\n❌ Some tests failed.\n");
    }
    
    return test_failed > 0 ? 1 : 0;
}