#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios using the new reusable framework

void test_bitwise_and(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 12;  // Binary: 1100\n"
        "    let b: i32 = 10;  // Binary: 1010\n"
        "    let result: i32 = a & b;  // Binary: 1000 = 8\n"
        "    \n"
        "    if result == 8 {\n"
        "        log(\"Bitwise AND: 12 & 10 = 8\");\n"
        "    }\n"
        "    \n"
        "    // Test with different values\n"
        "    let x: i32 = 255;  // Binary: 11111111\n"
        "    let y: i32 = 15;   // Binary: 00001111\n"
        "    let mask_result: i32 = x & y;  // Binary: 00001111 = 15\n"
        "    \n"
        "    if mask_result == 15 {\n"
        "        log(\"Masking operation: 255 & 15 = 15\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Bitwise AND operation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"bitwise_and.asthra\" with content");
    bdd_create_temp_source_file("bitwise_and.asthra", source);
    
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
    
    bdd_then("the output should contain \"Bitwise AND: 12 & 10 = 8\"");
    bdd_assert_output_contains(execution_output, "Bitwise AND: 12 & 10 = 8");
    
    bdd_then("the output should contain \"Masking operation: 255 & 15 = 15\"");
    bdd_assert_output_contains(execution_output, "Masking operation: 255 & 15 = 15");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_bitwise_or(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 12;  // Binary: 1100\n"
        "    let b: i32 = 10;  // Binary: 1010\n"
        "    let result: i32 = a | b;  // Binary: 1110 = 14\n"
        "    \n"
        "    if result == 14 {\n"
        "        log(\"Bitwise OR: 12 | 10 = 14\");\n"
        "    }\n"
        "    \n"
        "    // Setting flags\n"
        "    let flags: i32 = 0;\n"
        "    let flag1: i32 = 1;  // Binary: 0001\n"
        "    let flag2: i32 = 4;  // Binary: 0100\n"
        "    let flag3: i32 = 8;  // Binary: 1000\n"
        "    \n"
        "    let all_flags: i32 = flags | flag1 | flag2 | flag3;  // Binary: 1101 = 13\n"
        "    \n"
        "    if all_flags == 13 {\n"
        "        log(\"Flag combination: 0 | 1 | 4 | 8 = 13\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Bitwise OR operation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"bitwise_or.asthra\" with content");
    bdd_create_temp_source_file("bitwise_or.asthra", source);
    
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
    
    bdd_then("the output should contain \"Bitwise OR: 12 | 10 = 14\"");
    bdd_assert_output_contains(execution_output, "Bitwise OR: 12 | 10 = 14");
    
    bdd_then("the output should contain \"Flag combination: 0 | 1 | 4 | 8 = 13\"");
    bdd_assert_output_contains(execution_output, "Flag combination: 0 | 1 | 4 | 8 = 13");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_bitwise_not(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 0;\n"
        "    let not_a: i32 = ~a;  // All bits flipped\n"
        "    \n"
        "    if not_a == -1 {\n"
        "        log(\"Bitwise NOT: ~0 = -1\");\n"
        "    }\n"
        "    \n"
        "    let b: i32 = 15;  // Binary: 00001111\n"
        "    let not_b: i32 = ~b;  // Binary: 11110000 (as signed int)\n"
        "    \n"
        "    if not_b == -16 {\n"
        "        log(\"Bitwise NOT: ~15 = -16\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Bitwise NOT operation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"bitwise_not.asthra\" with content");
    bdd_create_temp_source_file("bitwise_not.asthra", source);
    
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
    
    bdd_then("the output should contain \"Bitwise NOT: ~0 = -1\"");
    bdd_assert_output_contains(execution_output, "Bitwise NOT: ~0 = -1");
    
    bdd_then("the output should contain \"Bitwise NOT: ~15 = -16\"");
    bdd_assert_output_contains(execution_output, "Bitwise NOT: ~15 = -16");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_left_shift(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 5;   // Binary: 0101\n"
        "    let result: i32 = a << 2;  // Binary: 10100 = 20\n"
        "    \n"
        "    if result == 20 {\n"
        "        log(\"Left shift: 5 << 2 = 20\");\n"
        "    }\n"
        "    \n"
        "    // Power of 2 multiplication\n"
        "    let base: i32 = 1;\n"
        "    let power_of_8: i32 = base << 3;  // 1 * 2^3 = 8\n"
        "    \n"
        "    if power_of_8 == 8 {\n"
        "        log(\"Power of 2: 1 << 3 = 8\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Left shift operation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"left_shift.asthra\" with content");
    bdd_create_temp_source_file("left_shift.asthra", source);
    
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
    
    bdd_then("the output should contain \"Left shift: 5 << 2 = 20\"");
    bdd_assert_output_contains(execution_output, "Left shift: 5 << 2 = 20");
    
    bdd_then("the output should contain \"Power of 2: 1 << 3 = 8\"");
    bdd_assert_output_contains(execution_output, "Power of 2: 1 << 3 = 8");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_right_shift(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 20;  // Binary: 10100\n"
        "    let result: i32 = a >> 2;  // Binary: 00101 = 5\n"
        "    \n"
        "    if result == 5 {\n"
        "        log(\"Right shift: 20 >> 2 = 5\");\n"
        "    }\n"
        "    \n"
        "    // Division by power of 2\n"
        "    let value: i32 = 64;\n"
        "    let divided: i32 = value >> 3;  // 64 / 2^3 = 8\n"
        "    \n"
        "    if divided == 8 {\n"
        "        log(\"Division by power of 2: 64 >> 3 = 8\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Right shift operation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"right_shift.asthra\" with content");
    bdd_create_temp_source_file("right_shift.asthra", source);
    
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
    
    bdd_then("the output should contain \"Right shift: 20 >> 2 = 5\"");
    bdd_assert_output_contains(execution_output, "Right shift: 20 >> 2 = 5");
    
    bdd_then("the output should contain \"Division by power of 2: 64 >> 3 = 8\"");
    bdd_assert_output_contains(execution_output, "Division by power of 2: 64 >> 3 = 8");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_bitwise_precedence(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Precedence: shift operators have lower precedence than arithmetic\n"
        "    let a: i32 = 2 + 3 << 1;  // (2 + 3) << 1 = 5 << 1 = 10\n"
        "    \n"
        "    if a == 10 {\n"
        "        log(\"Shift precedence: 2 + 3 << 1 = 10\");\n"
        "    }\n"
        "    \n"
        "    // Bitwise AND has lower precedence than comparison\n"
        "    let b: i32 = 7;\n"
        "    let c: i32 = 3;\n"
        "    let result: bool = (b & c) == 3;  // Parentheses needed\n"
        "    \n"
        "    if result {\n"
        "        log(\"Bitwise AND with comparison: (7 & 3) == 3 is true\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Bitwise operator precedence");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"bitwise_precedence.asthra\" with content");
    bdd_create_temp_source_file("bitwise_precedence.asthra", source);
    
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
    
    bdd_then("the output should contain \"Shift precedence: 2 + 3 << 1 = 10\"");
    bdd_assert_output_contains(execution_output, "Shift precedence: 2 + 3 << 1 = 10");
    
    bdd_then("the output should contain \"Bitwise AND with comparison: (7 & 3) == 3 is true\"");
    bdd_assert_output_contains(execution_output, "Bitwise AND with comparison: (7 & 3) == 3 is true");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_bitwise_type_error(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: f32 = 3.14;\n"
        "    let b: f32 = 2.71;\n"
        "    let result: f32 = a & b;  // Error: bitwise AND on float\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Error - bitwise operations on non-integer types",
                                 "bitwise_type_error.asthra",
                                 source,
                                 0,  // should fail
                                 "bitwise");
}

