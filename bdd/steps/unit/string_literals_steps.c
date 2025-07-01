#include "bdd_unit_common.h"
// Test scenarios for string literals

// Regular string literals
void test_simple_string_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Hello\";\n"
        "  if s == \"Hello\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple string literal",
                               "string_simple.asthra",
                               source,
                               NULL,
                               42);
}

void test_empty_string_literal(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"\";\n"
        "  if s == \"\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty string literal",
                               "string_empty.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_spaces(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Hello World\";\n"
        "  if s == \"Hello World\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("String with spaces",
                               "string_spaces.asthra",
                               source,
                               NULL,
                               42);
}

// Escape sequences in regular strings
void test_string_with_newline_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Line1\\nLine2\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with newline escape",
                               "string_newline.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_tab_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Column1\\tColumn2\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with tab escape",
                               "string_tab.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_carriage_return_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Text\\rReturn\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with carriage return escape",
                               "string_cr.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_backslash_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Path\\\\to\\\\file\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with backslash escape",
                               "string_backslash.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_quote_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"He said \\\"Hello\\\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with quote escape",
                               "string_quote.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_single_quote_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"It\\'s working\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with single quote escape",
                               "string_single_quote.asthra",
                               source,
                               NULL,
                               42);
}

void test_string_with_null_character_escape(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Null\\0End\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with null character escape",
                               "string_null.asthra",
                               source,
                               NULL,
                               42);
}

// Multi-line processed strings
void test_simple_multi_line_processed_string(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"\"\"Hello\n"
        "World\"\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple multi-line processed string",
                               "string_multiline_simple.asthra",
                               source,
                               NULL,
                               42);
}

void test_multi_line_processed_string_with_escape_sequences(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"\"\"Line1\\nLine2\n"
        "Line3\\tTabbed\"\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-line processed string with escape sequences",
                               "string_multiline_escapes.asthra",
                               source,
                               NULL,
                               42);
}

void test_empty_multi_line_processed_string(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"\"\"\"\"\";\n"
        "  if s == \"\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty multi-line processed string",
                               "string_multiline_empty.asthra",
                               source,
                               NULL,
                               42);
}

// Raw multi-line strings
void test_simple_raw_multi_line_string(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = r\"\"\"No escape processing\\n\"\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple raw multi-line string",
                               "string_raw_simple.asthra",
                               source,
                               NULL,
                               42);
}

void test_raw_string_with_backslashes(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = r\"\"\"C:\\path\\to\\file\"\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Raw string with backslashes",
                               "string_raw_backslashes.asthra",
                               source,
                               NULL,
                               42);
}

void test_raw_string_with_quotes(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = r\"\"\"He said \"Hello\" without escaping\"\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Raw string with quotes",
                               "string_raw_quotes.asthra",
                               source,
                               NULL,
                               42);
}

// String comparisons
void test_compare_regular_strings(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s1: string = \"test\";\n"
        "  let s2: string = \"test\";\n"
        "  if s1 == s2 {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Compare regular strings",
                               "string_compare.asthra",
                               source,
                               NULL,
                               42);
}

void test_compare_different_string_types(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s1: string = \"Hello\";\n"
        "  let s2: string = \"\"\"Hello\"\"\";\n"
        "  if s1 == s2 {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Compare different string types",
                               "string_compare_types.asthra",
                               source,
                               NULL,
                               42);
}

// String concatenation
void test_concatenate_two_strings(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s1: string = \"Hello\";\n"
        "  let s2: string = \" World\";\n"
        "  let s3: string = s1 + s2;\n"
        "  if s3 == \"Hello World\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Concatenate two strings",
                               "string_concat.asthra",
                               source,
                               NULL,
                               42);
}

// String in expressions
void test_string_in_conditional_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let name: string = \"Alice\";\n"
        "  if name == \"Alice\" || name == \"Bob\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("String in conditional expression",
                               "string_conditional.asthra",
                               source,
                               NULL,
                               42);
}

// String as function parameter
void test_pass_string_to_function(void) {
    const char* source = 
        "package test;\n"
        "pub fn check_string(s: string) -> i32 {\n"
        "  if s == \"correct\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "  return check_string(\"correct\");\n"
        "}\n";
    
    bdd_run_execution_scenario("Pass string to function",
                               "string_function_param.asthra",
                               source,
                               NULL,
                               42);
}

// Const strings
void test_const_string_declaration(void) {
    const char* source = 
        "package test;\n"
        "pub const MESSAGE: string = \"Hello, World!\";\n"
        "pub fn main(none) -> i32 {\n"
        "  if MESSAGE == \"Hello, World!\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Const string declaration",
                               "string_const.asthra",
                               source,
                               NULL,
                               42);
}

