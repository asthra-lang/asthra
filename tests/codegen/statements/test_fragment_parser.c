/**
 * Asthra Programming Language Compiler
 * Statement Fragment Parser for Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Parse statement fragments by wrapping them in valid Asthra programs
 */

#include "expression_oriented_test_utils.h"
#include "statement_generation_test_fixtures.h"

// Forward declaration of helper functions
static void add_function_declarations(char *buffer, size_t *pos, size_t buffer_size);
static void add_struct_declarations(char *buffer, size_t *pos, size_t buffer_size);
static void add_variable_declarations(char *buffer, size_t *pos, size_t buffer_size);

/**
 * Parse a statement fragment by wrapping it in a valid Asthra program
 * This is needed because the parser expects complete programs starting with 'package'
 */
ASTNode *parse_statement_fragment(const char *fragment, const char *test_name) {
    // Create a buffer large enough for the wrapper
    size_t buffer_size = strlen(fragment) + 8192; // Increased buffer size
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        printf("ERROR: Failed to allocate buffer for %s\n", test_name);
        return NULL;
    }

    // Detect if this is a return statement with a value
    // Only detect return with value if the fragment is PRIMARILY a return statement
    bool is_return_with_value = false;
    const char *trimmed = fragment;
    while (*trimmed && (*trimmed == ' ' || *trimmed == '\t')) {
        trimmed++;
    }

    // Only consider it a return with value if the fragment starts with "return" and has a value
    if (strncmp(trimmed, "return", 6) == 0) {
        const char *after_return = trimmed + 6;
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
    const char *return_type = "void";
    const char *default_return = "    return ();\n";

    // Wrap the fragment in a minimal valid Asthra program
    // Create the wrapper program piece by piece
    size_t pos = 0;
    pos += snprintf(buffer + pos, buffer_size - pos, "package test;\n\n");

    // Add all the necessary declarations
    add_function_declarations(buffer, &pos, buffer_size);
    add_struct_declarations(buffer, &pos, buffer_size);

    // Create test function with proper return type
    pos += snprintf(buffer + pos, buffer_size - pos, "pub fn test_function(none) -> %s {\n",
                    return_type);

    // Add variable declarations
    add_variable_declarations(buffer, &pos, buffer_size);

    // Add the test fragment with expression-oriented transformations
    pos += snprintf(buffer + pos, buffer_size - pos, "    // Test fragment\n");
    pos += snprintf(buffer + pos, buffer_size - pos, "    ");

    // Apply expression-oriented transformations
    char transformed_fragment[4096];
    wrap_fragment_for_expression_context(fragment, transformed_fragment,
                                         sizeof(transformed_fragment));

    // Transformation successful
    pos += snprintf(buffer + pos, buffer_size - pos, "%s", transformed_fragment);

    // Ensure fragment is treated as a statement
    size_t frag_len = strlen(transformed_fragment);
    if (frag_len > 0 && transformed_fragment[frag_len - 1] != ';') {
        // Check if it's an if-else expression (ends with })
        if (strstr(transformed_fragment, "if") && strstr(transformed_fragment, "else") &&
            transformed_fragment[frag_len - 1] == '}') {
            pos += snprintf(buffer + pos, buffer_size - pos, ";");
        } else if (transformed_fragment[frag_len - 1] != '}') {
            // Other expressions need semicolons too
            pos += snprintf(buffer + pos, buffer_size - pos, ";");
        }
    }
    pos += snprintf(buffer + pos, buffer_size - pos, "\n    \n");

    // Add default return if needed
    pos += snprintf(buffer + pos, buffer_size - pos, "%s", default_return);
    pos += snprintf(buffer + pos, buffer_size - pos, "}\n");

    // Debug: write to file to check line numbers
    FILE *debug_file = fopen("/tmp/test_debug.asthra", "w");
    if (debug_file) {
        fprintf(debug_file, "%s", buffer);
        fclose(debug_file);
    }

    // Parse the complete program
    ASTNode *program = parse_test_source(buffer, "test.asthra");

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
    ASTNode *test_function = NULL;
    ASTNodeList *declarations = program->data.program.declarations;
    if (declarations) {
        size_t decl_count = ast_node_list_size(declarations);
        for (size_t i = 0; i < decl_count; i++) {
            ASTNode *decl = ast_node_list_get(declarations, i);
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
    ASTNode *body = test_function->data.function_decl.body;
    if (!body || body->type != AST_BLOCK) {
        ast_free_node(program);
        printf("ERROR: Test function has no body for %s\n", test_name);
        return NULL;
    }

    // Find the actual test statement - it should be the last statement added
    ASTNode *test_statement = NULL;
    ASTNodeList *statements = body->data.block.statements;
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

// Helper function to add function declarations
static void add_function_declarations(char *buffer, size_t *pos, size_t buffer_size) {
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "// Common test functions\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn action1(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn action2(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn action3(none) -> void { return (); }\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos, "pub fn body(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn process(n: int) -> void { return (); }\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos, "pub fn update(none) -> void { return (); }\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos, "pub fn action(none) -> void { return (); }\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos, "pub fn func(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn nested_func(a: int, b: int) -> int { return a + b; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn get_value(none) -> int { return 42; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn initialize(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn cleanup_resources(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn print_debug_info(level: int) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn update_display(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn save_state(none) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn print(s: string, value: int) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn printf(s: string) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn calculate(x: int, y: int, z: int) -> int { return x + y + z; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn compute_index(none) -> int { return 0; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn timestamp(none) -> int { return 0; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn transform(m: int, s: int) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn log(level: int, msg: string) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn validate(input: string, pattern: string) -> bool { return true; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn getObject(none) -> TestObject { return TestObject { member: 0 }; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn handle_error(none) -> bool { return false; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "\n");
}

// Helper function to add struct declarations
static void add_struct_declarations(char *buffer, size_t *pos, size_t buffer_size) {
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "// Test structures\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "pub struct TestObject {\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    pub member: int,\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    pub property: TestProperty\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "}\n\n");

    *pos += snprintf(buffer + *pos, buffer_size - *pos, "pub struct TestProperty {\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    pub value: int\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "}\n\n");

    // Remove impl blocks - not supported in Asthra
    // Instead, use regular functions that take the struct as first parameter
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn TestObject_method(obj: TestObject, p: int) -> void { return (); }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn TestObject_method1(obj: TestObject) -> TestObject { return obj; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn TestObject_method2(obj: TestObject) -> TestObject { return obj; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn TestObject_method3(obj: TestObject) -> void { return (); }\n\n");

    *pos += snprintf(buffer + *pos, buffer_size - *pos, "pub struct Module { pub field: int }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn Module_function(m: Module, p: int) -> void { return (); }\n\n");

    *pos += snprintf(buffer + *pos, buffer_size - *pos, "pub struct Class { pub field: int }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn Class_staticMethod(c: Class, p: int) -> void { return (); }\n\n");

    // Add helper functions
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn func1(a: int) -> int { return a * 2; }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn func2(b: int) -> int { return b + 10; }\n\n");

    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub struct Transformation { pub data: int }\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "pub fn Transformation_scale(t: Transformation) -> int { return 2; }\n\n");
}

// Helper function to add variable declarations
static void add_variable_declarations(char *buffer, size_t *pos, size_t buffer_size) {
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    // Test variables\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut x: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut y: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut z: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut i: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut j: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut a: int = 1;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut b: int = 2;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut c: int = 3;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut d: int = 4;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut e: int = 5;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut f: int = 6;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut m: int = 7;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut n: int = 10;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut p: int = 11;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut q: int = 12;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut r: int = 13;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut s: int = 14;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut t: int = 15;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut u: int = 16;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut rows: int = 5;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut cols: int = 5;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut start: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut end: int = 10;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let mut valid: bool = true;\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos, "    let mut arr: []int = [1, 2, 3, 4, 5];\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let condition: bool = true;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let condition1: bool = true;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let condition2: bool = false;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let outer_condition: bool = true;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let inner_condition: bool = true;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let outer_loop: bool = true;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let value: int = 42;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let args: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let index: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let array: []int = [1, 2, 3];\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let row: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let col: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let offset: int = 5;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let base: int = 100;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let divisor: int = 10;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let counter: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let flag: bool = true;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let true_val: int = 1;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let false_val: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let output: string = \"output\";\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let matrix: []int = [1, 2, 3, 4];\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let transformation: Transformation = Transformation { data: 1 };\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let variable: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let mut object: TestObject = TestObject { member: 0, property: "
                     "TestProperty { value: 42 } };\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let mut obj: TestObject = TestObject { member: 0, property: TestProperty "
                     "{ value: 42 } };\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let module: Module = Module { field: 0 };\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let param: int = 42;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let debug_level: int = 1;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let format_string: string = \"format\";\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let value1: int = 1;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let value2: int = 2;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let other: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let scale_factor: int = 1;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let level: int = 1;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let message: string = \"msg\";\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let user: TestObject = TestObject { member: 0, property: TestProperty { "
                     "value: 42 } };\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let input: string = \"input\";\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos,
                     "    let regex_pattern: string = \"pattern\";\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos,
                 "    let instance: TestObject = TestObject { member: 0, property: TestProperty "
                 "{ value: 42 } };\n");
    *pos +=
        snprintf(buffer + *pos, buffer_size - *pos, "    let struct_ptr: *TestObject = &object;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let pointer: *int = &variable;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    let cast_type: int = 0;\n");
    *pos += snprintf(buffer + *pos, buffer_size - *pos, "    \n");
}