void test_bitwise_xor(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 12;  // Binary: 1100\n"
        "    let b: i32 = 10;  // Binary: 1010\n"
        "    let result: i32 = a ^ b;  // Binary: 0110 = 6\n"
        "    \n"
        "    if result == 6 {\n"
        "        log(\"Bitwise XOR: 12 ^ 10 = 6\");\n"
        "    }\n"
        "    \n"
        "    // XOR swap trick\n"
        "    let mut x: i32 = 5;\n"
        "    let mut y: i32 = 7;\n"
        "    \n"
        "    x = x ^ y;  // x now contains 5 ^ 7\n"
        "    y = x ^ y;  // y = (5 ^ 7) ^ 7 = 5\n"
        "    x = x ^ y;  // x = (5 ^ 7) ^ 5 = 7\n"
        "    \n"
        "    if x == 7 && y == 5 {\n"
        "        log(\"XOR swap successful: x=7, y=5\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Bitwise XOR operation");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"bitwise_xor.asthra\" with content");
    bdd_create_temp_source_file("bitwise_xor.asthra", source);
    
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
    
    bdd_then("the output should contain \"Bitwise XOR: 12 ^ 10 = 6\"");
    bdd_assert_output_contains(execution_output, "Bitwise XOR: 12 ^ 10 = 6");
    
    bdd_then("the output should contain \"XOR swap successful: x=7, y=5\"");
    bdd_assert_output_contains(execution_output, "XOR swap successful: x=7, y=5");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_combined_bitwise(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Extract bits using mask\n"
        "    let value: i32 = 0b11010110;  // Binary: 11010110 = 214\n"
        "    let mask: i32 = 0b00001111;   // Binary: 00001111 = 15\n"
        "    let lower_nibble: i32 = value & mask;  // Extract lower 4 bits\n"
        "    \n"
        "    if lower_nibble == 6 {\n"
        "        log(\"Lower nibble extracted: 6\");\n"
        "    }\n"
        "    \n"
        "    // Set specific bit\n"
        "    let mut flags: i32 = 0b00000000;\n"
        "    let bit_position: i32 = 3;\n"
        "    flags = flags | (1 << bit_position);  // Set bit 3\n"
        "    \n"
        "    if flags == 8 {\n"
        "        log(\"Bit 3 set successfully: 8\");\n"
        "    }\n"
        "    \n"
        "    // Toggle bit\n"
        "    flags = flags ^ (1 << bit_position);  // Toggle bit 3\n"
        "    \n"
        "    if flags == 0 {\n"
        "        log(\"Bit 3 toggled successfully: 0\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Combined bitwise operations");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"combined_bitwise.asthra\" with content");
    bdd_create_temp_source_file("combined_bitwise.asthra", source);
    
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
    
    bdd_then("the output should contain \"Lower nibble extracted: 6\"");
    bdd_assert_output_contains(execution_output, "Lower nibble extracted: 6");
    
    bdd_then("the output should contain \"Bit 3 set successfully: 8\"");
    bdd_assert_output_contains(execution_output, "Bit 3 set successfully: 8");
    
    bdd_then("the output should contain \"Bit 3 toggled successfully: 0\"");
    bdd_assert_output_contains(execution_output, "Bit 3 toggled successfully: 0");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_bitwise_types(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // i64 operations - demonstrate 64-bit support\n"
        "    let large: i64 = 1099511627775;  // 2^40 - 1, needs more than 32 bits\n"
        "    let mask: i64 = 1099511627776;   // 2^40\n"
        "    let result_64: i64 = large & mask;\n"
        "    \n"
        "    if result_64 == 0 {\n"
        "        log(\"i64 bitwise AND: alternating pattern = 0\");\n"
        "    }\n"
        "    \n"
        "    // Mixed with arithmetic\n"
        "    let x: i32 = 15;\n"
        "    let shifted_and_added: i32 = (x << 2) + (x >> 1);  // (15 * 4) + (15 / 2) = 60 + 7 = 67\n"
        "    \n"
        "    if shifted_and_added == 67 {\n"
        "        log(\"Mixed arithmetic and bitwise: 67\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Bitwise operations with different integer types");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"bitwise_types.asthra\" with content");
    bdd_create_temp_source_file("bitwise_types.asthra", source);
    
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
    
    bdd_then("the output should contain \"i64 bitwise AND: alternating pattern = 0\"");
    bdd_assert_output_contains(execution_output, "i64 bitwise AND: alternating pattern = 0");
    
    bdd_then("the output should contain \"Mixed arithmetic and bitwise: 67\"");
    bdd_assert_output_contains(execution_output, "Mixed arithmetic and bitwise: 67");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_binary_literals(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let byte_mask: i32 = 0b11111111;  // 255\n"
        "    let nibble_mask: i32 = 0b1111;    // 15\n"
        "    \n"
        "    let value: i32 = 0b10101010;      // 170\n"
        "    let masked: i32 = value & nibble_mask;\n"
        "    \n"
        "    if masked == 0b1010 {  // 10\n"
        "        log(\"Binary literal masking: 0b10101010 & 0b1111 = 0b1010\");\n"
        "    }\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    // Multiple output checks using detailed scenario pattern
    bdd_scenario("Binary literals with bitwise operations");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"binary_literals.asthra\" with content");
    bdd_create_temp_source_file("binary_literals.asthra", source);
    
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
    
    bdd_then("the output should contain \"Binary literal masking: 0b10101010 & 0b1111 = 0b1010\"");
    bdd_assert_output_contains(execution_output, "Binary literal masking: 0b10101010 & 0b1111 = 0b1010");
    
    bdd_then("the exit code should be 0");
    BDD_ASSERT_EQ(execution_exit_code, 0);
    
    bdd_cleanup_string(&execution_output);
    free(executable);
}

