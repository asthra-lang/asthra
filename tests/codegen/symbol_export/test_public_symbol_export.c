/**
 * Public Symbol Export Tests
 *
 * This file contains tests specifically focused on verifying that public
 * symbols (marked with 'pub') are properly exported in generated code.
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "symbol_export_test_common.h"

// =============================================================================
// PUBLIC SYMBOL EXPORT TESTS
// =============================================================================

AsthraTestResult test_public_symbol_export(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "pub struct Point { pub x: f64, pub y: f64 }\n"
                         "impl Point {\n"
                         "    pub fn new(x: f64, y: f64) -> Point {\n"
                         "        return Point { x: x, y: y };\n"
                         "    }\n"
                         "    pub fn distance(self) -> f64 {\n"
                         "        return sqrt(self.x * self.x + self.y * self.y);\n"
                         "    }\n"
                         "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check that public symbols are exported
    bool has_public_new_export = has_symbol_export(result.output, "Point_associated_new");
    bool has_public_distance_export = has_symbol_export(result.output, "Point_instance_distance");

    if (!ASTHRA_TEST_ASSERT(context, has_public_new_export,
                            "Generated code should export public associated function")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_public_distance_export,
                            "Generated code should export public instance method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_public_struct_export(AsthraTestContext *context) {
    const char *source =
        "pub struct Vector3 { pub x: f64; pub y: f64; pub z: f64; }\n"
        "impl Vector3 {\n"
        "    pub fn zero() -> Vector3 {\n"
        "        return Vector3 { x: 0.0, y: 0.0, z: 0.0 };\n"
        "    }\n"
        "    pub fn magnitude(self) -> f64 {\n"
        "        return sqrt(self.x * self.x + self.y * self.y + self.z * self.z);\n"
        "    }\n"
        "    pub fn normalize(self) -> Vector3 {\n"
        "        let mag = self.magnitude();\n"
        "        return Vector3 { x: self.x / mag, y: self.y / mag, z: self.z / mag };\n"
        "    }\n"
        "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check that all public methods are exported
    bool has_zero_export = has_symbol_export(result.output, "Vector3_associated_zero");
    bool has_magnitude_export = has_symbol_export(result.output, "Vector3_instance_magnitude");
    bool has_normalize_export = has_symbol_export(result.output, "Vector3_instance_normalize");

    if (!ASTHRA_TEST_ASSERT(context, has_zero_export,
                            "Generated code should export public zero function")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_magnitude_export,
                            "Generated code should export public magnitude method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_normalize_export,
                            "Generated code should export public normalize method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE EXECUTION
// =============================================================================

AsthraTestResult run_public_symbol_export_tests(void) {
    printf("Running public symbol export tests...\n");

    AsthraTestStatistics *stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    AsthraTestMetadata base_metadata = {.name = "Public Symbol Export Tests",
                                        .file = __FILE__,
                                        .line = __LINE__,
                                        .function = __func__,
                                        .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                        .timeout_ns = 5000000000ULL, // 5 seconds
                                        .skip = false,
                                        .skip_reason = NULL};

    struct {
        const char *name;
        AsthraTestResult (*test_func)(AsthraTestContext *);
    } tests[] = {
        {"Public symbol export", test_public_symbol_export},
        {"Public struct export", test_public_struct_export},
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < test_count; i++) {
        AsthraTestMetadata metadata = base_metadata;
        metadata.name = tests[i].name;
        metadata.line = __LINE__;

        AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
        if (!context) {
            printf("❌ Failed to create test context for '%s'\n", tests[i].name);
            overall_result = ASTHRA_TEST_FAIL;
            continue;
        }

        asthra_test_context_start(context);
        AsthraTestResult result = tests[i].test_func(context);
        asthra_test_context_end(context, result);

        if (result == ASTHRA_TEST_PASS) {
            printf("✅ %s: PASS\n", tests[i].name);
        } else {
            printf("❌ %s: FAIL", tests[i].name);
            if (context->error_message) {
                printf(" - %s", context->error_message);
            }
            printf("\n");
            overall_result = ASTHRA_TEST_FAIL;
        }

        asthra_test_context_destroy(context);
    }

    printf("Public symbol export tests completed.\n");
    asthra_test_statistics_destroy(stats);
    return overall_result;
}
