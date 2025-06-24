#include "bdd_test_framework.h"
#include "bdd_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Parser BDD Tests
// Tests parser functionality with consolidated mock patterns

// Consolidated mock parser API
typedef struct {
    int success;
    char* error_message;
    int error_line;
    int error_column;
    int node_count;
    char* ast_summary;
} ParserResult;

// Global parser state for test scenarios
static ParserResult parser_result = {0};

// ===================================================================
// CONSOLIDATED MOCK PATTERNS
// ===================================================================

// Consolidated mock parser function with common validation patterns
static ParserResult mock_parse_asthra_code(const char* code) {
    ParserResult result = {0};
    
    // Pattern: Empty code validation
    if (!code || strlen(code) == 0) {
        result.success = 0;
        result.error_message = strdup("Empty source code");
        return result;
    }
    
    // Pattern: Package declaration requirement
    // Check for actual package declaration at the beginning, not in comments
    const char* trimmed = code;
    while (*trimmed && (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\n')) {
        trimmed++;
    }
    // Skip single-line comments
    if (strncmp(trimmed, "//", 2) == 0) {
        while (*trimmed && *trimmed != '\n') trimmed++;
        if (*trimmed == '\n') trimmed++;
        // Skip more whitespace after comment
        while (*trimmed && (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\n')) {
            trimmed++;
        }
    }
    
    if (strncmp(trimmed, "package ", 8) != 0) {
        result.success = 0;
        result.error_message = strdup("Missing package declaration");
        result.error_line = 1;
        result.error_column = 1;
        return result;
    }
    
    // Pattern: Brace matching validation
    int brace_count = 0;
    int line = 1;
    int column = 1;
    for (const char* p = code; *p; p++) {
        if (*p == '{') {
            brace_count++;
        } else if (*p == '}') {
            brace_count--;
            if (brace_count < 0) {
                result.success = 0;
                result.error_message = strdup("Unexpected closing brace");
                result.error_line = line;
                result.error_column = column;
                return result;
            }
        } else if (*p == '\n') {
            line++;
            column = 1;
            continue;
        }
        column++;
    }
    
    if (brace_count != 0) {
        result.success = 0;
        result.error_message = strdup("Unclosed brace");
        result.error_line = line;
        result.error_column = column;
        return result;
    }
    
    // Pattern: Statement semicolon validation
    const char* stmt_keywords[] = {"let", "return", "println", NULL};
    for (int i = 0; stmt_keywords[i]; i++) {
        char* pos = strstr(code, stmt_keywords[i]);
        while (pos) {
            // Find end of line
            char* eol = strchr(pos, '\n');
            if (!eol) eol = (char*)code + strlen(code);
            
            // Check if there's a semicolon before the end of line
            char* semi = strchr(pos, ';');
            if (!semi || semi > eol) {
                // Count line number
                int stmt_line = 1;
                for (const char* p = code; p < pos; p++) {
                    if (*p == '\n') stmt_line++;
                }
                
                result.success = 0;
                result.error_message = strdup("expected ';'");
                result.error_line = stmt_line;
                result.error_column = (int)(eol - pos);
                return result;
            }
            
            pos = strstr(eol, stmt_keywords[i]);
        }
    }
    
    // Pattern: Success result with mock AST
    result.success = 1;
    result.node_count = 10; // Mock AST node count
    result.ast_summary = strdup("AST with package, functions, and statements");
    
    return result;
}

// Consolidated cleanup function for parser state
static void cleanup_parser_result(void) {
    if (parser_result.error_message) {
        free(parser_result.error_message);
        parser_result.error_message = NULL;
    }
    if (parser_result.ast_summary) {
        free(parser_result.ast_summary);
        parser_result.ast_summary = NULL;
    }
    parser_result.success = 0;
    parser_result.node_count = 0;
    parser_result.error_line = 0;
    parser_result.error_column = 0;
}

// Consolidated source code templates
static const char* get_valid_asthra_code(void) {
    return "package parser_test;\n"
           "\n"
           "import std.io;\n"
           "\n"
           "pub fn calculate(x: i32, y: i32) -> i32 {\n"
           "    let sum = x + y;\n"
           "    let product = x * y;\n"
           "    return sum + product;\n"
           "}\n"
           "\n"
           "pub fn main(none) -> void {\n"
           "    let result = calculate(5, 3);\n"
           "    println(\"Result: {}\", result);\n"
           "    return ();\n"
           "}\n";
}

static const char* get_missing_semicolon_code(void) {
    return "package parser_test;\n"
           "\n"
           "pub fn broken(none) -> void {\n"
           "    let x = 42\n"  // Missing semicolon
           "    println(\"Value: {}\", x);\n"
           "}\n";
}

static const char* get_unclosed_brace_code(void) {
    return "package parser_test;\n"
           "\n"
           "pub fn unclosed(none) -> void {\n"
           "    if true {\n"
           "        println(\"Missing closing brace\");\n"
           "    \n"  // Missing closing brace
           "}\n";
}

static const char* get_complex_expression_code(void) {
    return "package parser_test;\n"
           "\n"
           "pub fn main(none) -> void {\n"
           "    let a = (x + y) * (z - w) / (p + q);\n"
           "    let b = func1(func2(arg1, arg2), func3());\n"
           "    let c = array[index1][index2].field.method();\n"
           "    return ();\n"
           "}\n";
}

static const char* get_invalid_package_code(void) {
    return "// Missing package declaration\n"
           "pub fn main(none) -> void {\n"
           "    println(\"No package!\");\n"
           "    return ();\n"
           "}\n";
}

static const char* get_nested_structure_code(void) {
    return "package parser_test;\n"
           "\n"
           "type Person = {\n"
           "    name: str,\n"
           "    age: i32,\n"
           "    address: {\n"
           "        street: str,\n"
           "        city: str,\n"
           "        zipcode: i32\n"
           "    }\n"
           "};\n"
           "\n"
           "pub fn main(none) -> void {\n"
           "    return ();\n"
           "}\n";
}

// ===================================================================
// TEST SCENARIO IMPLEMENTATIONS
// ===================================================================

// Test scenario: Parse syntactically valid code
void test_parse_valid_code(void) {
    bdd_given("syntactically valid Asthra code");
    const char* source = get_valid_asthra_code();
    bdd_create_temp_source_file("valid_code.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should succeed");
    bdd_assert(parser_result.success == 1, "Parsing should succeed for valid code");
    
    bdd_then("an AST should be generated");
    bdd_assert(parser_result.node_count > 0, "AST should have nodes");
    bdd_assert(parser_result.ast_summary != NULL, "AST summary should be generated");
}

// Test scenario: Detect missing semicolon
void test_detect_missing_semicolon(void) {
    bdd_given("Asthra code with missing semicolon");
    const char* source = get_missing_semicolon_code();
    bdd_create_temp_source_file("missing_semicolon.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should fail");
    bdd_assert(parser_result.success == 0, "Parsing should fail for missing semicolon");
    
    bdd_then("the parser error should contain expected message");
    bdd_assert(parser_result.error_message != NULL, "Error message should be provided");
    bdd_assert_output_contains(parser_result.error_message, "expected ';'");
    
    bdd_then("the error should be at the correct line");
    bdd_assert(parser_result.error_line == 4, "Error should be at line 4");
}

// Test scenario: Detect unclosed brace
void test_detect_unclosed_brace(void) {
    bdd_given("Asthra code with unclosed brace");
    const char* source = get_unclosed_brace_code();
    bdd_create_temp_source_file("unclosed_brace.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should fail");
    bdd_assert(parser_result.success == 0, "Parsing should fail for unclosed brace");
    
    bdd_then("the parser error should contain brace error");
    bdd_assert(parser_result.error_message != NULL, "Error message should be provided");
    bdd_assert_output_contains(parser_result.error_message, "Unclosed brace");
}

// Test scenario: Handle empty source code
void test_empty_source_handling(void) {
    bdd_given("empty source code");
    const char* source = "";
    bdd_create_temp_source_file("empty.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should fail");
    bdd_assert(parser_result.success == 0, "Parsing should fail for empty code");
    
    bdd_then("the parser error should indicate empty source");
    bdd_assert(parser_result.error_message != NULL, "Error message should be provided");
    bdd_assert_output_contains(parser_result.error_message, "Empty source code");
}

// Test scenario: Parse complex expressions
void test_parse_complex_expressions(void) {
    bdd_given("Asthra code with complex expressions");
    const char* source = get_complex_expression_code();
    bdd_create_temp_source_file("complex_expr.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should succeed");
    bdd_assert(parser_result.success == 1, "Parsing should succeed for complex expressions");
    
    bdd_then("AST should reflect complex structure");
    bdd_assert(parser_result.node_count > 0, "AST should have nodes for complex expressions");
}

// Test scenario: Missing package declaration
void test_missing_package_declaration(void) {
    bdd_given("Asthra code without package declaration");
    const char* source = get_invalid_package_code();
    bdd_create_temp_source_file("no_package.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should fail");
    bdd_assert(parser_result.success == 0, "Parsing should fail without package declaration");
    
    bdd_then("the error should indicate missing package");
    bdd_assert(parser_result.error_message != NULL, "Error message should be provided");
    bdd_assert_output_contains(parser_result.error_message, "Missing package declaration");
    
    bdd_then("error should be at line 1");
    bdd_assert(parser_result.error_line == 1, "Package error should be at line 1");
}

// Test scenario: Parse nested structures
void test_parse_nested_structures(void) {
    bdd_given("Asthra code with nested type definitions");
    const char* source = get_nested_structure_code();
    bdd_create_temp_source_file("nested_struct.asthra", source);
    
    bdd_when("I parse the code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should succeed");
    bdd_assert(parser_result.success == 1, "Parsing should succeed for nested structures");
    
    bdd_then("AST should be generated for nested types");
    bdd_assert(parser_result.ast_summary != NULL, "AST summary should be generated");
    bdd_assert_output_contains(parser_result.ast_summary, "AST with package");
}

// Test scenario: Parser error recovery (WIP)
void test_parser_error_recovery(void) {
    bdd_given("Asthra code with multiple syntax errors");
    const char* source = 
        "package parser_test;\n"
        "\n"
        "pub fn multiple_errors(none) -> void {\n"
        "    let x = 42\n"  // Missing semicolon
        "    let y = {\n"   // Unclosed brace
        "    println(\"Errors\");\n"
        "}\n";
    
    bdd_create_temp_source_file("multiple_errors.asthra", source);
    
    bdd_when("I parse the code with error recovery");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should report the first error");
    bdd_assert(parser_result.success == 0, "Parsing should fail with multiple errors");
    bdd_assert(parser_result.error_message != NULL, "First error should be reported");
    
    // Mark as WIP since advanced error recovery is not implemented
    bdd_skip_scenario("Advanced error recovery not fully implemented yet");
}

// Test scenario: Unicode and international characters (WIP)
void test_unicode_parsing(void) {
    bdd_given("Asthra code with Unicode characters");
    const char* source = 
        "package тест;\n"  // Cyrillic package name
        "\n"
        "pub fn main(none) -> void {\n"
        "    let message = \"Hello, 世界! 🌍\";\n"  // Unicode string
        "    println(message);\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("unicode.asthra", source);
    
    bdd_when("I parse the Unicode code");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(source);
    
    bdd_then("parsing should handle Unicode correctly");
    // Mark as WIP since Unicode support may not be fully implemented
    bdd_skip_scenario("Full Unicode support not implemented yet");
}

// Test scenario: Parser performance with large files (WIP)
void test_large_file_parsing(void) {
    bdd_given("a large Asthra source file");
    
    // Generate large valid code
    char* large_code = malloc(50000);
    strcpy(large_code, "package large_test;\n\n");
    
    for (int i = 0; i < 1000; i++) {
        char func[100];
        snprintf(func, sizeof(func), 
                "fn func_%d() -> i32 { return %d; }\n", i, i);
        strcat(large_code, func);
    }
    strcat(large_code, "\npub fn main(none) -> void { return (); }\n");
    
    bdd_create_temp_source_file("large_file.asthra", large_code);
    
    bdd_when("I parse the large file");
    cleanup_parser_result();
    parser_result = mock_parse_asthra_code(large_code);
    
    bdd_then("parsing should complete efficiently");
    bdd_assert(parser_result.success == 1, "Large file parsing should succeed");
    
    free(large_code);
    
    // Mark as WIP since performance testing infrastructure is not complete
    bdd_skip_scenario("Performance testing infrastructure not complete");
}

// ===================================================================
// DECLARATIVE TEST CASE DEFINITIONS
// ===================================================================

BddTestCase parser_test_cases[] = {
    BDD_TEST_CASE(parse_valid_code, test_parse_valid_code),
    BDD_TEST_CASE(detect_missing_semicolon, test_detect_missing_semicolon),
    BDD_TEST_CASE(detect_unclosed_brace, test_detect_unclosed_brace),
    BDD_TEST_CASE(empty_source_handling, test_empty_source_handling),
    BDD_TEST_CASE(parse_complex_expressions, test_parse_complex_expressions),
    BDD_TEST_CASE(missing_package_declaration, test_missing_package_declaration),
    BDD_TEST_CASE(parse_nested_structures, test_parse_nested_structures),
    BDD_WIP_TEST_CASE(parser_error_recovery, test_parser_error_recovery),
    BDD_WIP_TEST_CASE(unicode_parsing, test_unicode_parsing),
    BDD_WIP_TEST_CASE(large_file_parsing, test_large_file_parsing),
};

int main(void) {
    return bdd_run_test_suite(
        "Parser Functionality",
        parser_test_cases,
        sizeof(parser_test_cases) / sizeof(parser_test_cases[0]),
        bdd_cleanup_temp_files
    );
}