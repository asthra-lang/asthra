#include "test_function_calls_common.h"

/**
 * Test instance method call generation (obj.method)
 *
 * This test validates that the code generator properly translates instance
 * method calls like obj.distance() into the correct C function calls like
 * Point_instance_distance() with proper self parameter passing.
 */
AsthraTestResult test_instance_method_calls(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub struct Point { \n"
                         "    pub x: f64,\n"
                         "    pub y: f64\n"
                         "}\n"
                         "\n"
                         "impl Point {\n"
                         "    pub fn new(x: f64, y: f64) -> Point {\n"
                         "        return Point { x: x, y: y };\n"
                         "    }\n"
                         "    pub fn distance(self) -> f64 {\n"
                         "        // Just return a simple calculation for now\n"
                         "        return self.x * self.x + self.y * self.y;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let p: Point = Point { x: 3.0, y: 4.0 };\n"
                         "    let d: f64 = p.distance(none);\n"
                         "    return ();\n"
                         "}";

    FunctionCallTestPipeline pipeline;
    if (!run_test_pipeline(&pipeline, source, context)) {
        cleanup_test_pipeline(&pipeline);
        return ASTHRA_TEST_FAIL;
    }

    // For now, just verify that code generation succeeded
    // A complete test would examine the instruction buffer for the correct call
    bool generation_succeeded = pipeline.success && pipeline.output != NULL;

    if (!ASTHRA_TEST_ASSERT(context, generation_succeeded,
                            "Code generation should complete successfully")) {
        cleanup_test_pipeline(&pipeline);
        return ASTHRA_TEST_FAIL;
    }

    // TODO: Implement proper instruction buffer examination
    // to verify Point_instance_distance call is generated

    cleanup_test_pipeline(&pipeline);
    return ASTHRA_TEST_PASS;
}

// Main test runner for instance method calls
int main(void) {
    printf("=== Instance Method Call Tests ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run instance method call test
    if (test_instance_method_calls(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Instance Method Calls: PASS\n");
        passed++;
    } else {
        printf("❌ Instance Method Calls: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
