#include "bdd_unit_common.h"

void test_single_line_comments(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// This is a single-line comment\n"
        "pub fn main(none) -> i32 {\n"
        "    // Another comment inside function\n"
        "    let value: i32 = 42; // End-of-line comment\n"
        "    return value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Single-line comments",
                               "comment_single.asthra",
                               source,
                               NULL,
                               42);
}

void test_multiple_single_line_comments(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// First comment line\n"
        "// Second comment line\n"
        "// Third comment line\n"
        "pub fn main(none) -> i32 {\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple single-line comments",
                               "comment_multiple_single.asthra",
                               source,
                               NULL,
                               0);
}

void test_multi_line_comments(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "/*\n"
        " * This is a multi-line comment\n"
        " * spanning several lines\n"
        " * with detailed explanation\n"
        " */\n"
        "pub fn main(none) -> i32 {\n"
        "    return 1;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-line comments",
                               "comment_multi.asthra",
                               source,
                               NULL,
                               1);
}

void test_comments_between_declarations(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Function to calculate square\n"
        "pub fn square(x: i32) -> i32 {\n"
        "    return x * x;\n"
        "}\n"
        "\n"
        "/* \n"
        " * Main function entry point\n"
        " */\n"
        "pub fn main(none) -> i32 {\n"
        "    // Calculate square of 5\n"
        "    let result: i32 = square(5);\n"
        "    return result;\n"
        "}\n";
    
    bdd_run_execution_scenario("Comments between declarations",
                               "comment_between_decl.asthra",
                               source,
                               NULL,
                               25);
}

void test_comments_in_struct_definitions(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Point structure for 2D coordinates\n"
        "pub struct Point {\n"
        "    x: i32, // X coordinate\n"
        "    y: i32  // Y coordinate\n"
        "}\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    /* Create a new point */\n"
        "    let point: Point = Point { \n"
        "        x: 3, // Set X\n"
        "        y: 4  // Set Y\n"
        "    };\n"
        "    return point.x + point.y;\n"
        "}\n";
    
    bdd_run_execution_scenario("Comments in struct definitions",
                               "comment_struct.asthra",
                               source,
                               NULL,
                               7);
}

void test_comments_in_control_flow(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let value: i32 = 10;\n"
        "    \n"
        "    // Check if value is positive\n"
        "    if value > 0 {\n"
        "        // Value is positive\n"
        "        return 1;\n"
        "    } else {\n"
        "        /* Value is not positive */\n"
        "        return 0;\n"
        "    }\n"
        "}\n";
    
    bdd_run_execution_scenario("Comments in control flow",
                               "comment_control_flow.asthra",
                               source,
                               NULL,
                               1);
}

void test_comments_in_loops(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut sum: i32 = 0;\n"
        "    \n"
        "    // Iterate through numbers 0 to 4\n"
        "    for i in range(5) {\n"
        "        // Add current number to sum\n"
        "        sum = sum + i;\n"
        "        /* Continue to next iteration */\n"
        "    }\n"
        "    \n"
        "    return sum; // Return total sum\n"
        "}\n";
    
    bdd_run_execution_scenario("Comments in loops",
                               "comment_loops.asthra",
                               source,
                               NULL,
                               10);
}

void test_mixed_comment_styles(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Single line comment\n"
        "pub fn helper(value: i32) -> i32 {\n"
        "    /* Multi-line comment in function */\n"
        "    return value * 2; // End-of-line comment\n"
        "}\n"
        "\n"
        "/*\n"
        " * Main function documentation\n"
        " * Uses both comment styles\n"
        " */\n"
        "pub fn main(none) -> i32 {\n"
        "    // Call helper function\n"
        "    let result: i32 = helper(21); /* Get doubled value */\n"
        "    return result; // Return result\n"
        "}\n";
    
    bdd_run_execution_scenario("Mixed comment styles",
                               "comment_mixed.asthra",
                               source,
                               NULL,
                               42);
}

void test_comment_only_lines(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "//\n"
        "// Empty comment lines\n"
        "//\n"
        "\n"
        "/*\n"
        " *\n"
        " * Empty multi-line sections\n"
        " *\n"
        " */\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    return 0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Comment-only lines",
                               "comment_only_lines.asthra",
                               source,
                               NULL,
                               0);
}

void test_comments_with_code_content(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn main(none) -> i32 {\n"
        "    // let commented_var: i32 = 999;\n"
        "    /* \n"
        "     * if false_condition {\n"
        "     *     return 999;\n"
        "     * }\n"
        "     */\n"
        "    return 42; // This should execute normally\n"
        "}\n";
    
    bdd_run_execution_scenario("Comments with code content",
                               "comment_code_content.asthra",
                               source,
                               NULL,
                               42);
}

BDD_DECLARE_TEST_CASES(comment_syntax)
    BDD_TEST_CASE("single-line comments", test_single_line_comments),
    BDD_TEST_CASE("multiple single-line comments", test_multiple_single_line_comments),
    BDD_TEST_CASE("multi-line comments", test_multi_line_comments),
    BDD_TEST_CASE("comments between declarations", test_comments_between_declarations),
    BDD_TEST_CASE("comments in struct definitions", test_comments_in_struct_definitions),
    BDD_TEST_CASE("comments in control flow", test_comments_in_control_flow),
    BDD_TEST_CASE("comments in loops", test_comments_in_loops),
    BDD_TEST_CASE("mixed comment styles", test_mixed_comment_styles),
    BDD_TEST_CASE("comment-only lines", test_comment_only_lines),
    BDD_TEST_CASE("comments with code content", test_comments_with_code_content)
BDD_END_TEST_CASES()

BDD_UNIT_TEST_MAIN("Comment syntax", comment_syntax_test_cases)