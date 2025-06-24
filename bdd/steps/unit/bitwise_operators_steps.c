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

// Test scenario: Bitwise AND operation
void test_bitwise_and(void) {
    bdd_scenario("Bitwise AND operation");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bitwise_and.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Bitwise AND: 12 & 10 = 8");
    then_output_contains("Masking operation: 255 & 15 = 15");
    then_exit_code_is(0);
}

// Test scenario: Bitwise OR operation
void test_bitwise_or(void) {
    bdd_scenario("Bitwise OR operation");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bitwise_or.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Bitwise OR: 12 | 10 = 14");
    then_output_contains("Flag combination: 0 | 1 | 4 | 8 = 13");
    then_exit_code_is(0);
}

// Test scenario: Bitwise XOR operation
void test_bitwise_xor(void) {
    bdd_scenario("Bitwise XOR operation");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bitwise_xor.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Bitwise XOR: 12 ^ 10 = 6");
    then_output_contains("XOR swap successful: x=7, y=5");
    then_exit_code_is(0);
}

// Test scenario: Bitwise NOT operation
void test_bitwise_not(void) {
    bdd_scenario("Bitwise NOT operation");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bitwise_not.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Bitwise NOT: ~0 = -1");
    then_output_contains("Bitwise NOT: ~15 = -16");
    then_exit_code_is(0);
}

// Test scenario: Left shift operation
void test_left_shift(void) {
    bdd_scenario("Left shift operation");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("left_shift.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Left shift: 5 << 2 = 20");
    then_output_contains("Power of 2: 1 << 3 = 8");
    then_exit_code_is(0);
}

// Test scenario: Right shift operation
void test_right_shift(void) {
    bdd_scenario("Right shift operation");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("right_shift.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Right shift: 20 >> 2 = 5");
    then_output_contains("Division by power of 2: 64 >> 3 = 8");
    then_exit_code_is(0);
}

// Test scenario: Combined bitwise operations
void test_combined_bitwise(void) {
    bdd_scenario("Combined bitwise operations");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("combined_bitwise.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Lower nibble extracted: 6");
    then_output_contains("Bit 3 set successfully: 8");
    then_output_contains("Bit 3 toggled successfully: 0");
    then_exit_code_is(0);
}

// Test scenario: Bitwise operations with different integer types
void test_bitwise_types(void) {
    bdd_scenario("Bitwise operations with different integer types");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // i64 operations\n"
        "    let a_64: i64 = 0xFF00FF00FF00FF00;\n"
        "    let b_64: i64 = 0x00FF00FF00FF00FF;\n"
        "    let result_64: i64 = a_64 & b_64;\n"
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
    
    given_file_with_content("bitwise_types.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("i64 bitwise AND: alternating pattern = 0");
    then_output_contains("Mixed arithmetic and bitwise: 67");
    then_exit_code_is(0);
}

// Test scenario: Bitwise operator precedence
void test_bitwise_precedence(void) {
    bdd_scenario("Bitwise operator precedence");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bitwise_precedence.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Shift precedence: 2 + 3 << 1 = 10");
    then_output_contains("Bitwise AND with comparison: (7 & 3) == 3 is true");
    then_exit_code_is(0);
}

// Test scenario: Binary literals with bitwise operations
void test_binary_literals(void) {
    bdd_scenario("Binary literals with bitwise operations");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("binary_literals.asthra", source);
    when_compile_file();
    then_compilation_should_succeed();
    then_executable_created();
    when_run_executable();
    then_output_contains("Binary literal masking: 0b10101010 & 0b1111 = 0b1010");
    then_exit_code_is(0);
}

// Test scenario: Error - bitwise operations on non-integer types
void test_bitwise_type_error(void) {
    bdd_scenario("Error - bitwise operations on non-integer types");
    
    given_asthra_compiler_available();
    
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
    
    given_file_with_content("bitwise_type_error.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("bitwise");
}

// Test scenario: Error - shift by negative amount
void test_negative_shift(void) {
    bdd_scenario("Error - shift by negative amount");
    
    given_asthra_compiler_available();
    
    const char* source = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let a: i32 = 10;\n"
        "    let shift_amount: i32 = -2;\n"
        "    let result: i32 = a << shift_amount;  // Error or undefined behavior\n"
        "    \n"
        "    return ();\n"
        "}\n";
    
    given_file_with_content("negative_shift.asthra", source);
    when_compile_file();
    then_compilation_should_fail();
    then_error_contains("shift");
}

// Main test runner
int main(void) {
    bdd_init("Bitwise Operators");
    
    // Check if @wip scenarios should be skipped
    if (bdd_should_skip_wip()) {
        // Skip @wip scenarios
        bdd_skip_scenario("Bitwise XOR operation [@wip]");
        bdd_skip_scenario("Combined bitwise operations [@wip]");
        bdd_skip_scenario("Bitwise operations with different integer types [@wip]");
        bdd_skip_scenario("Binary literals with bitwise operations [@wip]");
        bdd_skip_scenario("Error - shift by negative amount [@wip]");
        
        // Run only non-@wip scenarios
        test_bitwise_and();
        test_bitwise_or();
        test_bitwise_not();
        test_left_shift();
        test_right_shift();
        test_bitwise_precedence();
        test_bitwise_type_error();
    } else {
        // Run all bitwise operator test scenarios
        test_bitwise_and();
        test_bitwise_or();
        test_bitwise_xor();
        test_bitwise_not();
        test_left_shift();
        test_right_shift();
        test_combined_bitwise();
        test_bitwise_types();
        test_bitwise_precedence();
        test_binary_literals();
        test_bitwise_type_error();
        test_negative_shift();
    }
    
    // Cleanup
    common_cleanup();
    
    return bdd_report();
}