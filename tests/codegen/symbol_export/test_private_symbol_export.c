/**
 * Private Symbol Export Tests
 *
 * This file contains tests specifically focused on verifying that private
 * symbols (without 'pub') are NOT exported in generated code.
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "symbol_export_test_common.h"

// =============================================================================
// PRIVATE SYMBOL EXPORT TESTS
// =============================================================================

AsthraTestResult test_private_symbol_not_exported(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "priv struct Point { priv x: f64, priv y: f64 }\n" // Private struct
                         "impl Point {\n"
                         "    priv fn new(x: f64, y: f64) -> Point {\n" // Private method
                         "        return Point { x: x, y: y };\n"
                         "    }\n"
                         "    priv fn private_helper(self) -> f64 {\n" // Private method
                         "        return self.x + self.y;\n"
                         "    }\n"
                         "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check that private symbols are NOT exported
    bool has_private_new_export = has_symbol_export(result.output, "Point_associated_new");
    bool has_private_helper_export =
        has_symbol_export(result.output, "Point_instance_private_helper");

    if (!ASTHRA_TEST_ASSERT(context, !has_private_new_export,
                            "Generated code should NOT export private associated function")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_private_helper_export,
                            "Generated code should NOT export private instance method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_private_struct_not_exported(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "priv struct InternalData { priv value: i32, priv secret: f64 }\n"
                         "impl InternalData {\n"
                         "    priv fn create(val: i32, sec: f64) -> InternalData {\n"
                         "        return InternalData { value: val, secret: sec };\n"
                         "    }\n"
                         "    priv fn get_value(self) -> i32 {\n"
                         "        return self.value;\n"
                         "    }\n"
                         "    priv fn internal_compute(self) -> f64 {\n"
                         "        return self.secret * 2.0;\n"
                         "    }\n"
                         "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check that all private methods are NOT exported
    bool has_create_export = has_symbol_export(result.output, "InternalData_associated_create");
    bool has_get_value_export = has_symbol_export(result.output, "InternalData_instance_get_value");
    bool has_compute_export =
        has_symbol_export(result.output, "InternalData_instance_internal_compute");

    if (!ASTHRA_TEST_ASSERT(context, !has_create_export,
                            "Generated code should NOT export private create function")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_get_value_export,
                            "Generated code should NOT export private get_value method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_compute_export,
                            "Generated code should NOT export private compute method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE EXECUTION
// =============================================================================

AsthraTestResult run_private_symbol_export_tests(void) {
    printf("Running private symbol export tests...\n");

    AsthraTestStatistics *stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    AsthraTestMetadata base_metadata = {.name = "Private Symbol Export Tests",
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
        {"Private symbol not exported", test_private_symbol_not_exported},
        {"Private struct not exported", test_private_struct_not_exported},
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

    printf("Private symbol export tests completed.\n");
    asthra_test_statistics_destroy(stats);
    return overall_result;
}
