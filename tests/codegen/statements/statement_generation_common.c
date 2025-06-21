/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Common fixtures and utilities implementation for statement generation tests
 */

#include "statement_generation_test_fixtures.h"
#include "expression_oriented_test_utils.h"

// =============================================================================
// FIXTURE MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Setup test fixture with a code generator
 */
CodeGenTestFixture* setup_codegen_fixture(void) {
    CodeGenTestFixture* fixture = calloc(1, sizeof(CodeGenTestFixture));
    if (!fixture) return NULL;
    
    fixture->generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!fixture->generator) {
        free(fixture);
        return NULL;
    }
    
    fixture->analyzer = setup_semantic_analyzer();
    if (!fixture->analyzer) {
        code_generator_destroy(fixture->generator);
        free(fixture);
        return NULL;
    }
    
    // Connect the semantic analyzer to the code generator
    fixture->generator->semantic_analyzer = fixture->analyzer;
    
    fixture->output_buffer_size = 4096;
    fixture->output_buffer = malloc(fixture->output_buffer_size);
    if (!fixture->output_buffer) {
        destroy_semantic_analyzer(fixture->analyzer);
        code_generator_destroy(fixture->generator);
        free(fixture);
        return NULL;
    }
    
    return fixture;
}

/**
 * Cleanup test fixture and free all resources
 */
void cleanup_codegen_fixture(CodeGenTestFixture* fixture) {
    if (!fixture) return;
    
    if (fixture->test_ast) {
        ast_free_node(fixture->test_ast);
    }
    if (fixture->output_buffer) {
        free(fixture->output_buffer);
    }
    if (fixture->analyzer) {
        destroy_semantic_analyzer(fixture->analyzer);
    }
    if (fixture->generator) {
        code_generator_destroy(fixture->generator);
    }
    free(fixture);
}

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Generate code for a statement and verify basic success
 */
bool generate_and_verify_statement(AsthraTestContext* context, 
                                  CodeGenTestFixture* fixture, 
                                  ASTNode* ast, 
                                  const char* test_name) {
    if (!ast) {
        return false;
    }
    
    bool result = code_generate_statement(fixture->generator, ast);
    if (!result) {
        if (context) {
            context->result = ASTHRA_TEST_FAIL;
            printf("ERROR: Failed to generate code for %s\n", test_name);
        }
        return false;
    }
    
    return true;
}

/**
 * Parse test source and verify parsing succeeded
 */
ASTNode* parse_and_verify_source(AsthraTestContext* context, 
                                const char* source, 
                                const char* filename, 
                                const char* test_name) {
    ASTNode* ast = parse_test_source(source, filename);
    if (!ast) {
        if (context) {
            context->result = ASTHRA_TEST_FAIL;
            printf("ERROR: Failed to parse source for %s\n", test_name);
        }
        return NULL;
    }
    
    return ast;
}

/**
 * Parse a statement fragment by wrapping it in a valid Asthra program
 * This is needed because the parser expects complete programs starting with 'package'
 */
