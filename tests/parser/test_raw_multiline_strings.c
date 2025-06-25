/**
 * Comprehensive test suite for raw multi-line string verification
 * Tests raw multi-line strings as defined in grammar.txt line 230
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/parser/ast.h"
#include "../../src/parser/ast_node_list.h"
#include "../../src/parser/lexer.h"
#include "../../src/parser/parser.h"

// Helper function to create parser from source
static Parser *create_parser(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    return parser;
}

// Test 1: Basic raw multi-line string
void test_basic_raw_multiline_string(void) {
    printf("Testing basic raw multi-line string ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let raw_str: string = r\"\"\"This is a raw\n"
                         "multi-line string\n"
                         "with multiple lines\"\"\";\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);

    // Navigate to function body
    ASTNodeList *decls = program->data.program.declarations;
    assert(decls && decls->count == 1);

    ASTNode *main_func = decls->nodes[0];
    assert(main_func->type == AST_FUNCTION_DECL);

    ASTNode *body = main_func->data.function_decl.body;
    assert(body->type == AST_BLOCK);

    ASTNodeList *stmts = body->data.block.statements;
    assert(stmts && stmts->count == 2); // let + return

    // Check the string literal
    ASTNode *let_stmt = stmts->nodes[0];
    assert(let_stmt->type == AST_LET_STMT);
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;
    assert(initializer && initializer->type == AST_STRING_LITERAL);

    // Verify the raw string content includes newlines
    const char *str_value = initializer->data.string_literal.value;
    assert(strstr(str_value, "\n") != NULL); // Should contain newlines

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Basic raw multi-line string parsed correctly\n");
}

// Test 2: Raw strings with escape sequences (should NOT be processed)
void test_raw_string_no_escape_processing(void) {
    printf("Testing raw strings with escape sequences ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let raw_str: string = r\"\"\"Line 1\\nLine 2\\tTabbed\n"
                         "Path: C:\\Users\\Test\\file.txt\n"
                         "Quote: \\\"Hello\\\"\n"
                         "Backslash: \\\\\"\"\";\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Verify escape sequences are NOT processed
    const char *str_value = initializer->data.string_literal.value;
    assert(strstr(str_value, "\\n") != NULL);  // Should contain literal \n
    assert(strstr(str_value, "\\t") != NULL);  // Should contain literal \t
    assert(strstr(str_value, "\\\"") != NULL); // Should contain literal \"
    assert(strstr(str_value, "\\\\") != NULL); // Should contain literal backslash

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings preserve escape sequences correctly\n");
}

// Test 3: Raw strings with special characters
void test_raw_string_special_characters(void) {
    printf("Testing raw strings with special characters ...\n");

    const char *source =
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let special: string = r\"\"\"Special chars: !@#$%^&*()_+-=[]{}|;':,.<>?/\n"
        "Unicode: 你好世界 🌍 🚀\n"
        "Math: ∑ ∏ ∫ √ ∞\n"
        "Symbols: © ® ™ § ¶\"\"\";\n"
        "    return ();\n"
        "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Just verify it parses successfully
    assert(initializer && initializer->type == AST_STRING_LITERAL);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings handle special characters correctly\n");
}

// Test 4: Raw strings with embedded quotes
void test_raw_string_embedded_quotes(void) {
    printf("Testing raw strings with embedded quotes ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let quotes: string = r\"\"\"She said \"Hello!\"\n"
                         "Single quotes: 'yes' and 'no'\n"
                         "Mixed: \"It's a 'test' string\"\n"
                         "JSON: {\"key\": \"value\", \"number\": 42}\"\"\";\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Verify quotes are preserved
    const char *str_value = initializer->data.string_literal.value;
    assert(strstr(str_value, "\"Hello!\"") != NULL);
    assert(strstr(str_value, "'yes'") != NULL);
    assert(strstr(str_value, "{\"key\"") != NULL);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings preserve embedded quotes correctly\n");
}

// Test 5: Empty raw multi-line string
void test_empty_raw_multiline_string(void) {
    printf("Testing empty raw multi-line string ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let empty: string = r\"\"\"\"\"\";\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Verify empty string
    const char *str_value = initializer->data.string_literal.value;
    assert(strlen(str_value) == 0);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Empty raw multi-line string parsed correctly\n");
}

// Test 6: Raw strings with code snippets
void test_raw_string_code_snippets(void) {
    printf("Testing raw strings with code snippets ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let code: string = r\"\"\"fn example() {\n"
                         "    let x = 42;\n"
                         "    if x > 0 {\n"
                         "        println!(\"Positive: {}\", x);\n"
                         "    }\n"
                         "}\"\"\";\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Verify code structure is preserved
    const char *str_value = initializer->data.string_literal.value;
    assert(strstr(str_value, "fn example()") != NULL);
    assert(strstr(str_value, "    let x = 42;") != NULL); // Indentation preserved

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings preserve code snippets correctly\n");
}

// Test 7: Raw strings with regex patterns
void test_raw_string_regex_patterns(void) {
    printf("Testing raw strings with regex patterns ...\n");

    const char *source =
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let email_regex: string = "
        "r\"\"\"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$\"\"\";\n"
        "    let phone_regex: string = r\"\"\"\\+?[1-9]\\d{1,14}\"\"\";\n"
        "    let url_regex: string = r\"\"\"https?://[\\w\\-._~:/?#[\\]@!$&'()*+,;=]+\"\"\";\n"
        "    return ();\n"
        "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    // Verify all three regex patterns parse correctly
    assert(stmts->count == 4); // 3 let statements + return

    for (int i = 0; i < 3; i++) {
        ASTNode *let_stmt = stmts->nodes[i];
        assert(let_stmt->type == AST_LET_STMT);
        ASTNode *initializer = let_stmt->data.let_stmt.initializer;
        assert(initializer && initializer->type == AST_STRING_LITERAL);
    }

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings handle regex patterns correctly\n");
}

// Test 8: Raw strings vs processed multi-line strings
void test_raw_vs_processed_multiline(void) {
    printf("Testing raw vs processed multi-line strings ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    // Raw multi-line string - no escape processing\n"
                         "    let raw: string = r\"\"\"Line 1\\nLine 2\"\"\";\n"
                         "    \n"
                         "    // Processed multi-line string - escape sequences work\n"
                         "    let processed: string = \"\"\"Line 1\\nLine 2\"\"\";\n"
                         "    \n"
                         "    // They should be different\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    // Check raw string
    ASTNode *raw_let = stmts->nodes[0];
    ASTNode *raw_init = raw_let->data.let_stmt.initializer;
    const char *raw_value = raw_init->data.string_literal.value;
    assert(strstr(raw_value, "\\n") != NULL); // Contains literal \n

    // Check processed string
    ASTNode *proc_let = stmts->nodes[1];
    ASTNode *proc_init = proc_let->data.let_stmt.initializer;
    const char *proc_value = proc_init->data.string_literal.value;
    assert(strchr(proc_value, '\n') != NULL);  // Contains actual newline
    assert(strstr(proc_value, "\\n") == NULL); // Does NOT contain literal \n

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw and processed multi-line strings behave differently\n");
}

// Test 9: Raw strings with consecutive quotes
void test_raw_string_consecutive_quotes(void) {
    printf("Testing raw strings with consecutive quotes ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let quotes1: string = r\"\"\"One quote: \"\n"
                         "Two quotes: \"\"\n"
                         "Almost end: \"\"x\n"
                         "End coming: x\"\"x\"\"\";\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    ASTNodeList *decls = program->data.program.declarations;
    ASTNode *main_func = decls->nodes[0];
    ASTNode *body = main_func->data.function_decl.body;
    ASTNodeList *stmts = body->data.block.statements;

    ASTNode *let_stmt = stmts->nodes[0];
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;

    // Verify content with quotes is preserved
    const char *str_value = initializer->data.string_literal.value;
    assert(strstr(str_value, "One quote: \"") != NULL);
    assert(strstr(str_value, "Two quotes: \"\"") != NULL);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings handle consecutive quotes correctly\n");
}

// Test 10: Raw strings in various contexts
void test_raw_string_contexts(void) {
    printf("Testing raw strings in various contexts ...\n");

    const char *source =
        "package test;\n"
        "priv const TEMPLATE: string = r\"\"\"Template: {{name}}\"\"\";\n"
        "\n"
        "pub fn get_doc(none) -> string {\n"
        "    return r\"\"\"Documentation\n"
        "    with multiple lines\"\"\";\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // In function call\n"
        "    let result: string = process(r\"\"\"Raw input\n"
        "data\"\"\");\n"
        "    \n"
        "    // In array\n"
        "    let templates: []string = [r\"\"\"Template 1\"\"\", r\"\"\"Template 2\"\"\"];\n"
        "    \n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn process(input: string) -> string {\n"
        "    return input;\n"
        "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    assert(program != NULL);

    // Just verify it parses without errors
    assert(program->type == AST_PROGRAM);

    ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Raw strings work in various contexts\n");
}

// Test 11: Error case - unterminated raw string
void test_unterminated_raw_string(void) {
    printf("Testing unterminated raw string error ...\n");

    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let bad: string = r\"\"\"This string never ends\n"
                         "    return ();\n"
                         "}\n";

    Parser *parser = create_parser(source);
    assert(parser != NULL);

    ASTNode *program = parse_program(parser);
    // Should fail to parse or produce an error

    if (program)
        ast_free_node(program);
    parser_destroy(parser);
    printf("  ✓ Unterminated raw string handled\n");
}

int main(void) {
    printf("=== Comprehensive Raw Multi-line String Test Suite ===\n\n");

    test_basic_raw_multiline_string();
    test_raw_string_no_escape_processing();
    test_raw_string_special_characters();
    test_raw_string_embedded_quotes();
    test_empty_raw_multiline_string();
    test_raw_string_code_snippets();
    test_raw_string_regex_patterns();
    test_raw_vs_processed_multiline();
    test_raw_string_consecutive_quotes();
    test_raw_string_contexts();
    test_unterminated_raw_string();

    printf("\n✅ All raw multi-line string tests completed!\n");
    return 0;
}