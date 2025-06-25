/**
 * Main Orchestrator for pub/impl/self Parser Tests
 *
 * This module coordinates execution of all pub/impl/self parser test modules
 * and provides comprehensive reporting and statistics.
 *
 * Test Modules:
 * - Basic parsing tests (fundamental functionality)
 * - Advanced parsing tests (complex scenarios)
 * - Postfix expression ambiguity tests (v1.20 grammar fix)
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "test_pub_impl_self_common.h"
#include <sys/wait.h>
#include <unistd.h>

// =============================================================================
// TEST MODULE EXECUTION
// =============================================================================

typedef struct {
    const char *name;
    const char *executable;
    const char *description;
} TestModule;

static TestModule test_modules[] = {{"Basic Parser Tests", "./test_basic_pub_impl_self",
                                     "Fundamental pub/impl/self parsing functionality"},
                                    {"Advanced Parser Tests", "./test_advanced_pub_impl_self",
                                     "Complex scenarios and nested structures"},
                                    {"Postfix Expression Tests",
                                     "./test_postfix_expression_ambiguity",
                                     "v1.20 grammar fix for :: operator ambiguity"}};

static const size_t module_count = sizeof(test_modules) / sizeof(test_modules[0]);

static int run_test_module(const TestModule *module) {
    printf("🔄 Running %s...\n", module->name);
    printf("   Description: %s\n", module->description);
    printf("   Executable: %s\n\n", module->executable);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process - execute the test module
        execl(module->executable, module->executable, (char *)NULL);
        // If execl returns, there was an error
        printf("❌ Failed to execute %s\n", module->executable);
        exit(1);
    } else if (pid > 0) {
        // Parent process - wait for child to complete
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code == 0) {
                printf("✅ %s: PASSED\n\n", module->name);
                return 0;
            } else {
                printf("❌ %s: FAILED (exit code: %d)\n\n", module->name, exit_code);
                return 1;
            }
        } else {
            printf("❌ %s: TERMINATED ABNORMALLY\n\n", module->name);
            return 1;
        }
    } else {
        printf("❌ Failed to fork process for %s\n\n", module->name);
        return 1;
    }
}

// =============================================================================
// COMPREHENSIVE TEST SUITE EXECUTION
// =============================================================================

static AsthraTestResult run_comprehensive_test_suite(void) {
    printf("pub/impl/self Parser Test Suite\n");
    printf("===============================\n\n");

    printf("Running comprehensive test suite with %zu modules:\n\n", module_count);

    int passed_modules = 0;
    int failed_modules = 0;

    for (size_t i = 0; i < module_count; i++) {
        int result = run_test_module(&test_modules[i]);
        if (result == 0) {
            passed_modules++;
        } else {
            failed_modules++;
        }
    }

    printf("=== Comprehensive Test Suite Summary ===\n");
    printf("Total modules: %zu\n", module_count);
    printf("Passed modules: %d\n", passed_modules);
    printf("Failed modules: %d\n", failed_modules);
    printf("Success rate: %.1f%%\n", (double)passed_modules / module_count * 100.0);

    if (failed_modules == 0) {
        printf(
            "\n🎉 ALL MODULES PASSED! pub/impl/self parser functionality is working correctly.\n");
        return ASTHRA_TEST_PASS;
    } else {
        printf("\n❌ %d module(s) failed. Please check individual module output above.\n",
               failed_modules);
        return ASTHRA_TEST_FAIL;
    }
}

// =============================================================================
// SMOKE TEST EXECUTION
// =============================================================================

static AsthraTestResult run_smoke_test(void) {
    printf("pub/impl/self Parser Smoke Test\n");
    printf("===============================\n\n");

    // Quick smoke test to verify basic functionality
    const char *smoke_test_source = "pub struct Point { pub x: f64, pub y: f64 }\n"
                                    "impl Point {\n"
                                    "    pub fn new(x: f64, y: f64) -> Point {\n"
                                    "        return Point { x: x, y: y };\n"
                                    "    }\n"
                                    "    pub fn distance(self) -> f64 {\n"
                                    "        return sqrt(self.x * self.x + self.y * self.y);\n"
                                    "    }\n"
                                    "}\n"
                                    "let point = Point::new(3.0, 4.0);\n"
                                    "let dist = point.distance(void);";

    Parser *parser = create_test_parser(smoke_test_source);

    if (!parser) {
        printf("❌ Smoke test failed: Could not create parser\n");
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *program = parser_parse_program(parser);

    if (!program) {
        printf("❌ Smoke test failed: Could not parse program\n");
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (program->type != AST_PROGRAM) {
        printf("❌ Smoke test failed: Expected AST_PROGRAM, got %d\n", program->type);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    printf("✅ Smoke test passed: Basic pub/impl/self functionality working\n\n");

    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(int argc, char *argv[]) {
    printf("Asthra pub/impl/self Parser Test Suite\n");
    printf("======================================\n\n");

    // Check for command line arguments
    bool run_smoke_only = false;
    bool run_comprehensive = true;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--smoke") == 0) {
            run_smoke_only = true;
            run_comprehensive = false;
        } else if (strcmp(argv[i], "--comprehensive") == 0) {
            run_comprehensive = true;
            run_smoke_only = false;
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --smoke          Run smoke test only\n");
            printf("  --comprehensive  Run comprehensive test suite (default)\n");
            printf("  --help           Show this help message\n");
            return 0;
        }
    }

    AsthraTestResult result = ASTHRA_TEST_PASS;

    if (run_smoke_only) {
        result = run_smoke_test();
    } else if (run_comprehensive) {
        // Run smoke test first
        AsthraTestResult smoke_result = run_smoke_test();
        if (smoke_result == ASTHRA_TEST_FAIL) {
            printf("❌ Smoke test failed - skipping comprehensive tests\n");
            return 1;
        }

        // Run comprehensive test suite
        result = run_comprehensive_test_suite();
    }

    printf("\n");
    if (result == ASTHRA_TEST_PASS) {
        printf("🎉 pub/impl/self parser test suite completed successfully!\n");
        return 0;
    } else {
        printf("❌ pub/impl/self parser test suite failed!\n");
        return 1;
    }
}
