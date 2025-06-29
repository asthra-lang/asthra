#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios using the new reusable framework

void test_human_review_annotation(void) {
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
    
    bdd_run_execution_scenario("Human review annotation on function",
                               "human_review_function.asthra",
                               source,
                               "Critical operation needs human review",
                               0);
}

void test_multiple_review_levels(void) {
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
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("Multiple human review levels");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"review_levels.asthra\" with content");
    bdd_create_temp_source_file("review_levels.asthra", source);
    
    bdd_when("I compile the file");
    char* executable = strdup(bdd_get_temp_source_file());
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    int exit_code = bdd_compile_source_file(bdd_get_temp_source_file(), executable, NULL);
    
    bdd_then("the compilation should succeed");
    BDD_ASSERT_EQ(exit_code, 0);
    
    bdd_then("an executable should be created");
    struct stat st;
    int exists = (stat(executable, &st) == 0);
    BDD_ASSERT_TRUE(exists);
    
    bdd_when("I run the executable");
    char command[512];
    snprintf(command, sizeof(command), "./%s 2>&1", executable);
    
    int execution_exit_code;
    char* execution_output = bdd_execute_command(command, &execution_exit_code);
    
    bdd_then("the output should contain \"Low priority review\"");
    bdd_assert_output_contains(execution_output, "Low priority review");
    
    bdd_then("the output should contain \"Medium priority review\"");
    bdd_assert_output_contains(execution_output, "Medium priority review");
    
    bdd_then("the output should contain \"High priority review\"");
    bdd_assert_output_contains(execution_output, "High priority review");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_constant_time_annotation(void) {
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
    
    bdd_run_execution_scenario("Security annotation - constant time",
                               "constant_time.asthra",
                               source,
                               "Constant time comparison",
                               0);
}

void test_volatile_memory_annotation(void) {
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
    
    bdd_run_execution_scenario("Security annotation - volatile memory",
                               "volatile_memory.asthra",
                               source,
                               "Clearing sensitive memory",
                               0);
}

void test_ownership_annotation(void) {
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
    
    bdd_run_execution_scenario("Ownership annotation on struct",
                               "ownership_struct.asthra",
                               source,
                               "Ownership annotations work",
                               0);
}

void test_ffi_transfer_annotations(void) {
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
    
    bdd_run_execution_scenario("FFI transfer annotations",
                               "ffi_transfer.asthra",
                               source,
                               "FFI transfer annotations work",
                               0);
}

void test_borrowed_annotation(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "extern \"C\" fn use_borrowed(data: #[borrowed] *const i32) -> void;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Borrowed reference annotation works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Borrowed reference annotation",
                               "borrowed_ref.asthra",
                               source,
                               "Borrowed reference annotation works",
                               0);
}

void test_non_deterministic_annotation(void) {
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
    
    bdd_run_execution_scenario("Non-deterministic annotation",
                               "non_deterministic.asthra",
                               source,
                               "Non-deterministic annotation works",
                               0);
}

void test_generic_annotation(void) {
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
    
    // For multiple output checks, we'll use the detailed scenario pattern
    bdd_scenario("Generic semantic annotation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"generic_annotation.asthra\" with content");
    bdd_create_temp_source_file("generic_annotation.asthra", source);
    
    bdd_when("I compile the file");
    char* executable = strdup(bdd_get_temp_source_file());
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    int exit_code = bdd_compile_source_file(bdd_get_temp_source_file(), executable, NULL);
    
    bdd_then("the compilation should succeed");
    BDD_ASSERT_EQ(exit_code, 0);
    
    bdd_then("an executable should be created");
    struct stat st;
    int exists = (stat(executable, &st) == 0);
    BDD_ASSERT_TRUE(exists);
    
    bdd_when("I run the executable");
    char command[512];
    snprintf(command, sizeof(command), "./%s 2>&1", executable);
    
    int execution_exit_code;
    char* execution_output = bdd_execute_command(command, &execution_exit_code);
    
    bdd_then("the output should contain \"Deprecated function\"");
    bdd_assert_output_contains(execution_output, "Deprecated function");
    
    bdd_then("the output should contain \"Performance critical\"");
    bdd_assert_output_contains(execution_output, "Performance critical");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_multiple_annotations(void) {
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
    
    bdd_run_execution_scenario("Multiple annotations on same element",
                               "multiple_annotations.asthra",
                               source,
                               "Multiple annotations",
                               0);
}

void test_invalid_review_level(void) {
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
    
    bdd_run_compilation_scenario("Invalid human review level",
                                 "invalid_review_level.asthra",
                                 source,
                                 0,  // should fail
                                 "invalid review level");
}

void test_invalid_ownership(void) {
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
    
    bdd_run_compilation_scenario("Invalid ownership type",
                                 "invalid_ownership.asthra",
                                 source,
                                 0,  // should fail
                                 "invalid ownership type");
}

void test_annotation_none_params(void) {
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
    
    bdd_run_execution_scenario("Annotation with none parameters",
                               "annotation_none_params.asthra",
                               source,
                               "Annotation with none params",
                               0);
}

// Define test cases using the new framework - @wip tags based on original file
BddTestCase annotations_test_cases[] = {
    BDD_WIP_TEST_CASE(human_review_annotation, test_human_review_annotation),
    BDD_WIP_TEST_CASE(multiple_review_levels, test_multiple_review_levels),
    BDD_WIP_TEST_CASE(constant_time_annotation, test_constant_time_annotation),
    BDD_WIP_TEST_CASE(volatile_memory_annotation, test_volatile_memory_annotation),
    BDD_TEST_CASE(ownership_annotation, test_ownership_annotation),
    BDD_WIP_TEST_CASE(ffi_transfer_annotations, test_ffi_transfer_annotations),
    BDD_WIP_TEST_CASE(borrowed_annotation, test_borrowed_annotation),
    BDD_TEST_CASE(non_deterministic_annotation, test_non_deterministic_annotation),
    BDD_WIP_TEST_CASE(generic_annotation, test_generic_annotation),
    BDD_WIP_TEST_CASE(multiple_annotations, test_multiple_annotations),
    BDD_WIP_TEST_CASE(invalid_review_level, test_invalid_review_level),
    BDD_WIP_TEST_CASE(invalid_ownership, test_invalid_ownership),
    BDD_WIP_TEST_CASE(annotation_none_params, test_annotation_none_params),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Annotations",
                              annotations_test_cases,
                              sizeof(annotations_test_cases) / sizeof(annotations_test_cases[0]),
                              bdd_cleanup_temp_files);
}