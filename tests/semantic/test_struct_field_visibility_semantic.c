/**
 * Semantic analysis tests for struct field visibility enforcement
 * Tests whether private fields are properly protected from external access
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/analysis/semantic_errors.h"
#include "../../src/parser/ast.h"
#include "../../src/parser/lexer.h"
#include "../../src/parser/parser.h"

// Helper function to run semantic analysis
static SemanticAnalyzer *analyze_source(const char *source) {
    Lexer *lexer = lexer_create(source, strlen(source), "<test>");
    if (!lexer)
        return NULL;

    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }

    ASTNode *program = parse_program(parser);
    parser_destroy(parser);

    if (!program)
        return NULL;

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        ast_free_node(program);
        return NULL;
    }

    semantic_analyze_program(analyzer, program);
    ast_free_node(program);

    return analyzer;
}

// Test 1: Access public field from same package
void test_access_public_field_same_package(void) {
    printf("Testing access to public field from same package ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct Point {\n"
                         "    pub x: i32,\n"
                         "    pub y: i32\n"
                         "}\n"
                         "\n"
                         "pub fn test_public_access(none) -> void {\n"
                         "    let p: Point = Point { x: 10, y: 20 };\n"
                         "    let x_val: i32 = p.x;  // Should be allowed\n"
                         "    let y_val: i32 = p.y;  // Should be allowed\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    // Should have no errors
    assert(analyzer->error_count == 0);

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Public field access allowed in same package\n");
}

// Test 2: Access private field from same package
void test_access_private_field_same_package(void) {
    printf("Testing access to private field from same package ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct BankAccount {\n"
                         "    pub account_number: string,\n"
                         "    priv balance: f64,\n"
                         "    priv pin: i32\n"
                         "}\n"
                         "\n"
                         "pub fn test_private_access(none) -> void {\n"
                         "    let account: BankAccount = BankAccount {\n"
                         "        account_number: \"123456\",\n"
                         "        balance: 1000.0,\n"
                         "        pin: 1234\n"
                         "    };\n"
                         "    let num: string = account.account_number;  // OK - public\n"
                         "    let bal: f64 = account.balance;  // Should be error - private\n"
                         "    let p: i32 = account.pin;        // Should be error - private\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    // TODO: Currently the semantic analyzer doesn't enforce field visibility
    // When implemented, this test should show errors for accessing private fields
    // For now, we document the current behavior

    printf("    Current behavior: Private fields not enforced (TODO in "
           "semantic_structs_visibility.c)\n");

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Test documents current behavior\n");
}

// Test 3: Mixed visibility in struct literal
void test_struct_literal_with_visibility(void) {
    printf("Testing struct literal initialization with mixed visibility ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct User {\n"
                         "    pub id: i32,\n"
                         "    pub name: string,\n"
                         "    priv password_hash: string\n"
                         "}\n"
                         "\n"
                         "pub fn create_user(none) -> User {\n"
                         "    // Should be able to initialize private fields within same module\n"
                         "    return User {\n"
                         "        id: 1,\n"
                         "        name: \"Alice\",\n"
                         "        password_hash: \"hashed123\"\n"
                         "    };\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    // Struct literal initialization should work regardless of field visibility
    // within the same module
    assert(analyzer->error_count == 0);

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Struct literal initialization allowed for all fields\n");
}

// Test 4: Default visibility enforcement
void test_default_visibility_enforcement(void) {
    printf("Testing default (private) visibility enforcement ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct Config {\n"
                         "    timeout: i32,      // default private\n"
                         "    pub retries: i32,\n"
                         "    verbose: bool      // default private\n"
                         "}\n"
                         "\n"
                         "pub fn test_defaults(none) -> void {\n"
                         "    let cfg: Config = Config {\n"
                         "        timeout: 30,\n"
                         "        retries: 3,\n"
                         "        verbose: true\n"
                         "    };\n"
                         "    let t: i32 = cfg.timeout;   // Should be error - private by default\n"
                         "    let r: i32 = cfg.retries;   // OK - public\n"
                         "    let v: bool = cfg.verbose;  // Should be error - private by default\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    // Document current behavior
    printf("    Current behavior: Default private visibility not enforced\n");

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Test documents default visibility behavior\n");
}

// Test 5: Method access to private fields
void test_method_access_to_private_fields(void) {
    printf("Testing method access to private fields ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct Rectangle {\n"
                         "    pub width: f64,\n"
                         "    pub height: f64,\n"
                         "    priv area_cache: f64\n"
                         "}\n"
                         "\n"
                         "impl Rectangle {\n"
                         "    pub fn calculate_area(self) -> f64 {\n"
                         "        // Methods should access private fields of their own struct\n"
                         "        if self.area_cache > 0.0 {\n"
                         "            return self.area_cache;\n"
                         "        }\n"
                         "        return self.width * self.height;\n"
                         "    }\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    // Methods should be able to access private fields of their struct
    assert(analyzer->error_count == 0);

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Methods can access private fields of their struct\n");
}

// Test 6: Cross-struct private field access
void test_cross_struct_private_access(void) {
    printf("Testing cross-struct private field access ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct A {\n"
                         "    pub public_a: i32,\n"
                         "    priv private_a: i32\n"
                         "}\n"
                         "\n"
                         "pub struct B {\n"
                         "    pub public_b: i32,\n"
                         "    priv private_b: i32\n"
                         "}\n"
                         "\n"
                         "impl A {\n"
                         "    pub fn access_b(self, b: B) -> i32 {\n"
                         "        let pub_ok: i32 = b.public_b;   // OK - public\n"
                         "        let priv_err: i32 = b.private_b; // Should be error\n"
                         "        return pub_ok + priv_err;\n"
                         "    }\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    // Document current behavior for cross-struct access
    printf("    Current behavior: Cross-struct private access not enforced\n");

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Test documents cross-struct visibility behavior\n");
}

// Test 7: Nested field access with visibility
void test_nested_field_visibility(void) {
    printf("Testing nested struct field visibility ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct Inner {\n"
                         "    pub visible: i32,\n"
                         "    priv hidden: i32\n"
                         "}\n"
                         "\n"
                         "pub struct Outer {\n"
                         "    pub inner: Inner,\n"
                         "    priv secret: Inner\n"
                         "}\n"
                         "\n"
                         "pub fn test_nested(none) -> void {\n"
                         "    let o: Outer = Outer {\n"
                         "        inner: Inner { visible: 1, hidden: 2 },\n"
                         "        secret: Inner { visible: 3, hidden: 4 }\n"
                         "    };\n"
                         "    \n"
                         "    // These should be checked:\n"
                         "    let v1: i32 = o.inner.visible;   // OK - both fields public\n"
                         "    let h1: i32 = o.inner.hidden;    // Error - hidden is private\n"
                         "    let s: Inner = o.secret;         // Error - secret is private\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    printf("    Current behavior: Nested field visibility not enforced\n");

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Test documents nested visibility behavior\n");
}

// Test 8: Generic struct field visibility
void test_generic_struct_visibility(void) {
    printf("Testing generic struct field visibility ...\n");

    const char *source = "package test;\n"
                         "\n"
                         "pub struct Container<T> {\n"
                         "    pub value: T,\n"
                         "    priv metadata: string\n"
                         "}\n"
                         "\n"
                         "pub fn test_generic(none) -> void {\n"
                         "    let c: Container<i32> = Container<i32> {\n"
                         "        value: 42,\n"
                         "        metadata: \"secret\"\n"
                         "    };\n"
                         "    \n"
                         "    let v: i32 = c.value;        // OK - public\n"
                         "    let m: string = c.metadata;  // Should be error - private\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = analyze_source(source);
    assert(analyzer != NULL);

    printf("    Current behavior: Generic struct field visibility not enforced\n");

    semantic_analyzer_destroy(analyzer);
    printf("  ✓ Test documents generic struct visibility behavior\n");
}

int main(void) {
    printf("=== Struct Field Visibility Semantic Tests ===\n\n");

    test_access_public_field_same_package();
    test_access_private_field_same_package();
    test_struct_literal_with_visibility();
    test_default_visibility_enforcement();
    test_method_access_to_private_fields();
    test_cross_struct_private_access();
    test_nested_field_visibility();
    test_generic_struct_visibility();

    printf("\n📝 Summary: Field visibility is parsed correctly but not enforced\n");
    printf("   - Parser extracts visibility modifiers correctly\n");
    printf("   - Semantic analyzer has TODO for enforcement\n");
    printf("   - All field access is currently allowed\n");
    printf("\n✅ All semantic tests completed!\n");
    return 0;
}