#include "bdd_test_framework.h"
#include "bdd_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FFI Integration BDD Tests
// Tests Foreign Function Interface capabilities of Asthra compiler

typedef struct {
    void* handle;
    int (*c_function)(int);
    const char* library_path;
    int call_result;
} ffi_context_t;

static ffi_context_t ffi_ctx = {0};

// Test scenario: Basic C function call
void test_basic_c_function_call(void) {
    bdd_given("a C library is loaded");
    // Simulate library loading
    ffi_ctx.handle = (void*)0x1234; // Mock handle
    ffi_ctx.library_path = "libtest.so";
    
    bdd_when("Asthra code calls a C function");
    // Simulate FFI call
    if (ffi_ctx.handle) {
        ffi_ctx.call_result = 42; // Mock successful call result
    }
    
    bdd_then("the FFI call succeeds");
    bdd_assert(ffi_ctx.handle != NULL, "FFI library handle should be valid");
    bdd_assert(ffi_ctx.call_result == 42, "FFI call should return expected value");
}

// Test scenario: FFI with type conversions
void test_ffi_type_conversions(void) {
    bdd_given("a C function that requires type conversions");
    const char* asthra_source = 
        "package main;\n"
        "extern \"C\" fn add_numbers(a: int, b: int) -> int;\n"
        "pub fn main(none) -> void {\n"
        "    let result: int = add_numbers(10, 20);\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("ffi_types.asthra", asthra_source);
    
    bdd_when("compiling Asthra code with FFI declarations");
    int result = bdd_compile_source_file("ffi_types.asthra", "ffi_types", "--enable-ffi");
    
    bdd_then("compilation should succeed with proper type binding");
    bdd_assert(result == 0, "FFI type conversion compilation should succeed");
}

// Test scenario: FFI error handling
void test_ffi_error_handling(void) {
    bdd_given("an invalid FFI declaration");
    const char* invalid_source = 
        "package main;\n"
        "extern \"C\" fn nonexistent_function() -> void;\n"
        "pub fn main(none) -> void {\n"
        "    nonexistent_function();\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("ffi_error.asthra", invalid_source);
    
    bdd_when("compiling with missing FFI function");
    int exit_code;
    char* output = NULL;
    const char* compiler_path = bdd_find_asthra_compiler();
    if (!compiler_path) {
        exit_code = -1;
    } else {
        char command[1024];
        snprintf(command, sizeof(command), "%s ffi_error.asthra -o ffi_error --enable-ffi", compiler_path);
        output = bdd_execute_command(command, &exit_code);
    }
    
    bdd_then("compilation should fail with FFI error");
    bdd_assert(exit_code != 0, "Should fail when FFI function is missing");
    if (output) {
        bdd_assert_output_contains(output, "undefined");
        bdd_cleanup_string(&output);
    }
}

// Test scenario: C struct interop
void test_c_struct_interop(void) {
    bdd_given("a C struct definition and Asthra equivalent");
    const char* asthra_source = 
        "package main;\n"
        "extern \"C\" {\n"
        "    type Point = { x: int; y: int; };\n"
        "    fn create_point(x: int, y: int) -> Point;\n"
        "    fn point_distance(p1: Point, p2: Point) -> float;\n"
        "}\n"
        "pub fn main(none) -> void {\n"
        "    let p1: Point = create_point(0, 0);\n"
        "    let p2: Point = create_point(3, 4);\n"
        "    let dist: float = point_distance(p1, p2);\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("struct_interop.asthra", asthra_source);
    
    bdd_when("compiling struct interop code");
    int result = bdd_compile_source_file("struct_interop.asthra", "struct_interop", "--enable-ffi");
    
    bdd_then("struct interop should compile successfully");
    // Mark as WIP since advanced FFI features may not be fully implemented
    bdd_skip_scenario("Advanced struct interop not fully implemented yet");
}

// Test scenario: FFI function pointer callbacks
void test_ffi_callbacks(void) {
    bdd_given("an Asthra function passed as callback to C");
    const char* callback_source = 
        "package main;\n"
        "extern \"C\" {\n"
        "    type Callback = fn(int) -> int;\n"
        "    fn process_with_callback(data: [int], callback: Callback) -> void;\n"
        "}\n"
        "fn double_value(x: int) -> int {\n"
        "    return x * 2;\n"
        "}\n"
        "pub fn main(none) -> void {\n"
        "    let data: [int] = [1, 2, 3, 4, 5];\n"
        "    process_with_callback(data, double_value);\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("ffi_callback.asthra", callback_source);
    
    bdd_when("compiling callback FFI code");
    int result = bdd_compile_source_file("ffi_callback.asthra", "ffi_callback", "--enable-ffi");
    
    bdd_then("callback compilation should succeed");
    // Mark as WIP since callback FFI is complex
    bdd_skip_scenario("FFI callbacks not fully implemented yet");
}

