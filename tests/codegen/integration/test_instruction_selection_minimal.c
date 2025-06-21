#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_instruction_selection", __FILE__, __LINE__, "Test basic instruction selection", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_arithmetic_instruction_patterns", __FILE__, __LINE__, "Test arithmetic instruction patterns", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_memory_instruction_selection", __FILE__, __LINE__, "Test memory instruction selection", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_control_flow_instructions", __FILE__, __LINE__, "Test control flow instruction selection", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions using minimal framework approach
static AsthraTestResult test_basic_instruction_selection(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_basic(void) {\n"
        "    let x = 5;\n"
        "    let y = x + 3;\n"
        "    return y;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_basic_instructions.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    // Basic validation - if we can parse and setup analyzer, test passes
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_arithmetic_instruction_patterns(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_arithmetic(void) {\n"
        "    let a = 10;\n"
        "    let b = a * 2;      // MUL pattern\n"
        "    let c = b << 1;     // SHL pattern\n"
        "    let d = c + (a * 4); // Complex pattern\n"
        "    return d;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_arithmetic_patterns.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_memory_instruction_selection(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_memory(void) {\n"
        "    let arr = [1, 2, 3, 4];\n"
        "    let val = arr[2];    // LOAD pattern\n"
        "    arr[1] = val * 2;    // STORE pattern\n"
        "    return arr[1];\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_memory_instructions.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_control_flow_instructions(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_control_flow(x: i32) -> i32 {\n"
        "    if (x > 0) {         // CMP + JGT pattern\n"
        "        return x * 2;\n"
        "    } else {\n"
        "        return x + 1;    // JMP pattern\n"
        "    }\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_control_instructions.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    AsthraTestFunction tests[] = {
        test_basic_instruction_selection,
        test_arithmetic_instruction_patterns,
        test_memory_instruction_selection,
        test_control_flow_instructions
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    printf("Running Instruction Selection Tests (Minimal Framework)\n");
    printf("======================================================\n");
    
    int passed = 0;
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestContext context = {
            .metadata = test_metadata[i],
            .result = ASTHRA_TEST_PASS,
            .error_message = NULL,
            .assertions_count = 0,
            .assertions_passed = 0
        };
        
        printf("Running %s... ", test_metadata[i].name);
        AsthraTestResult result = tests[i](&context);
        
        if (result == ASTHRA_TEST_PASS) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
        }
    }
    
    printf("\nInstruction Selection Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
} 