ASTNode* parse_statement_fragment(const char* fragment, const char* test_name) {
    // Create a buffer large enough for the wrapper
    size_t buffer_size = strlen(fragment) + 4096;  // Increased buffer size
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        printf("ERROR: Failed to allocate buffer for %s\n", test_name);
        return NULL;
    }
    
    // Detect if this is a return statement with a value
    // Only detect return with value if the fragment is PRIMARILY a return statement
    bool is_return_with_value = false;
    const char* trimmed = fragment;
    while (*trimmed && (*trimmed == ' ' || *trimmed == '\t')) {
        trimmed++;
    }
    
    // Only consider it a return with value if the fragment starts with "return" and has a value
    if (strncmp(trimmed, "return", 6) == 0) {
        const char* after_return = trimmed + 6;
        while (*after_return && (*after_return == ' ' || *after_return == '\t')) {
            after_return++;
        }
        // Check if there's a value after return (not just ; or ())
        if (*after_return && *after_return != ';' && 
            !(after_return[0] == '(' && after_return[1] == ')')) {
            is_return_with_value = true;
        }
    }
    
    // Always use void return type for test functions to avoid semantic errors
    // The test is about statement generation, not return types
    const char* return_type = "void";
    const char* default_return = "    return ();\n";
    
    // Wrap the fragment in a minimal valid Asthra program
    // Create the wrapper program piece by piece
    strcpy(buffer, "package test;\n\n");
    
    // Add common function declarations
    strcat(buffer, "// Common test functions\n");
    strcat(buffer, "pub fn action1(none) -> void { return (); }\n");
    strcat(buffer, "pub fn action2(none) -> void { return (); }\n");
    strcat(buffer, "pub fn action3(none) -> void { return (); }\n");
    strcat(buffer, "pub fn body(none) -> void { return (); }\n");
    strcat(buffer, "pub fn process(n: int) -> void { return (); }\n");
    strcat(buffer, "pub fn update(none) -> void { return (); }\n");
    strcat(buffer, "pub fn action(none) -> void { return (); }\n");
    strcat(buffer, "pub fn func(none) -> void { return (); }\n");
    strcat(buffer, "pub fn nested_func(a: int, b: int) -> int { return a + b; }\n");
    strcat(buffer, "pub fn get_value(none) -> int { return 42; }\n");
    strcat(buffer, "pub fn initialize(none) -> void { return (); }\n");
    strcat(buffer, "pub fn cleanup_resources(none) -> void { return (); }\n");
    strcat(buffer, "pub fn print_debug_info(level: int) -> void { return (); }\n");
    strcat(buffer, "pub fn update_display(none) -> void { return (); }\n");
    strcat(buffer, "pub fn save_state(none) -> void { return (); }\n");
    strcat(buffer, "pub fn print(s: string, value: int) -> void { return (); }\n");
    strcat(buffer, "pub fn printf(s: string) -> void { return (); }\n");
    strcat(buffer, "pub fn calculate(x: int, y: int, z: int) -> int { return x + y + z; }\n");
    strcat(buffer, "pub fn compute_index(none) -> int { return 0; }\n");
    strcat(buffer, "pub fn timestamp(none) -> int { return 0; }\n");
    strcat(buffer, "pub fn transform(m: int, s: int) -> void { return (); }\n");
    strcat(buffer, "pub fn log(level: int, msg: string) -> void { return (); }\n");
    strcat(buffer, "pub fn validate(input: string, pattern: string) -> bool { return true; }\n");
    strcat(buffer, "pub fn getObject(none) -> TestObject { return TestObject { member: 0 }; }\n");
    strcat(buffer, "pub fn handle_error(none) -> bool { return false; }\n");
    strcat(buffer, "\n");
    
    // Add struct declarations
    strcat(buffer, "// Test structures\n");
    strcat(buffer, "pub struct TestObject {\n");
    strcat(buffer, "    pub member: int,\n");
    strcat(buffer, "    pub property: TestProperty\n");
    strcat(buffer, "}\n\n");
    
    strcat(buffer, "pub struct TestProperty {\n");
    strcat(buffer, "    pub value: int\n");
    strcat(buffer, "}\n\n");
    
    // Remove impl blocks - not supported in Asthra
    // Instead, use regular functions that take the struct as first parameter
    strcat(buffer, "pub fn TestObject_method(obj: TestObject, p: int) -> void { return (); }\n");
    strcat(buffer, "pub fn TestObject_method1(obj: TestObject) -> TestObject { return obj; }\n");
    strcat(buffer, "pub fn TestObject_method2(obj: TestObject) -> TestObject { return obj; }\n");
    strcat(buffer, "pub fn TestObject_method3(obj: TestObject) -> void { return (); }\n\n");
    
    strcat(buffer, "pub struct Module { pub field: int }\n");
    strcat(buffer, "pub fn Module_function(m: Module, p: int) -> void { return (); }\n\n");
    
    strcat(buffer, "pub struct Class { pub field: int }\n");
    strcat(buffer, "pub fn Class_staticMethod(c: Class, p: int) -> void { return (); }\n\n");
    
    // Add helper functions
    strcat(buffer, "pub fn func1(a: int) -> int { return a * 2; }\n");
    strcat(buffer, "pub fn func2(b: int) -> int { return b + 10; }\n\n");
    
    strcat(buffer, "pub struct Transformation { pub data: int }\n");
    strcat(buffer, "pub fn Transformation_scale(t: Transformation) -> int { return 2; }\n\n");
    
    // Create test function with proper return type
    char func_header[256];
    sprintf(func_header, "pub fn test_function(none) -> %s {\n", return_type);
    strcat(buffer, func_header);
    
    // Add variable declarations
    strcat(buffer, "    // Test variables\n");
    strcat(buffer, "    let mut x: int = 0;\n");
    strcat(buffer, "    let mut y: int = 0;\n");
    strcat(buffer, "    let mut z: int = 0;\n");
    strcat(buffer, "    let mut i: int = 0;\n");
    strcat(buffer, "    let mut j: int = 0;\n");
    strcat(buffer, "    let mut a: int = 1;\n");
    strcat(buffer, "    let mut b: int = 2;\n");
    strcat(buffer, "    let mut c: int = 3;\n");
    strcat(buffer, "    let mut d: int = 4;\n");
    strcat(buffer, "    let mut e: int = 5;\n");
    strcat(buffer, "    let mut f: int = 6;\n");
    strcat(buffer, "    let mut m: int = 7;\n");
    strcat(buffer, "    let mut n: int = 10;\n");
    strcat(buffer, "    let mut p: int = 11;\n");
    strcat(buffer, "    let mut q: int = 12;\n");
    strcat(buffer, "    let mut r: int = 13;\n");
    strcat(buffer, "    let mut s: int = 14;\n");
    strcat(buffer, "    let mut t: int = 15;\n");
    strcat(buffer, "    let mut u: int = 16;\n");
    strcat(buffer, "    let mut rows: int = 5;\n");
    strcat(buffer, "    let mut cols: int = 5;\n");
    strcat(buffer, "    let mut start: int = 0;\n");
    strcat(buffer, "    let mut end: int = 10;\n");
    strcat(buffer, "    let mut valid: bool = true;\n");
    strcat(buffer, "    let mut arr: []int = [1, 2, 3, 4, 5];\n");
    strcat(buffer, "    let condition: bool = true;\n");
    strcat(buffer, "    let condition1: bool = true;\n");
    strcat(buffer, "    let condition2: bool = false;\n");
    strcat(buffer, "    let outer_condition: bool = true;\n");
    strcat(buffer, "    let inner_condition: bool = true;\n");
    strcat(buffer, "    let outer_loop: bool = true;\n");
    strcat(buffer, "    let value: int = 42;\n");
    strcat(buffer, "    let args: int = 0;\n");
    strcat(buffer, "    let index: int = 0;\n");
    strcat(buffer, "    let array: []int = [1, 2, 3];\n");
    strcat(buffer, "    let row: int = 0;\n");
    strcat(buffer, "    let col: int = 0;\n");
    strcat(buffer, "    let offset: int = 5;\n");
    strcat(buffer, "    let base: int = 100;\n");
    strcat(buffer, "    let divisor: int = 10;\n");
    strcat(buffer, "    let counter: int = 0;\n");
    strcat(buffer, "    let flag: bool = true;\n");
    strcat(buffer, "    let true_val: int = 1;\n");
    strcat(buffer, "    let false_val: int = 0;\n");
    strcat(buffer, "    let output: string = \"output\";\n");
    strcat(buffer, "    let matrix: []int = [1, 2, 3, 4];\n");
    strcat(buffer, "    let transformation: Transformation = Transformation { data: 1 };\n");
    strcat(buffer, "    let variable: int = 0;\n");
    strcat(buffer, "    let mut object: TestObject = TestObject { member: 0, property: TestProperty { value: 42 } };\n");
    strcat(buffer, "    let mut obj: TestObject = TestObject { member: 0, property: TestProperty { value: 42 } };\n");
    strcat(buffer, "    let module: Module = Module { field: 0 };\n");
    strcat(buffer, "    let param: int = 42;\n");
    strcat(buffer, "    let debug_level: int = 1;\n");
    strcat(buffer, "    let format_string: string = \"format\";\n");
    strcat(buffer, "    let value1: int = 1;\n");
    strcat(buffer, "    let value2: int = 2;\n");
    strcat(buffer, "    let other: int = 0;\n");
    strcat(buffer, "    let scale_factor: int = 1;\n");
    strcat(buffer, "    let level: int = 1;\n");
    strcat(buffer, "    let message: string = \"msg\";\n");
    strcat(buffer, "    let user: TestObject = TestObject { member: 0, property: TestProperty { value: 42 } };\n");
    strcat(buffer, "    let input: string = \"input\";\n");
    strcat(buffer, "    let regex_pattern: string = \"pattern\";\n");
    strcat(buffer, "    let instance: TestObject = TestObject { member: 0, property: TestProperty { value: 42 } };\n");
    strcat(buffer, "    let struct_ptr: *TestObject = &object;\n");
    strcat(buffer, "    let pointer: *int = &variable;\n");
    strcat(buffer, "    let cast_type: int = 0;\n");
    strcat(buffer, "    \n");
    
    // Add the test fragment with expression-oriented transformations
    strcat(buffer, "    // Test fragment\n");
    strcat(buffer, "    ");
    
    // Apply expression-oriented transformations
    char transformed_fragment[4096];
    wrap_fragment_for_expression_context(fragment, transformed_fragment, sizeof(transformed_fragment));
    
    // Transformation successful
    
    strcat(buffer, transformed_fragment);
    
    // Ensure fragment is treated as a statement
    size_t frag_len = strlen(transformed_fragment);
    if (frag_len > 0 && transformed_fragment[frag_len - 1] != ';') {
        // Check if it's an if-else expression (ends with })
        if (strstr(transformed_fragment, "if") && strstr(transformed_fragment, "else") && 
            transformed_fragment[frag_len - 1] == '}') {
            strcat(buffer, ";");
        } else if (transformed_fragment[frag_len - 1] != '}') {
            // Other expressions need semicolons too
            strcat(buffer, ";");
        }
    }
    strcat(buffer, "\n    \n");
    
    // Add default return if needed
    strcat(buffer, default_return);
    strcat(buffer, "}\n");
    
    // Debug: write to file to check line numbers
    FILE* debug_file = fopen("/tmp/test_debug.asthra", "w");
    if (debug_file) {
        fprintf(debug_file, "%s", buffer);
        fclose(debug_file);
    }
    
    // Parse the complete program
    ASTNode* program = parse_test_source(buffer, "test.asthra");
    
    free(buffer);
    
    if (!program) {
        printf("ERROR: Failed to parse wrapped source for %s\n", test_name);
        return NULL;
    }
    
    // Extract the test function from the program
    if (program->type != AST_PROGRAM) {
        ast_free_node(program);
        printf("ERROR: Expected program node for %s\n", test_name);
        return NULL;
    }
    
    // Find the test function in the declarations
    ASTNode* test_function = NULL;
    ASTNodeList* declarations = program->data.program.declarations;
    if (declarations) {
        size_t decl_count = ast_node_list_size(declarations);
        for (size_t i = 0; i < decl_count; i++) {
            ASTNode* decl = ast_node_list_get(declarations, i);
            if (decl && decl->type == AST_FUNCTION_DECL && 
                strcmp(decl->data.function_decl.name, "test_function") == 0) {
                test_function = decl;
                break;
            }
        }
    }
    
    if (!test_function) {
        ast_free_node(program);
        printf("ERROR: Could not find test function for %s\n", test_name);
        return NULL;
    }
    
    // Get the function body (block statement)
    ASTNode* body = test_function->data.function_decl.body;
    if (!body || body->type != AST_BLOCK) {
        ast_free_node(program);
        printf("ERROR: Test function has no body for %s\n", test_name);
        return NULL;
    }
    
    // Find the actual test statement - it should be the last statement added
    ASTNode* test_statement = NULL;
    ASTNodeList* statements = body->data.block.statements;
    if (statements) {
        size_t stmt_count = ast_node_list_size(statements);
        // The test statement should be the last one added (after all the setup variables)
        if (stmt_count > 0) {
            test_statement = ast_node_list_get(statements, stmt_count - 1);
        }
    }
    
    // For now, just return the whole program instead of extracting the statement
    // This avoids complexity of cloning nodes and the code generator can handle the full program
    return program;
}