// String length
void test_get_string_length(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"Hello\";\n"
        "  let len: i32 = s.len as i32;\n"
        "  if len == 5 {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return len;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Get string length",
                               "string_length.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_string_with_only_escape_sequences(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"\\n\\t\\r\\\\\\\"\\'\\0\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("String with only escape sequences",
                               "string_only_escapes.asthra",
                               source,
                               NULL,
                               42);
}

void test_multi_line_string_with_indentation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let s: string = \"\"\"\n"
        "    Indented line 1\n"
        "      Indented line 2\n"
        "        Indented line 3\n"
        "    \"\"\";\n"
        "  return 42;\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-line string with indentation",
                               "string_multiline_indent.asthra",
                               source,
                               NULL,
                               42);
}

// String in match expressions
void test_match_on_string_value(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let cmd: string = \"run\";\n"
        "  match cmd {\n"
        "    \"run\" => { return 42; }\n"
        "    \"stop\" => { return 0; }\n"
        "    _ => { return 1; }\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Match on string value",
                               "string_match.asthra",
                               source,
                               NULL,
                               42);
}

// Mutable strings
void test_modify_mutable_string_variable(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "  let mut s: string = \"old\";\n"
        "  s = \"new\";\n"
        "  if s == \"new\" {\n"
        "    return 42;\n"
        "  } else {\n"
        "    return 0;\n"
        "  }\n"
        "}\n";
    
    bdd_run_execution_scenario("Modify mutable string variable",
                               "string_mutable.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BDD_DECLARE_TEST_CASES(string_literals)
    // Regular string literals
    BDD_WIP_TEST_CASE(simple_string_literal, test_simple_string_literal),
    BDD_WIP_TEST_CASE(empty_string_literal, test_empty_string_literal),
    BDD_WIP_TEST_CASE(string_with_spaces, test_string_with_spaces),
    
    // Escape sequences in regular strings
    BDD_WIP_TEST_CASE(string_with_newline_escape, test_string_with_newline_escape),
    BDD_WIP_TEST_CASE(string_with_tab_escape, test_string_with_tab_escape),
    BDD_WIP_TEST_CASE(string_with_carriage_return_escape, test_string_with_carriage_return_escape),
    BDD_WIP_TEST_CASE(string_with_backslash_escape, test_string_with_backslash_escape),
    BDD_WIP_TEST_CASE(string_with_quote_escape, test_string_with_quote_escape),
    BDD_WIP_TEST_CASE(string_with_single_quote_escape, test_string_with_single_quote_escape),
    BDD_WIP_TEST_CASE(string_with_null_character_escape, test_string_with_null_character_escape),
    
    // Multi-line processed strings
    BDD_WIP_TEST_CASE(simple_multi_line_processed_string, test_simple_multi_line_processed_string),
    BDD_WIP_TEST_CASE(multi_line_processed_string_with_escape_sequences, test_multi_line_processed_string_with_escape_sequences),
    BDD_WIP_TEST_CASE(empty_multi_line_processed_string, test_empty_multi_line_processed_string),
    
    // Raw multi-line strings
    BDD_WIP_TEST_CASE(simple_raw_multi_line_string, test_simple_raw_multi_line_string),
    BDD_WIP_TEST_CASE(raw_string_with_backslashes, test_raw_string_with_backslashes),
    BDD_WIP_TEST_CASE(raw_string_with_quotes, test_raw_string_with_quotes),
    
    // String comparisons
    BDD_WIP_TEST_CASE(compare_regular_strings, test_compare_regular_strings),
    BDD_WIP_TEST_CASE(compare_different_string_types, test_compare_different_string_types),
    
    // String concatenation
    BDD_WIP_TEST_CASE(concatenate_two_strings, test_concatenate_two_strings),
    
    // String in expressions
    BDD_WIP_TEST_CASE(string_in_conditional_expression, test_string_in_conditional_expression),
    
    // String as function parameter
    BDD_WIP_TEST_CASE(pass_string_to_function, test_pass_string_to_function),
    
    // Const strings
    BDD_WIP_TEST_CASE(const_string_declaration, test_const_string_declaration),
    
    // String length
    BDD_WIP_TEST_CASE(get_string_length, test_get_string_length),
    
    // Edge cases
    BDD_WIP_TEST_CASE(string_with_only_escape_sequences, test_string_with_only_escape_sequences),
    BDD_WIP_TEST_CASE(multi_line_string_with_indentation, test_multi_line_string_with_indentation),
    
    // String in match expressions
    BDD_WIP_TEST_CASE(match_on_string_value, test_match_on_string_value),
    
    // Mutable strings
    BDD_WIP_TEST_CASE(modify_mutable_string_variable, test_modify_mutable_string_variable),
BDD_END_TEST_CASES()

// Main test runner
BDD_UNIT_TEST_MAIN("String Literals", string_literals_test_cases)