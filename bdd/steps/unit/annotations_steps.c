#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "bdd_support.h"

// External functions from common_steps.c
extern void given_asthra_compiler_available(void);
extern void given_file_with_content(const char* filename, const char* content);
extern void when_compile_file(void);
extern void when_run_executable(void);
extern void then_compilation_should_succeed(void);
extern void then_compilation_should_fail(void);
extern void then_executable_created(void);
extern void then_output_contains(const char* expected_output);
extern void then_exit_code_is(int expected_code);
extern void then_error_contains(const char* expected_error);
extern void common_cleanup(void);

// Test scenario: Human review annotation on function
void test_human_review_annotation(void) {
    bdd_scenario("Human review annotation on function");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[human_review(high)]\n"
        "pub fn critical_operation(none) -> void {\n"
        "    log(\"Critical operation needs human review\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    critical_operation();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("human_review_function.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Critical operation needs human review");
    then_exit_code_is(0);
}

// Test scenario: Multiple human review levels
void test_multiple_review_levels(void) {
    bdd_scenario("Multiple human review levels");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[human_review(low)]\n"
        "pub fn simple_task(none) -> void {\n"
        "    log(\"Low priority review\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "#[human_review(medium)]\n"
        "pub fn normal_task(none) -> void {\n"
        "    log(\"Medium priority review\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "#[human_review(high)]\n"
        "pub fn critical_task(none) -> void {\n"
        "    log(\"High priority review\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    simple_task();\n"
        "    normal_task();\n"
        "    critical_task();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("review_levels.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Low priority review");
    then_output_contains("Medium priority review");
    then_output_contains("High priority review");
    then_exit_code_is(0);
}

// Test scenario: Security annotation - constant time
void test_constant_time_annotation(void) {
    bdd_scenario("Security annotation - constant time");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[constant_time]\n"
        "pub fn secure_compare(a: i32, b: i32) -> bool {\n"
        "    return a == b;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let result: bool = secure_compare(42, 42);\n"
        "    log(\"Constant time comparison\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("constant_time.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Constant time comparison");
    then_exit_code_is(0);
}

// Test scenario: Security annotation - volatile memory
void test_volatile_memory_annotation(void) {
    bdd_scenario("Security annotation - volatile memory");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[volatile_memory]\n"
        "pub fn secure_clear(none) -> void {\n"
        "    log(\"Clearing sensitive memory\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    secure_clear();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("volatile_memory.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Clearing sensitive memory");
    then_exit_code_is(0);
}

// Test scenario: Ownership annotation on struct
void test_ownership_annotation(void) {
    bdd_scenario("Ownership annotation on struct");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[ownership(gc)]\n"
        "pub struct ManagedData {\n"
        "    value: i32\n"
        "}\n"
        "\n"
        "#[ownership(c)]\n"
        "pub struct UnmanagedData {\n"
        "    ptr: *mut i32\n"
        "}\n"
        "\n"
        "#[ownership(pinned)]\n"
        "pub struct PinnedData {\n"
        "    buffer: [1024]u8\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Ownership annotations work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("ownership_struct.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Ownership annotations work");
    then_exit_code_is(0);
}