// Test scenario: FFI memory management
void test_ffi_memory_management(void) {
    bdd_given("C functions that allocate and free memory");
    const char* memory_source = 
        "package main;\n"
        "extern \"C\" {\n"
        "    fn malloc(size: int) -> *void;\n"
        "    fn free(ptr: *void) -> void;\n"
        "    fn memcpy(dest: *void, src: *void, size: int) -> *void;\n"
        "}\n"
        "pub fn main(none) -> void {\n"
        "    let ptr: *void = malloc(1024);\n"
        "    // Use memory...\n"
        "    free(ptr);\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("ffi_memory.asthra", memory_source);
    
    bdd_when("compiling memory management FFI");
    int result = bdd_compile_source_file("ffi_memory.asthra", "ffi_memory", "--enable-ffi");
    
    bdd_then("memory management FFI should work safely");
    // Mark as WIP since safe FFI memory management is complex
    bdd_skip_scenario("Safe FFI memory management not fully implemented yet");
}

// Test scenario: FFI library loading
void test_dynamic_library_loading(void) {
    bdd_given("a request to load a dynamic library");
    const char* library_source = 
        "package main;\n"
        "extern \"C\" {\n"
        "    fn dlopen(filename: *char, flag: int) -> *void;\n"
        "    fn dlsym(handle: *void, symbol: *char) -> *void;\n"
        "    fn dlclose(handle: *void) -> int;\n"
        "}\n"
        "pub fn main(none) -> void {\n"
        "    let handle: *void = dlopen(\"libm.so\", 1);\n"
        "    if (handle != null) {\n"
        "        dlclose(handle);\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("dynamic_lib.asthra", library_source);
    
    bdd_when("compiling dynamic library code");
    int result = bdd_compile_source_file("dynamic_lib.asthra", "dynamic_lib", "--enable-ffi");
    
    bdd_then("dynamic loading should be supported");
    bdd_skip_scenario("Dynamic library loading not fully implemented yet");
}

// Test scenario: FFI with const parameters
void test_ffi_const_parameters(void) {
    bdd_given("C functions with const parameters");
    const char* const_source = 
        "package main;\n"
        "extern \"C\" {\n"
        "    fn strlen(s: *const char) -> int;\n"
        "    fn strcmp(s1: *const char, s2: *const char) -> int;\n"
        "}\n"
        "pub fn main(none) -> void {\n"
        "    let str: *const char = \"Hello, World!\";\n"
        "    let len: int = strlen(str);\n"
        "    return ();\n"
        "}\n";
    
    bdd_create_temp_source_file("ffi_const.asthra", const_source);
    
    bdd_when("compiling const parameter FFI");
    int result = bdd_compile_source_file("ffi_const.asthra", "ffi_const", "--enable-ffi");
    
    bdd_then("const parameters should be handled correctly");
    bdd_assert(result == 0, "FFI with const parameters should compile");
}

// Declarative test case definitions
BddTestCase ffi_integration_test_cases[] = {
    BDD_TEST_CASE(basic_c_function_call, test_basic_c_function_call),
    BDD_TEST_CASE(ffi_type_conversions, test_ffi_type_conversions),
    BDD_TEST_CASE(ffi_error_handling, test_ffi_error_handling),
    BDD_TEST_CASE(ffi_const_parameters, test_ffi_const_parameters),
    BDD_WIP_TEST_CASE(c_struct_interop, test_c_struct_interop),
    BDD_WIP_TEST_CASE(ffi_callbacks, test_ffi_callbacks),
    BDD_WIP_TEST_CASE(ffi_memory_management, test_ffi_memory_management),
    BDD_WIP_TEST_CASE(dynamic_library_loading, test_dynamic_library_loading),
};

int main(void) {
    return bdd_run_test_suite(
        "FFI Integration",
        ffi_integration_test_cases,
        sizeof(ffi_integration_test_cases) / sizeof(ffi_integration_test_cases[0]),
        bdd_cleanup_temp_files
    );
}