/**
 * Common test pattern: parse source, generate code, cleanup
 */
AsthraTestResult test_statement_generation_pattern(AsthraTestContext* context,
                                                  CodeGenTestFixture* fixture,
                                                  const char* source,
                                                  const char* test_name) {
    // Use the fragment parser for statement fragments
    ASTNode* program = parse_statement_fragment(source, test_name);
    if (!program) {
        if (context) {
            context->result = ASTHRA_TEST_FAIL;
        }
        return ASTHRA_TEST_FAIL;
    }
    
    // First, run semantic analysis on the program
    if (fixture->analyzer) {
        bool semantic_success = semantic_analyze_program(fixture->analyzer, program);
        if (!semantic_success) {
            // Print any semantic errors
            if (fixture->analyzer->error_count > 0) {
                printf("Semantic errors:\n");
                for (size_t i = 0; i < fixture->analyzer->error_count && i < 5; i++) {
                    printf("  - %s\n", fixture->analyzer->errors[i].message);
                }
            }
            ast_free_node(program);
            if (context) {
                context->result = ASTHRA_TEST_FAIL;
                printf("ERROR: Semantic analysis failed for %s\n", test_name);
            }
            return ASTHRA_TEST_FAIL;
        }
        
        // Set the semantic analyzer for the code generator
        fixture->generator->semantic_analyzer = fixture->analyzer;
    }
    
    // For now, generate code for the entire program
    // In a more sophisticated implementation, we would extract just the statement
    bool success = code_generate_program(fixture->generator, program);
    
    if (!success && context) {
        context->result = ASTHRA_TEST_FAIL;
        printf("ERROR: Failed to generate code for %s\n", test_name);
    }
    
    ast_free_node(program);
    
    return success ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// =============================================================================
// TEST SUITE CONFIGURATION
// =============================================================================

/**
 * Create standard test suite configuration for statement generation tests
 */
AsthraTestSuiteConfig create_statement_test_suite_config(const char* suite_name,
                                                        const char* description,
                                                        AsthraTestStatistics* stats) {
    AsthraTestSuiteConfig config = {
        .name = suite_name,
        .description = description,
        .parallel_execution = false,
        .stop_on_failure = false,
        .verbose_output = true,
        .json_output = false,
        .default_timeout_ns = 10000000000ULL, // 10 seconds
        .max_parallel_tests = 1,
        .statistics = stats
    };
    
    return config;
}

/**
 * Standard test metadata template for statement generation tests
 */
AsthraTestMetadata create_statement_test_metadata(const char* test_name,
                                                 AsthraTestSeverity severity,
                                                 uint64_t timeout_ns) {
    AsthraTestMetadata metadata = {
        .name = test_name,
        .file = __FILE__,
        .line = __LINE__,
        .function = test_name,
        .severity = severity,
        .timeout_ns = timeout_ns,
        .skip = false,
        .skip_reason = NULL
    };
    
    return metadata;
} 
