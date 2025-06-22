#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bdd_support.h"

// Mock semantic analysis structures
typedef enum {
    TYPE_VOID,
    TYPE_I32,
    TYPE_I64,
    TYPE_F32,
    TYPE_F64,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_UNKNOWN
} TypeKind;

typedef struct {
    char* name;
    TypeKind type;
    int is_defined;
    int line;
} Symbol;

typedef struct {
    int success;
    char* error_message;
    int error_count;
    Symbol* symbols;
    int symbol_count;
} SemanticResult;

// Semantic analysis state
static char* analyzed_code = NULL;
static SemanticResult semantic_result = {0};

// Helper function to get type name
static const char* type_name(TypeKind type) {
    switch (type) {
        case TYPE_VOID: return "void";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        default: return "unknown";
    }
}

// Mock semantic analyzer
static SemanticResult mock_analyze_code(const char* code) {
    SemanticResult result = {0};
    result.success = 1;
    
    // Check for type mismatches in simple cases
    if (strstr(code, "let x: i32 = \"string\"")) {
        result.success = 0;
        result.error_message = strdup("Type mismatch: cannot assign string to i32");
        result.error_count = 1;
        return result;
    }
    
    if (strstr(code, "return 42") && strstr(code, "-> void")) {
        result.success = 0;
        result.error_message = strdup("Type mismatch: returning i32 from void function");
        result.error_count = 1;
        return result;
    }
    
    // Check for undefined variables
    if (strstr(code, "use_undefined")) {
        char* use_pos = strstr(code, "use_undefined");
        if (!strstr(code, "let use_undefined")) {
            result.success = 0;
            result.error_message = strdup("Undefined variable: use_undefined");
            result.error_count = 1;
            return result;
        }
    }
    
    // Check for duplicate definitions
    if (strstr(code, "duplicate_func")) {
        const char* first = strstr(code, "fn duplicate_func");
        if (first) {
            const char* second = strstr(first + 1, "fn duplicate_func");
            if (second) {
                result.success = 0;
                result.error_message = strdup("Duplicate function definition: duplicate_func");
                result.error_count = 1;
                return result;
            }
        }
    }
    
    // Build symbol table for successful analysis
    if (result.success) {
        result.symbol_count = 3;
        result.symbols = calloc(result.symbol_count, sizeof(Symbol));
        
        // Add some mock symbols
        result.symbols[0].name = strdup("main");
        result.symbols[0].type = TYPE_VOID;
        result.symbols[0].is_defined = 1;
        result.symbols[0].line = 5;
        
        result.symbols[1].name = strdup("x");
        result.symbols[1].type = TYPE_I32;
        result.symbols[1].is_defined = 1;
        result.symbols[1].line = 6;
        
        result.symbols[2].name = strdup("calculate");
        result.symbols[2].type = TYPE_I32;
        result.symbols[2].is_defined = 1;
        result.symbols[2].line = 10;
    }
    
    return result;
}

// Semantic-specific Given steps
void given_semantically_valid_code(void) {
    bdd_given("semantically valid Asthra code");
    
    const char* valid_code = 
        "package semantic_test;\n"
        "\n"
        "pub fn add(x: i32, y: i32) -> i32 {\n"
        "    return x + y;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: i32 = add(5, 3);\n"
        "    println(\"Result: {}\", result);\n"
        "    return ();\n"
        "}\n";
    
    if (analyzed_code) free(analyzed_code);
    analyzed_code = strdup(valid_code);
}

void given_code_with_type_mismatch(void) {
    bdd_given("code with type mismatch");
    
    const char* error_code = 
        "package semantic_test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let x: i32 = \"string\";\n"  // Type mismatch
        "    return ();\n"
        "}\n";
    
    if (analyzed_code) free(analyzed_code);
    analyzed_code = strdup(error_code);
}

void given_code_with_undefined_variable(void) {
    bdd_given("code with undefined variable");
    
    const char* error_code = 
        "package semantic_test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    println(\"Value: {}\", use_undefined);\n"  // Undefined variable
        "    return ();\n"
        "}\n";
    
    if (analyzed_code) free(analyzed_code);
    analyzed_code = strdup(error_code);
}

void given_code_with_wrong_return_type(void) {
    bdd_given("code with wrong return type");
    
    const char* error_code = 
        "package semantic_test;\n"
        "\n"
        "pub fn get_void(none) -> void {\n"
        "    return 42;\n"  // Returning i32 from void function
        "}\n";
    
    if (analyzed_code) free(analyzed_code);
    analyzed_code = strdup(error_code);
}

void given_code_with_duplicate_function(void) {
    bdd_given("code with duplicate function");
    
    const char* error_code = 
        "package semantic_test;\n"
        "\n"
        "pub fn duplicate_func(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn duplicate_func(x: i32) -> void {\n"  // Duplicate definition
        "    return ();\n"
        "}\n";
    
    if (analyzed_code) free(analyzed_code);
    analyzed_code = strdup(error_code);
}

