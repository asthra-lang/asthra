/**
 * Real Program Validation Tests - Feature Validation Functions
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_validation_common.h"

// =============================================================================
// FEATURE VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate that multi-line strings feature is working
 */
bool validate_multiline_strings_feature(const TestSuiteConfig *config) {
    const char *test_programs[] = {
        "package main; pub fn main(none) -> void { let x: string = r\"\"\"raw\nmultiline\"\"\";}",
        "package main; pub fn main(none) -> void { let x: string = "
        "\"\"\"processed\nmultiline\"\"\";}",
        "package main; pub fn main(none) -> void { let x: string = r\"\"\"{}\"\"\";}",
    };

    return validate_feature_functionality("multiline_strings", test_programs, 3, config);
}

/**
 * Validate that basic type system is working
 */
bool validate_type_system_feature(const TestSuiteConfig *config) {
    const char *test_programs[] = {
        "package main; pub fn main(none) -> void { let x: int = 42;}",
        "package main; pub fn main(none) -> void { let x: string = \"hello\";}",
        "package main; pub fn main(none) -> void { let x: bool = true;}",
        "package main; pub fn main(none) -> void { let x: []int = [1, 2, 3];}",
    };

    return validate_feature_functionality("type_system", test_programs, 4, config);
}

/**
 * Validate that function declarations are working
 */
bool validate_function_declarations_feature(const TestSuiteConfig *config) {
    const char *test_programs[] = {
        "package main; priv fn test(none) -> void {} pub fn main(none) -> void {}",
        "package main; priv fn add(a: int, b: int) -> int { return a + b; } pub fn main(none) -> "
        "void {}",
        "package main; priv fn greet(name: string) -> string { return \"Hello \" + name; } pub fn "
        "main(none) -> void {}",
    };

    return validate_feature_functionality("function_declarations", test_programs, 3, config);
}

/**
 * Validate that if-let statements are working
 */
bool validate_if_let_feature(const TestSuiteConfig *config) {
    const char *test_programs[] = {
        "package main; priv fn test(opt: Option<int>) -> void { if let Option.Some(x) = opt {} } "
        "pub fn main(none) -> void {}",
        "package main; priv fn test(result: Result<int, string>) -> void { if let Result.Ok(x) = "
        "result {} } pub fn main(none) -> void {}",
    };

    return validate_feature_functionality("if_let", test_programs, 2, config);
}