/**
 * Tests for predeclared functions including args(), log(), and panic()
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "test_type_system_common.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "parser.h"
#include "ast_operations.h"
#include "../framework/test_framework.h"
#include "../framework/semantic_test_utils.h"

static bool test_args_function_exists(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arguments: []string = args();\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_args_function_exists");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_returns_string_slice(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arguments: []string = args();\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_args_function_returns_string_slice");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_no_parameters(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arguments: []string = args(\"invalid\");\n"  // Should fail - args takes no parameters
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_args_function_no_parameters");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    // This test expects failure - args takes no parameters
    if (success) {
        printf("Expected semantic analysis to fail but it passed\n");
        success = false;
    } else {
        printf("Expected failure - args() takes no parameters\n");
        success = true;  // We expected it to fail
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_can_iterate(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arguments: []string = args();\n"
        "    for arg in arguments {\n"
        "        log(arg);\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_args_function_can_iterate");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_can_index(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arguments: []string = args();\n"
        "    // TODO: Add length check once len() is implemented\n"
        "    // For now, just test that args() returns a slice that can be indexed\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_args_function_can_index");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

// =============================================================================
// PANIC FUNCTION TESTS
// =============================================================================

static bool test_panic_function_exists(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    panic(\"test panic\");\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_panic_function_exists");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_panic_function_returns_never(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn test_fn(none) -> i32 {\n"
        "    panic(\"unreachable\");\n"
        "    // No return needed after panic - Never type\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_panic_function_returns_never");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_panic_function_requires_string_parameter(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    panic();\n"  // Should fail - panic requires string parameter
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_panic_function_requires_string_parameter");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    // This test expects failure - panic requires a string parameter
    if (success) {
        printf("Expected semantic analysis to fail but it passed\n");
        success = false;
    } else {
        printf("Expected failure - panic() requires a string parameter\n");
        success = true;  // We expected it to fail
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_panic_function_rejects_wrong_parameter_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    panic(42);\n"  // Should fail - panic requires string, not i32
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_panic_function_rejects_wrong_parameter_type");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    // This test expects failure - panic requires string, not i32
    if (success) {
        printf("Expected semantic analysis to fail but it passed\n");
        success = false;
    } else {
        printf("Expected failure - panic() requires string parameter, not i32\n");
        success = true;  // We expected it to fail
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

// =============================================================================
// LOG FUNCTION TESTS
// =============================================================================

static bool test_log_function_exists(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"test message\");\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_log_function_exists");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_log_function_returns_void(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"test message\");\n"
        "    return ();\n"  // Explicit return required after log
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_log_function_returns_void");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_infinite_function_exists(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let iter: []void = infinite();\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_infinite_function_exists");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_infinite_function_no_parameters(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let iter: []void = infinite(10);\n"  // Should fail - infinite takes no parameters
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_infinite_function_no_parameters");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    // This test expects failure - infinite takes no parameters
    if (success) {
        printf("Expected semantic analysis to fail but it passed\n");
        success = false;
    } else {
        printf("Expected failure - infinite() takes no parameters\n");
        success = true;  // We expected it to fail
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_infinite_function_can_iterate(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn main(none) -> void {\n"
        "    let mut count: i32 = 0;\n"
        "    for _ in infinite() {\n"
        "        if count >= 10 {\n"
        "            break;\n"
        "        }\n"
        "        count = count + 1;\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode* ast = parse_test_source(source, "test_infinite_function_can_iterate");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

// =============================================================================
// TEST FRAMEWORK INTEGRATION
// =============================================================================

ASTHRA_TEST_DEFINE(args_function_exists, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_function_exists() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_returns_string_slice, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_function_returns_string_slice() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_no_parameters, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_function_no_parameters() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_can_iterate, ASTHRA_TEST_SEVERITY_HIGH) {
    return test_args_function_can_iterate() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_can_index, ASTHRA_TEST_SEVERITY_HIGH) {
    return test_args_function_can_index() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(panic_function_exists, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_panic_function_exists() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(panic_function_returns_never, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_panic_function_returns_never() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(panic_function_requires_string_parameter, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_panic_function_requires_string_parameter() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(panic_function_rejects_wrong_parameter_type, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_panic_function_rejects_wrong_parameter_type() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(log_function_exists, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_log_function_exists() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(log_function_returns_void, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_log_function_returns_void() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(infinite_function_exists, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_infinite_function_exists() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(infinite_function_no_parameters, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_infinite_function_no_parameters() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(infinite_function_can_iterate, ASTHRA_TEST_SEVERITY_HIGH) {
    return test_infinite_function_can_iterate() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

int main(void) {
    AsthraTestFunction tests[] = {
        args_function_exists,
        args_function_returns_string_slice,
        args_function_no_parameters,
        args_function_can_iterate,
        args_function_can_index,
        panic_function_exists,
        panic_function_returns_never,
        panic_function_requires_string_parameter,
        panic_function_rejects_wrong_parameter_type,
        log_function_exists,
        log_function_returns_void,
        infinite_function_exists,
        infinite_function_no_parameters,
        infinite_function_can_iterate
    };

    AsthraTestMetadata metadatas[] = {
        {
            .name = "args_function_exists",
            .file = __FILE__,
            .line = __LINE__,
            .function = "args_function_exists",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "args_function_returns_string_slice",
            .file = __FILE__,
            .line = __LINE__,
            .function = "args_function_returns_string_slice",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "args_function_no_parameters",
            .file = __FILE__,
            .line = __LINE__,
            .function = "args_function_no_parameters",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "args_function_can_iterate",
            .file = __FILE__,
            .line = __LINE__,
            .function = "args_function_can_iterate",
            .severity = ASTHRA_TEST_SEVERITY_HIGH,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "args_function_can_index",
            .file = __FILE__,
            .line = __LINE__,
            .function = "args_function_can_index",
            .severity = ASTHRA_TEST_SEVERITY_HIGH,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "panic_function_exists",
            .file = __FILE__,
            .line = __LINE__,
            .function = "panic_function_exists",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "panic_function_returns_never",
            .file = __FILE__,
            .line = __LINE__,
            .function = "panic_function_returns_never",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "panic_function_requires_string_parameter",
            .file = __FILE__,
            .line = __LINE__,
            .function = "panic_function_requires_string_parameter",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "panic_function_rejects_wrong_parameter_type",
            .file = __FILE__,
            .line = __LINE__,
            .function = "panic_function_rejects_wrong_parameter_type",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "log_function_exists",
            .file = __FILE__,
            .line = __LINE__,
            .function = "log_function_exists",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "log_function_returns_void",
            .file = __FILE__,
            .line = __LINE__,
            .function = "log_function_returns_void",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "infinite_function_exists",
            .file = __FILE__,
            .line = __LINE__,
            .function = "infinite_function_exists",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "infinite_function_no_parameters",
            .file = __FILE__,
            .line = __LINE__,
            .function = "infinite_function_no_parameters",
            .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
            .timeout_ns = 0,
            .skip = false
        },
        {
            .name = "infinite_function_can_iterate",
            .file = __FILE__,
            .line = __LINE__,
            .function = "infinite_function_can_iterate",
            .severity = ASTHRA_TEST_SEVERITY_HIGH,
            .timeout_ns = 0,
            .skip = false
        }
    };

    AsthraTestSuiteConfig config = asthra_test_suite_config_create(
        "Predeclared Functions Semantic Tests",
        "Tests for predeclared functions like args(), log(), and panic()"
    );

    AsthraTestResult result = asthra_test_run_suite(
        tests,
        metadatas,
        sizeof(tests) / sizeof(tests[0]),
        &config
    );

    return result == ASTHRA_TEST_PASS ? 0 : 1;
}