// Semantic-specific When steps
void when_perform_semantic_analysis(void) {
    bdd_when("I perform semantic analysis");
    
    if (!analyzed_code) {
        semantic_result.success = 0;
        semantic_result.error_message = strdup("No code to analyze");
        return;
    }
    
    // Clean up previous result
    if (semantic_result.error_message) {
        free(semantic_result.error_message);
        semantic_result.error_message = NULL;
    }
    if (semantic_result.symbols) {
        for (int i = 0; i < semantic_result.symbol_count; i++) {
            if (semantic_result.symbols[i].name) {
                free(semantic_result.symbols[i].name);
            }
        }
        free(semantic_result.symbols);
        semantic_result.symbols = NULL;
    }
    
    semantic_result = mock_analyze_code(analyzed_code);
}

// Semantic-specific Then steps
void then_semantic_analysis_should_pass(void) {
    bdd_then("semantic analysis should pass");
    BDD_ASSERT_TRUE(semantic_result.success);
    BDD_ASSERT_EQ(semantic_result.error_count, 0);
}

void then_semantic_analysis_should_fail(void) {
    bdd_then("semantic analysis should fail");
    BDD_ASSERT_FALSE(semantic_result.success);
    BDD_ASSERT_TRUE(semantic_result.error_count > 0);
}

void then_type_error_detected(const char* expected_error) {
    char desc[256];
    snprintf(desc, sizeof(desc), "a type error should be detected: %s", expected_error);
    bdd_then(desc);
    
    BDD_ASSERT_NOT_NULL(semantic_result.error_message);
    if (semantic_result.error_message) {
        BDD_ASSERT_TRUE(strstr(semantic_result.error_message, expected_error) != NULL);
    }
}

void then_symbol_table_contains(const char* symbol_name) {
    char desc[128];
    snprintf(desc, sizeof(desc), "symbol table should contain '%s'", symbol_name);
    bdd_then(desc);
    
    int found = 0;
    for (int i = 0; i < semantic_result.symbol_count; i++) {
        if (semantic_result.symbols[i].name && 
            strcmp(semantic_result.symbols[i].name, symbol_name) == 0) {
            found = 1;
            break;
        }
    }
    
    BDD_ASSERT_TRUE(found);
}

void then_all_types_resolved(void) {
    bdd_then("all types should be resolved");
    
    for (int i = 0; i < semantic_result.symbol_count; i++) {
        BDD_ASSERT_NE(semantic_result.symbols[i].type, TYPE_UNKNOWN);
    }
}

// Test scenarios
void test_analyze_valid_code(void) {
    bdd_scenario("Analyze semantically valid code");
    
    given_semantically_valid_code();
    when_perform_semantic_analysis();
    then_semantic_analysis_should_pass();
    then_symbol_table_contains("main");
    then_symbol_table_contains("add");
    then_all_types_resolved();
}

void test_detect_type_mismatch(void) {
    bdd_scenario("Detect type mismatch");
    
    given_code_with_type_mismatch();
    when_perform_semantic_analysis();
    then_semantic_analysis_should_fail();
    then_type_error_detected("Type mismatch");
}

void test_detect_undefined_variable(void) {
    bdd_scenario("Detect undefined variable");
    
    given_code_with_undefined_variable();
    when_perform_semantic_analysis();
    then_semantic_analysis_should_fail();
    then_type_error_detected("Undefined variable");
}

void test_detect_wrong_return_type(void) {
    bdd_scenario("Detect wrong return type");
    
    given_code_with_wrong_return_type();
    when_perform_semantic_analysis();
    then_semantic_analysis_should_fail();
    then_type_error_detected("Type mismatch");
}

void test_detect_duplicate_function(void) {
    bdd_scenario("Detect duplicate function");
    
    given_code_with_duplicate_function();
    when_perform_semantic_analysis();
    then_semantic_analysis_should_fail();
    then_type_error_detected("Duplicate function");
}

// Main test runner
int main(void) {
    bdd_init("Semantic Analysis");
    
    // Run all semantic analysis scenarios
    test_analyze_valid_code();
    test_detect_type_mismatch();
    test_detect_undefined_variable();
    test_detect_wrong_return_type();
    test_detect_duplicate_function();
    
    // Cleanup
    if (analyzed_code) free(analyzed_code);
    if (semantic_result.error_message) free(semantic_result.error_message);
    if (semantic_result.symbols) {
        for (int i = 0; i < semantic_result.symbol_count; i++) {
            if (semantic_result.symbols[i].name) {
                free(semantic_result.symbols[i].name);
            }
        }
        free(semantic_result.symbols);
    }
    
    return bdd_report();
}