// Test scenario: FFI transfer annotations
void test_ffi_transfer_annotations(void) {
    bdd_scenario("FFI transfer annotations");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "extern \"C\" fn external_take_ownership(ptr: #[transfer_full] *mut i32) -> void;\n"
        "extern \"C\" fn external_borrow(ptr: #[transfer_none] *const i32) -> void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"FFI transfer annotations work\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("ffi_transfer.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("FFI transfer annotations work");
    then_exit_code_is(0);
}

// Test scenario: Borrowed reference annotation
void test_borrowed_annotation(void) {
    bdd_scenario("Borrowed reference annotation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "extern \"C\" fn use_borrowed(data: #[borrowed] *const i32) -> void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Borrowed reference annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("borrowed_ref.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Borrowed reference annotation works");
    then_exit_code_is(0);
}

// Test scenario: Non-deterministic annotation
void test_non_deterministic_annotation(void) {
    bdd_scenario("Non-deterministic annotation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[non_deterministic]\n"
        "pub fn random_value(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let val: i32 = random_value();\n"
        "    log(\"Non-deterministic annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("non_deterministic.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Non-deterministic annotation works");
    then_exit_code_is(0);
}

// Test scenario: Generic semantic annotation
void test_generic_annotation(void) {
    bdd_scenario("Generic semantic annotation");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[deprecated(reason = \"Use new_function instead\")]\n"
        "pub fn old_function(none) -> void {\n"
        "    log(\"Deprecated function\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "#[performance(critical = \"true\", cache_aligned = \"true\")]\n"
        "pub fn hot_path(none) -> void {\n"
        "    log(\"Performance critical\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    old_function();\n"
        "    hot_path();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("generic_annotation.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Deprecated function");
    then_output_contains("Performance critical");
    then_exit_code_is(0);
}

// Test scenario: Multiple annotations on same element
void test_multiple_annotations(void) {
    bdd_scenario("Multiple annotations on same element");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[human_review(high)]\n"
        "#[constant_time]\n"
        "#[performance(critical = \"true\")]\n"
        "pub fn secure_critical_operation(none) -> void {\n"
        "    log(\"Multiple annotations\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    secure_critical_operation();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("multiple_annotations.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Multiple annotations");
    then_exit_code_is(0);
}

// Test scenario: Invalid human review level
void test_invalid_review_level(void) {
    bdd_scenario("Invalid human review level");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[human_review(extreme)]\n"
        "pub fn some_function(none) -> void {\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("invalid_review_level.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("invalid review level");
}

// Test scenario: Invalid ownership type
void test_invalid_ownership(void) {
    bdd_scenario("Invalid ownership type");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[ownership(manual)]\n"
        "pub struct Data {\n"
        "    value: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("invalid_ownership.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("invalid ownership type");
}

// Test scenario: Annotation with none parameters
void test_annotation_none_params(void) {
    bdd_scenario("Annotation with none parameters");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "#[custom(none)]\n"
        "pub fn annotated_function(none) -> void {\n"
        "    log(\"Annotation with none params\");\n"
        "    return ();\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    annotated_function();\n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("annotation_none_params.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Annotation with none params");
    then_exit_code_is(0);
}

// Main test runner
int main(void) {
    bdd_init("Annotations");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios
        bdd_skip_scenario("Human review annotation on function [@wip]");
        bdd_skip_scenario("Multiple human review levels [@wip]");
        bdd_skip_scenario("Security annotation - constant time [@wip]");
        bdd_skip_scenario("Security annotation - volatile memory [@wip]");
        bdd_skip_scenario("FFI transfer annotations [@wip]");
        bdd_skip_scenario("Borrowed reference annotation [@wip]");
        bdd_skip_scenario("Generic semantic annotation [@wip]");
        bdd_skip_scenario("Multiple annotations on same element [@wip]");
        bdd_skip_scenario("Invalid human review level [@wip]");
        bdd_skip_scenario("Invalid ownership type [@wip]");
        bdd_skip_scenario("Annotation with none parameters [@wip]");
        
        // Run only non-@wip scenarios
        test_ownership_annotation();
        test_non_deterministic_annotation();
    } else {
        // Run all scenarios from annotations.feature
        test_human_review_annotation();
        test_multiple_review_levels();
        test_constant_time_annotation();
        test_volatile_memory_annotation();
        test_ownership_annotation();
        test_ffi_transfer_annotations();
        test_borrowed_annotation();
        test_non_deterministic_annotation();
        test_generic_annotation();
        test_multiple_annotations();
        test_invalid_review_level();
        test_invalid_ownership();
        test_annotation_none_params();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}