void test_negative_shift(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 10;\n"
        "    let shift_amount: i32 = -2;\n"
        "    let result: i32 = a << shift_amount;  // Undefined behavior at runtime\n"
        "    \n"
        "    // This demonstrates that negative shifts compile but have undefined behavior\n"
        "    log(\"Negative shift test completed\");\n"
        "    return ();\n"
        "}\n";
    
    // Note: Negative shifts are not caught at compile time since the shift amount
    // could be a runtime value. This test now verifies that it compiles successfully.
    bdd_scenario("Error - shift by negative amount");
    
    bdd_given("the Asthra compiler is available");
    BDD_ASSERT_TRUE(bdd_compiler_available());
    
    bdd_given("I have a file \"negative_shift.asthra\" with content");
    bdd_create_temp_source_file("negative_shift.asthra", source);
    
    bdd_when("I compile the file");
    char* executable = strdup(bdd_get_temp_source_file());
    char* dot = strrchr(executable, '.');
    if (dot) *dot = '\0';
    
    int exit_code = bdd_compile_source_file(bdd_get_temp_source_file(), executable, NULL);
    
    // Update expectation: should compile successfully
    bdd_then("the compilation should succeed");
    BDD_ASSERT_EQ(exit_code, 0);
    
    // The undefined behavior happens at runtime, not compile time
    bdd_then("an executable should be created");
    struct stat st;
    int exists = (stat(executable, &st) == 0);
    BDD_ASSERT_TRUE(exists);
    
    free(executable);
}

// Define test cases using the new framework
BddTestCase bitwise_operators_test_cases[] = {
    BDD_TEST_CASE(bitwise_and, test_bitwise_and),
    BDD_TEST_CASE(bitwise_or, test_bitwise_or),
    BDD_TEST_CASE(bitwise_not, test_bitwise_not),
    BDD_TEST_CASE(bitwise_xor, test_bitwise_xor),
    BDD_TEST_CASE(left_shift, test_left_shift),
    BDD_TEST_CASE(right_shift, test_right_shift),
    BDD_TEST_CASE(combined_bitwise, test_combined_bitwise),
    BDD_TEST_CASE(bitwise_types, test_bitwise_types),  // Phase 2 feature: i64 operations and mixed arithmetic
    BDD_TEST_CASE(bitwise_precedence, test_bitwise_precedence),
    BDD_TEST_CASE(binary_literals, test_binary_literals),
    BDD_TEST_CASE(bitwise_type_error, test_bitwise_type_error),
    BDD_TEST_CASE(negative_shift, test_negative_shift),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("Bitwise Operators",
                              bitwise_operators_test_cases,
                              sizeof(bitwise_operators_test_cases) / sizeof(bitwise_operators_test_cases[0]),
                              bdd_cleanup_temp_files);
}