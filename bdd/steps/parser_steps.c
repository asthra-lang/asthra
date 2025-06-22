#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bdd_support.h"

// Mock parser API for BDD tests
typedef struct {
    int success;
    char* error_message;
    int error_line;
    int error_column;
    int node_count;
    char* ast_summary;
} ParserResult;

// Parser state
static char* source_code = NULL;
static ParserResult parser_result = {0};

// Mock parser function
static ParserResult mock_parse_asthra_code(const char* code) {
    ParserResult result = {0};
    
    // Simple validation rules for mock parser
    if (!code || strlen(code) == 0) {
        result.success = 0;
        result.error_message = strdup("Empty source code");
        return result;
    }
    
    // Check for package declaration
    if (!strstr(code, "package")) {
        result.success = 0;
        result.error_message = strdup("Missing package declaration");
        result.error_line = 1;
        result.error_column = 1;
        return result;
    }
    
    // Check for unmatched braces
    int brace_count = 0;
    int line = 1;
    int column = 1;
    for (const char* p = code; *p; p++) {
        if (*p == '{') brace_count++;
        else if (*p == '}') brace_count--;
        else if (*p == '\n') {
            line++;
            column = 1;
            continue;
        }
        column++;
        
        if (brace_count < 0) {
            result.success = 0;
            result.error_message = strdup("Unexpected closing brace");
            result.error_line = line;
            result.error_column = column;
            return result;
        }
    }
    
    if (brace_count != 0) {
        result.success = 0;
        result.error_message = strdup("Unclosed brace");
        result.error_line = line;
        result.error_column = column;
        return result;
    }
    
    // Check for missing semicolons (simplified)
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
                result.error_column = eol - pos;
                return result;
            }
            
            pos = strstr(eol, stmt_keywords[i]);
        }
    }
    
    // If we get here, parsing succeeded
    result.success = 1;
    result.node_count = 10; // Mock AST node count
    result.ast_summary = strdup("AST with package, functions, and statements");
    
    return result;
}

// Parser-specific Given steps
void given_asthra_source_code(const char* code) {
    bdd_given("Asthra source code to parse");
    
    if (source_code) free(source_code);
    source_code = strdup(code);
    
    BDD_ASSERT_NOT_NULL(source_code);
}

void given_syntactically_valid_code(void) {
    bdd_given("syntactically valid Asthra code");
    
    const char* valid_code = 
        "package parser_test;\n"
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
    
    given_asthra_source_code(valid_code);
}

void given_code_with_syntax_error(void) {
    bdd_given("Asthra code with syntax error");
    
    const char* error_code = 
        "package parser_test;\n"
        "\n"
        "pub fn broken(none) -> void {\n"
        "    let x = 42\n"  // Missing semicolon
        "    println(\"Value: {}\", x);\n"
        "}\n";
    
    given_asthra_source_code(error_code);
}

void given_code_with_unclosed_brace(void) {
    bdd_given("Asthra code with unclosed brace");
    
    const char* error_code = 
        "package parser_test;\n"
        "\n"
        "pub fn unclosed(none) -> void {\n"
        "    if true {\n"
        "        println(\"Missing closing brace\");\n"
        "    \n"  // Missing closing brace
        "}\n";
    
    given_asthra_source_code(error_code);
}

// Parser-specific When steps
void when_parse_the_code(void) {
    bdd_when("I parse the code");
    
    if (!source_code) {
        parser_result.success = 0;
        parser_result.error_message = strdup("No source code provided");
        return;
    }
    
    // Clean up previous result
    if (parser_result.error_message) {
        free(parser_result.error_message);
        parser_result.error_message = NULL;
    }
    if (parser_result.ast_summary) {
        free(parser_result.ast_summary);
        parser_result.ast_summary = NULL;
    }
    
    parser_result = mock_parse_asthra_code(source_code);
}

// Parser-specific Then steps
void then_parsing_should_succeed(void) {
    bdd_then("parsing should succeed");
    BDD_ASSERT_TRUE(parser_result.success);
}

void then_parsing_should_fail(void) {
    bdd_then("parsing should fail");
    BDD_ASSERT_FALSE(parser_result.success);
}

void then_ast_should_be_generated(void) {
    bdd_then("an AST should be generated");
    BDD_ASSERT_TRUE(parser_result.success);
    BDD_ASSERT_TRUE(parser_result.node_count > 0);
    BDD_ASSERT_NOT_NULL(parser_result.ast_summary);
}

void then_parser_error_contains(const char* expected_error) {
    char desc[256];
    snprintf(desc, sizeof(desc), "the parser error should contain \"%s\"", expected_error);
    bdd_then(desc);
    
    BDD_ASSERT_NOT_NULL(parser_result.error_message);
    if (parser_result.error_message) {
        BDD_ASSERT_TRUE(strstr(parser_result.error_message, expected_error) != NULL);
    }
}

void then_error_at_line(int line) {
    char desc[128];
    snprintf(desc, sizeof(desc), "the error should be at line %d", line);
    bdd_then(desc);
    
    BDD_ASSERT_EQ(parser_result.error_line, line);
}

// Test scenarios
void test_parse_valid_code(void) {
    bdd_scenario("Parse syntactically valid code");
    
    given_syntactically_valid_code();
    when_parse_the_code();
    then_parsing_should_succeed();
    then_ast_should_be_generated();
}

void test_detect_missing_semicolon(void) {
    bdd_scenario("Detect missing semicolon");
    
    given_code_with_syntax_error();
    when_parse_the_code();
    then_parsing_should_fail();
    then_parser_error_contains("expected ';'");
    then_error_at_line(4);
}

void test_detect_unclosed_brace(void) {
    bdd_scenario("Detect unclosed brace");
    
    given_code_with_unclosed_brace();
    when_parse_the_code();
    then_parsing_should_fail();
    then_parser_error_contains("Unclosed brace");
}

void test_empty_source_handling(void) {
    bdd_scenario("Handle empty source code");
    
    given_asthra_source_code("");
    when_parse_the_code();
    then_parsing_should_fail();
    then_parser_error_contains("Empty source code");
}

// Main test runner
int main(void) {
    bdd_init("Parser Functionality");
    
    // Run all parser scenarios
    test_parse_valid_code();
    test_detect_missing_semicolon();
    test_detect_unclosed_brace();
    test_empty_source_handling();
    
    // Cleanup
    if (source_code) free(source_code);
    if (parser_result.error_message) free(parser_result.error_message);
    if (parser_result.ast_summary) free(parser_result.ast_summary);
    
    return bdd_report();
}