/**
 * Real Program Test Data Implementation
 *
 * Contains predefined test programs and benchmarks for comprehensive
 * real program testing to prevent false positives.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_data.h"
#include <stdbool.h>

// =============================================================================
// PREDEFINED TEST PROGRAMS
// =============================================================================

const char *BASIC_TYPES_PROGRAMS[] = {
    "package main; pub fn main(none) { let x: int = 42; }",
    "package main; pub fn main(none) { let text: string = \"hello\"; }",
    "package main; pub fn main(none) { let flag: bool = true; }",
    "package main; pub fn main(none) { let numbers: []int = [1, 2, 3]; }",
    "package main; pub fn main(none) { let ch: char = 'a'; }"};
const size_t BASIC_TYPES_PROGRAM_COUNT =
    sizeof(BASIC_TYPES_PROGRAMS) / sizeof(BASIC_TYPES_PROGRAMS[0]);

const char *FUNCTION_TEST_PROGRAMS[] = {
    "package main; fn add(a: int, b: int) -> int { return a + b; } pub fn main(none) { let result: "
    "int = add(5, 3); }",
    "package main; fn greet(name: string) -> string { return \"Hello, \" + name; } pub fn "
    "main(none) { let msg: string = greet(\"World\"); }",
    "package main; fn is_positive(x: int) -> bool { return x > 0; } pub fn main(none) { let check: "
    "bool = is_positive(42); }",
    "package main; fn double_array(arr: []int) -> []int { return arr; } pub fn main(none) { let "
    "nums: []int = double_array([1, 2, 3]); }",
    "package main; fn factorial(n: int) -> int { if n <= 1 { return 1; } return n * factorial(n - "
    "1); } pub fn main(none) { let fact: int = factorial(5); },",
    "package main; fn max(a: int, b: int) -> int { if a > b { return a; } return b; } pub fn "
    "main(none) { let maximum: int = max(10, 20); }",
    "package main; fn count_chars(text: string) -> int { return 0; } pub fn main(none) { let "
    "count: int = count_chars(\"hello\"); }",
    "package main; fn process_bool(flag: bool) -> bool { return !flag; } pub fn main(none) { let "
    "inverted: bool = process_bool(true); }"};
const size_t FUNCTION_TEST_PROGRAM_COUNT =
    sizeof(FUNCTION_TEST_PROGRAMS) / sizeof(FUNCTION_TEST_PROGRAMS[0]);

const char *CONTROL_FLOW_PROGRAMS[] = {
    "package main; pub fn main(none) { let x: int = 42; if x > 0 { let positive: bool = true; } }",
    "package main; pub fn main(none) { let x: int = -5; if x > 0 { let positive: bool = true; } "
    "else { let negative: bool = true; } }",
    "package main; pub fn main(none) { let i: int = 0; while i < 10 { i = i + 1; } }",
    "package main; pub fn main(none) { let arr: []int = [1, 2, 3]; for item in arr { let doubled: "
    "int = item * 2; } }",
    "package main; pub fn main(none) { let x: int = 1; match x { 1 => { let one: bool = true; } _ "
    "=> { let other: bool = true; } } }",
    "package main; pub fn main(none) { let nested: int = 5; if nested > 0 { if nested < 10 { let "
    "small_positive: bool = true; } } }"};
const size_t CONTROL_FLOW_PROGRAM_COUNT =
    sizeof(CONTROL_FLOW_PROGRAMS) / sizeof(CONTROL_FLOW_PROGRAMS[0]);

const char *ERROR_DETECTION_PROGRAMS[] = {
    "package main; pub fn main(none) { let x: int = \"not a number\"; }",       // Type mismatch
    "package main; pub fn main(none) { let x: int = 42;; }",                    // Syntax error
    "package main; pub fn main(none) { let arr: []invalid_type = [1, 2, 3]; }", // Invalid type
    "package invalid syntax here",                               // Missing package declaration
    "package main; pub fn main(none) { let x = 42; }",           // Missing type annotation
    "package main; pub fn main(none) { undefined_function(); }", // Undefined function
};
const size_t ERROR_DETECTION_PROGRAM_COUNT =
    sizeof(ERROR_DETECTION_PROGRAMS) / sizeof(ERROR_DETECTION_PROGRAMS[0]);

const PerformanceBenchmark REQUIRED_BENCHMARKS[] = {
    {"minimal_program", "package main; pub fn main(none) { let x: int = 42; }", 10, 1024, true,
     "Basic minimal program"},
    {"medium_program",
     "package main; fn helper(x: int) -> int { return x * 2; } pub fn main(none) { let a: int = 1; "
     "let b: int = helper(a); let c: string = \"hello\"; }",
     50, 4096, true, "Medium complexity program"},
    {"type_system",
     "package main; pub fn main(none) { let i: int = 42; let s: string = \"hello\"; let b: bool = "
     "true; let arr: []int = [1, 2, 3]; }",
     100, 8192, true, "Type system usage"},
    {"control_flow",
     "package main; pub fn main(none) { let i: int = 0; while i < 5 { if i % 2 == 0 { let even: "
     "bool = true; } i = i + 1; } }",
     150, 12288, false, "Control flow complexity"}};
const size_t REQUIRED_BENCHMARK_COUNT =
    sizeof(REQUIRED_BENCHMARKS) / sizeof(REQUIRED_BENCHMARKS[0]);