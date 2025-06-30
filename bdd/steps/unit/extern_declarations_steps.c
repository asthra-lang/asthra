#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios for extern declarations

void test_simple_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn printf(format: *const char) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Extern declaration works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple extern function declaration",
                               "simple_extern.asthra",
                               source,
                               "Extern declaration works",
                               0);
}

void test_multiple_params_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn strcmp(s1: *const char, s2: *const char) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Multiple param extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with multiple parameters",
                               "multiple_params_extern.asthra",
                               source,
                               "Multiple param extern works",
                               0);
}

void test_no_params_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn get_errno(none) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"No param extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with no parameters",
                               "no_params_extern.asthra",
                               source,
                               "No param extern works",
                               0);
}

void test_void_return_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn exit(status: i32) -> void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Void return extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function returning void",
                               "void_return_extern.asthra",
                               source,
                               "Void return extern works",
                               0);
}

void test_library_spec_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern \"C\" fn malloc(size: usize) -> *mut void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Library spec extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with library name",
                               "library_spec_extern.asthra",
                               source,
                               "Library spec extern works",
                               0);
}

void test_transfer_full_annotation(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn strdup(#[transfer_full] s: *const char) -> *mut char;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Transfer full annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with transfer_full annotation",
                               "transfer_full_annotation.asthra",
                               source,
                               "Transfer full annotation works",
                               0);
}

void test_transfer_none_annotation(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn strlen(#[transfer_none] s: *const char) -> usize;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Transfer none annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with transfer_none annotation",
                               "transfer_none_annotation.asthra",
                               source,
                               "Transfer none annotation works",
                               0);
}

void test_borrowed_annotation(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn fopen(#[borrowed] filename: *const char, #[borrowed] mode: *const char) -> *mut void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Borrowed annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with borrowed annotation",
                               "borrowed_annotation.asthra",
                               source,
                               "Borrowed annotation works",
                               0);
}

void test_return_annotation(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn create_buffer(size: usize) -> #[transfer_full] *mut u8;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Return annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Return type with FFI annotation",
                               "return_annotation.asthra",
                               source,
                               "Return annotation works",
                               0);
}

void test_pointer_types_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn memcpy(dest: *mut void, src: *const void, n: usize) -> *mut void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Pointer types extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with pointer types",
                               "pointer_types_extern.asthra",
                               source,
                               "Pointer types extern works",
                               0);
}

void test_numeric_types_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn process_numbers(a: i8, b: u16, c: i32, d: u64, e: f32, f: f64) -> i64;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Numeric types extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with various numeric types",
                               "numeric_types_extern.asthra",
                               source,
                               "Numeric types extern works",
                               0);
}

void test_private_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "priv extern fn internal_helper(x: i32) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Private extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Private extern function",
                               "private_extern.asthra",
                               source,
                               "Private extern works",
                               0);
}

void test_public_extern(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn public_api_call(data: *const u8, len: usize) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Public extern works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Public extern function",
                               "public_extern.asthra",
                               source,
                               "Public extern works",
                               0);
}

void test_extern_no_return_type(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn bad_extern(x: i32);\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Extern function without return type",
                                 "extern_no_return_type.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "return type");
}

void test_extern_invalid_param(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn bad_param(x) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Extern function with invalid parameter",
                                 "extern_invalid_param.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "type");
}

void test_multiple_ffi_annotations(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn conflicting(#[transfer_full] #[borrowed] ptr: *mut void) -> void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Should not compile\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Multiple FFI annotations on same parameter",
                                 "multiple_ffi_annotations.asthra",
                                 source,
                                 0,  // should_succeed = false
                                 "annotation");
}

void test_multiple_externs(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn open(path: *const char, flags: i32) -> i32;\n"
        "pub extern fn read(fd: i32, buf: *mut void, count: usize) -> isize;\n"
        "pub extern fn close(fd: i32) -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Multiple externs work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Multiple extern declarations",
                               "multiple_externs.asthra",
                               source,
                               "Multiple externs work",
                               0);
}

void test_mixed_annotations(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub extern fn process_data(\n"
        "    #[borrowed] input: *const u8,\n"
        "    input_len: usize,\n"
        "    #[transfer_full] output: *mut u8,\n"
        "    output_len: usize\n"
        ") -> i32;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Mixed annotations work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Extern function with mixed annotations",
                               "mixed_annotations.asthra",
                               source,
                               "Mixed annotations work",
                               0);
}

// Define test cases
BddTestCase extern_declarations_test_cases[] = {
    BDD_TEST_CASE(simple_extern, test_simple_extern),
    BDD_TEST_CASE(multiple_params_extern, test_multiple_params_extern),
    BDD_TEST_CASE(no_params_extern, test_no_params_extern),
    BDD_TEST_CASE(void_return_extern, test_void_return_extern),
    BDD_TEST_CASE(library_spec_extern, test_library_spec_extern),
    BDD_TEST_CASE(transfer_full_annotation, test_transfer_full_annotation),
    BDD_TEST_CASE(transfer_none_annotation, test_transfer_none_annotation),
    BDD_TEST_CASE(borrowed_annotation, test_borrowed_annotation),
    BDD_TEST_CASE(return_annotation, test_return_annotation),
    BDD_TEST_CASE(pointer_types_extern, test_pointer_types_extern),
    BDD_TEST_CASE(numeric_types_extern, test_numeric_types_extern),
    BDD_TEST_CASE(private_extern, test_private_extern),
    BDD_TEST_CASE(public_extern, test_public_extern),
    BDD_WIP_TEST_CASE(extern_no_return_type, test_extern_no_return_type),
    BDD_TEST_CASE(extern_invalid_param, test_extern_invalid_param),
    BDD_TEST_CASE(multiple_ffi_annotations, test_multiple_ffi_annotations),
    BDD_TEST_CASE(multiple_externs, test_multiple_externs),
    BDD_TEST_CASE(mixed_annotations, test_mixed_annotations),
};

// Main test runner
int main(void) {
    return bdd_run_test_suite("Extern Declarations",
                              extern_declarations_test_cases,
                              sizeof(extern_declarations_test_cases) / sizeof(extern_declarations_test_cases[0]),
                              bdd_cleanup_temp_files);
}