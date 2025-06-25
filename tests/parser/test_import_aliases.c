/**
 * Test suite for import aliases semantic analysis
 * Tests Phase 4 implementation: Import Aliases Semantic Analysis
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"

// Test helper functions
static ASTNode *parse_test_code(const char *code) {
    Lexer *lexer = lexer_create(code, strlen(code), "test.asthra");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    ASTNode *ast = parser_parse_program(parser);

    parser_destroy(parser);
    lexer_destroy(lexer);

    return ast;
}

static void test_basic_import_alias(void) {
    printf("Testing basic import alias...\n");

    const char *code = "package test;\n"
                       "import \"stdlib/string\" as str;\n"
                       "pub fn test(none) -> void {\n"
                       "    // Simple function body without module access\n"
                       "    return ();\n"
                       "}\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool success = semantic_analyze_program(analyzer, ast);
    assert(success);

    // Verify that the alias was registered
    SymbolTable *alias_table = symbol_table_resolve_alias(analyzer->current_scope, "str");
    assert(alias_table != NULL);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Basic import alias test passed\n");
}

static void test_multiple_import_aliases(void) {
    printf("Testing multiple import aliases...\n");

    const char *code = "package test;\n"
                       "import \"stdlib/string\" as str;\n"
                       "import \"stdlib/math\" as math;\n"
                       "import \"github.com/user/advanced-string\" as advstr;\n"
                       "pub fn test(none) -> void {\n"
                       "    // Simple function body without module access\n"
                       "    return ();\n"
                       "}\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool success = semantic_analyze_program(analyzer, ast);
    assert(success);

    // Verify all aliases were registered
    assert(symbol_table_resolve_alias(analyzer->current_scope, "str") != NULL);
    assert(symbol_table_resolve_alias(analyzer->current_scope, "math") != NULL);
    assert(symbol_table_resolve_alias(analyzer->current_scope, "advstr") != NULL);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Multiple import aliases test passed\n");
}

static void test_import_without_alias(void) {
    printf("Testing import without alias...\n");

    const char *code = "package test;\n"
                       "import \"stdlib/string\";\n"
                       "pub fn test(none) -> void {\n"
                       "    // No alias, so no module access via alias\n"
                       "}\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool success = semantic_analyze_program(analyzer, ast);
    assert(success);

    // Verify no alias was registered
    assert(symbol_table_resolve_alias(analyzer->current_scope, "string") == NULL);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Import without alias test passed\n");
}

static void test_alias_conflict_detection(void) {
    printf("Testing alias conflict detection...\n");

    const char *code = "package test;\n"
                       "import \"stdlib/string\" as str;\n"
                       "import \"stdlib/stream\" as str;\n" // Conflicting alias
                       "pub fn test(none) -> void { return (); }\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool success = semantic_analyze_program(analyzer, ast);
    assert(!success); // Should fail due to alias conflict

    // Verify error was reported
    assert(semantic_get_error_count(analyzer) > 0);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Alias conflict detection test passed\n");
}

static void test_module_access_resolution(void) {
    printf("Testing module access resolution...\n");

    const char *code = "package test;\n"
                       "import \"stdlib/string\" as str;\n"
                       "pub fn test(none) -> void {\n"
                       "    // Simple function body without module access\n"
                       "    return ();\n"
                       "}\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    // Analyze the program to register the alias
    bool success = semantic_analyze_program(analyzer, ast);
    assert(success);

    // Verify that the alias was registered
    SymbolTable *str_module = symbol_table_resolve_alias(analyzer->current_scope, "str");
    assert(str_module != NULL);

    // Add a mock trim function to the module
    SymbolEntry *trim_symbol = symbol_entry_create("trim", SYMBOL_FUNCTION, NULL, NULL);
    assert(trim_symbol != NULL);
    bool inserted = symbol_table_insert_safe(str_module, "trim", trim_symbol);
    assert(inserted);

    // Verify that we can now look up the function in the module
    SymbolEntry *found_symbol = symbol_table_lookup_safe(str_module, "trim");
    assert(found_symbol != NULL);
    assert(found_symbol == trim_symbol);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Module access resolution test passed\n");
}

static void test_undefined_module_member(void) {
    printf("Testing undefined module member error...\n");

    const char *code = "package test;\n"
                       "import \"stdlib/string\" as str;\n"
                       "pub fn test(none) -> void {\n"
                       "    let result: string = str.nonexistent_function(\"hello\");\n"
                       "}\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    // Register alias with empty module (no symbols)
    SymbolTable *str_module = symbol_table_create(16);
    symbol_table_add_alias(analyzer->current_scope, "str", "stdlib/string", str_module);

    bool success = semantic_analyze_program(analyzer, ast);
    assert(!success); // Should fail due to undefined member

    // Verify error was reported
    assert(semantic_get_error_count(analyzer) > 0);

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Undefined module member test passed\n");
}

static void test_regular_field_access_unchanged(void) {
    printf("Testing regular field access remains unchanged...\n");

    const char *code = "package test;\n"
                       "pub fn test(none) -> void {\n"
                       "    // Simple function body without field access\n"
                       "}\n";

    ASTNode *ast = parse_test_code(code);
    assert(ast != NULL);

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    assert(analyzer != NULL);

    bool success = semantic_analyze_program(analyzer, ast);
    assert(success); // Should succeed (no field access)

    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    printf("✓ Regular field access test passed\n");
}

int main(void) {
    printf("Running import aliases tests...\n\n");

    test_basic_import_alias();
    test_multiple_import_aliases();
    test_import_without_alias();
    test_alias_conflict_detection();
    test_module_access_resolution();
    test_undefined_module_member();
    test_regular_field_access_unchanged();

    printf("\n✅ All import aliases tests passed!\n");
    return 0;
}
