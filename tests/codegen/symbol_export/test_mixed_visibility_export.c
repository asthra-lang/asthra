/**
 * Mixed Visibility Symbol Export Tests
 *
 * This file contains tests specifically focused on verifying that structs
 * with mixed public and private visibility are handled correctly in code generation.
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "symbol_export_test_common.h"

// =============================================================================
// MIXED VISIBILITY EXPORT TESTS
// =============================================================================

AsthraTestResult test_mixed_visibility_export(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "pub struct Point { pub x: f64, priv y: f64 }\n"
                         "impl Point {\n"
                         "    pub fn new(x: f64, y: f64) -> Point {\n"
                         "        return Point { x: x, y: y };\n"
                         "    }\n"
                         "    priv fn private_helper(self) -> f64 {\n"
                         "        return self.x + self.y;\n"
                         "    }\n"
                         "    pub fn sum(self) -> f64 {\n"
                         "        return self.private_helper();\n"
                         "    }\n"
                         "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check public symbols are exported
    bool has_public_new_export = has_symbol_export(result.output, "Point_associated_new");
    bool has_public_sum_export = has_symbol_export(result.output, "Point_instance_sum");

    // Check private symbols are NOT exported
    bool has_private_helper_export =
        has_symbol_export(result.output, "Point_instance_private_helper");

    if (!ASTHRA_TEST_ASSERT(context, has_public_new_export,
                            "Generated code should export public new function")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_public_sum_export,
                            "Generated code should export public sum method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_private_helper_export,
                            "Generated code should NOT export private helper method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_multiple_struct_symbol_export(AsthraTestContext *context) {
    const char *source = "pub struct Point { pub x: f64; pub y: f64; }\n"
                         "impl Point {\n"
                         "    pub fn new(x: f64, y: f64) -> Point { }\n"
                         "}\n"
                         "\n"
                         "struct PrivateRect { width: f64; height: f64; }\n" // Private struct
                         "impl PrivateRect {\n"
                         "    fn new(w: f64, h: f64) -> PrivateRect { }\n" // Private method
                         "}\n"
                         "\n"
                         "pub struct Circle { pub center: Point; pub radius: f64; }\n"
                         "impl Circle {\n"
                         "    pub fn new(center: Point, radius: f64) -> Circle { }\n"
                         "    fn private_area(self) -> f64 { }\n" // Private method
                         "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check public symbols are exported
    bool has_point_export = has_symbol_export(result.output, "Point_associated_new");
    bool has_circle_export = has_symbol_export(result.output, "Circle_associated_new");

    // Check private symbols are NOT exported
    bool has_private_rect_export = has_symbol_export(result.output, "PrivateRect_associated_new");
    bool has_private_area_export = has_symbol_export(result.output, "Circle_instance_private_area");

    if (!ASTHRA_TEST_ASSERT(context, has_point_export,
                            "Generated code should export public Point methods")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_circle_export,
                            "Generated code should export public Circle methods")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_private_rect_export,
                            "Generated code should NOT export private PrivateRect methods")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_private_area_export,
                            "Generated code should NOT export private Circle methods")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_complex_mixed_visibility(AsthraTestContext *context) {
    const char *source = "pub struct Database { pub name: string; connection_string: string; }\n"
                         "impl Database {\n"
                         "    pub fn new(name: string, conn: string) -> Database {\n"
                         "        return Database { name: name, connection_string: conn };\n"
                         "    }\n"
                         "    fn validate_connection(self) -> bool {\n"
                         "        return self.connection_string.length() > 0;\n"
                         "    }\n"
                         "    pub fn connect(self) -> bool {\n"
                         "        return self.validate_connection();\n"
                         "    }\n"
                         "    fn internal_cleanup(self) -> void {\n"
                         "        // Internal cleanup logic\n"
                         "    }\n"
                         "    pub fn disconnect(self) -> void {\n"
                         "        self.internal_cleanup();\n"
                         "    }\n"
                         "}";

    CodegenTestResult result = run_codegen_pipeline(context, source);
    if (!result.success) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    // Check public methods are exported
    bool has_new_export = has_symbol_export(result.output, "Database_associated_new");
    bool has_connect_export = has_symbol_export(result.output, "Database_instance_connect");
    bool has_disconnect_export = has_symbol_export(result.output, "Database_instance_disconnect");

    // Check private methods are NOT exported
    bool has_validate_export =
        has_symbol_export(result.output, "Database_instance_validate_connection");
    bool has_cleanup_export =
        has_symbol_export(result.output, "Database_instance_internal_cleanup");

    if (!ASTHRA_TEST_ASSERT(context, has_new_export,
                            "Generated code should export public new function")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_connect_export,
                            "Generated code should export public connect method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_disconnect_export,
                            "Generated code should export public disconnect method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_validate_export,
                            "Generated code should NOT export private validate method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_cleanup_export,
                            "Generated code should NOT export private cleanup method")) {
        cleanup_codegen_result(&result);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_codegen_result(&result);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE EXECUTION
// =============================================================================

AsthraTestResult run_mixed_visibility_export_tests(void) {
    printf("Running mixed visibility export tests...\n");

    AsthraTestStatistics *stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    AsthraTestMetadata base_metadata = {.name = "Mixed Visibility Export Tests",
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
        {"Mixed visibility export", test_mixed_visibility_export},
        {"Multiple struct symbol export", test_multiple_struct_symbol_export},
        {"Complex mixed visibility", test_complex_mixed_visibility},
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

    printf("Mixed visibility export tests completed.\n");
    asthra_test_statistics_destroy(stats);
    return overall_result;
}
