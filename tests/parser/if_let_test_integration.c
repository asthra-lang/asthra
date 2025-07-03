/**
 * Asthra Programming Language
 * If-Let Statement Testing - Integration and Performance Tests Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 4.4: Integration tests and Phase 4.5: Performance and edge case tests
 */

#include "if_let_test_integration.h"

// =============================================================================
// PHASE 4.4: INTEGRATION TESTS IMPLEMENTATION
// =============================================================================

bool test_end_to_end_compilation_mock(void) {
    const char *source = "fn process_optional(opt: Option<i32>) -> i32 {\n"
                         "    if let Option.Some(value) = opt {\n"
                         "        return value * 2;\n"
                         "    } else {\n"
                         "        return 0;\n"
                         "    }\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    // Parse the function (which contains if-let)
    ASTNode *function = parse_function_decl(parser);
    bool success = (function != NULL && function->type == AST_FUNCTION_DECL);

    if (function)
        ast_free_node(function);
    destroy_test_parser(parser);
    return success;
}

bool test_grammar_compliance(void) {
    // Test cases that should parse according to the grammar:
    // IfLetStmt <- 'if' 'let' Pattern '=' Expr Block ('else' Block)?
    // Note: Struct patterns have been removed from the language

    const char *valid_cases[] = {
        "if let x = compute() { }",                           // Simple identifier pattern
        "if let Option.Some(x) = get_option() { }",           // Enum pattern
        "if let Option.Some(x) = maybe_value() { } else { }", // With else clause
        NULL};

    bool all_parsed = true;

    for (int i = 0; valid_cases[i] != NULL; i++) {
        Parser *parser = create_test_parser(valid_cases[i]);
        if (!parser) {
            all_parsed = false;
            continue;
        }

        ASTNode *stmt = parse_if_stmt(parser);
        if (!stmt || stmt->type != AST_IF_LET_STMT) {
            all_parsed = false;
        }

        if (stmt)
            ast_free_node(stmt);
        destroy_test_parser(parser);
    }

    return all_parsed;
}

// =============================================================================
// PHASE 4.5: PERFORMANCE AND EDGE CASE TESTS IMPLEMENTATION
// =============================================================================

bool test_complex_nested_patterns(void) {
    const char *source = "if let Option.Some(user_data) = get_complex_data() {\n"
                         "    process_user(user_data);\n"
                         "} else {\n"
                         "    handle_complex_error();\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!parser)
        return false;

    ASTNode *if_let_stmt = parse_if_stmt(parser);
    bool success = (if_let_stmt != NULL && if_let_stmt->type == AST_IF_LET_STMT);

    if (if_let_stmt)
        ast_free_node(if_let_stmt);
    destroy_test_parser(parser);
    return success;
}

bool test_memory_management(void) {
    const char *source = "if let Option.Some(value) = get_value() {\n"
                         "    use_value(value);\n"
                         "}";

    // Parse and free multiple times to check for memory issues
    for (int i = 0; i < 10; i++) {
        Parser *parser = create_test_parser(source);
        if (!parser)
            return false;

        ASTNode *if_let_stmt = parse_if_stmt(parser);
        if (!if_let_stmt || if_let_stmt->type != AST_IF_LET_STMT) {
            if (if_let_stmt)
                ast_free_node(if_let_stmt);
            destroy_test_parser(parser);
            return false;
        }

        ast_free_node(if_let_stmt);
        destroy_test_parser(parser);
    }

    return true;
}

bool run_integration_tests(void) {
    TEST_SECTION("Phase 4.4: Integration Tests");
    TEST_ASSERT(test_end_to_end_compilation_mock(), "End-to-end compilation (mock)");
    TEST_ASSERT(test_grammar_compliance(), "Grammar compliance validation");

    return (tests_failed == 0);
}

bool run_performance_tests(void) {
    TEST_SECTION("Phase 4.5: Performance and Edge Case Tests");
    TEST_ASSERT(test_complex_nested_patterns(), "Complex nested patterns");
    TEST_ASSERT(test_memory_management(), "Memory management validation");

    return (tests_failed == 0);
}
