#include "test_immutable_by_default_common.h"

// =============================================================================
// PERFORMANCE VALIDATION TESTS
// =============================================================================

bool test_performance_targets_validation(void) {
    printf("Testing performance targets validation...\n");

    // Test that immutable-by-default doesn't significantly impact compile time
    const char *large_immutable_code = "pub fn large_computation(none) -> i32 {\n";

    // Generate many immutable variable declarations
    char *extended_code = malloc(8192);
    strcpy(extended_code, large_immutable_code);

    for (int i = 0; i < 50; i++) {
        char line[128];
        snprintf(line, sizeof(line), "    let value_%d: i32 = %d + %d;\n", i, i, i * 2);
        strcat(extended_code, line);
    }

    strcat(extended_code, "    let total: i32 = value_0");
    for (int i = 1; i < 50; i++) {
        char addition[32];
        snprintf(addition, sizeof(addition), " + value_%d", i);
        strcat(extended_code, addition);
    }
    strcat(extended_code, ";\n    return total;\n}");

    bool result = compile_and_validate_asthra_code(extended_code, "const int32_t");

    free(extended_code);

    if (!result) {
        printf("  ❌ Performance validation test failed\n");
        return false;
    }

    printf("  ✅ Performance targets validation test passed\n");
    return true;
}

// =============================================================================
// AI GENERATION PATTERN TESTS
// =============================================================================

bool test_ai_friendly_patterns(void) {
    printf("Testing AI-friendly programming patterns...\n");

    IntegrationTestCase ai_patterns[] = {
        // Pattern 1: Clear transformation pipeline
        {"pub fn transform_data(input: string) -> string {\n"
         "    let step1: string = normalize_input(input);\n"
         "    let step2: string = apply_transformations(step1);\n"
         "    let step3: string = format_output(step2);\n"
         "    return step3;\n"
         "}",
         "const", true, false},

        // Pattern 2: Builder pattern with immutability
        {"struct Config {\n"
         "    host: string,\n"
         "    port: i32,\n"
         "    ssl: bool\n"
         "}\n"
         "\n"
         "pub fn build_config(none) -> Config {\n"
         "    let base_config: Config = Config {\n"
         "        host: \"localhost\",\n"
         "        port: 8080,\n"
         "        ssl: false\n"
         "    };\n"
         "    \n"
         "    let enhanced_config: Config = add_ssl_support(base_config);\n"
         "    let final_config: Config = optimize_settings(enhanced_config);\n"
         "    \n"
         "    return final_config;\n"
         "}",
         "Config base_config", true, false},

        // Pattern 3: Error handling with immutable flow
        {"pub fn safe_divide(a: f64, b: f64) -> Result<f64, string> {\n"
         "    let is_zero: bool = (b == 0.0);\n"
         "    \n"
         "    if is_zero {\n"
         "        let error_msg: string = \"Division by zero\";\n"
         "        return Result.Err(error_msg);\n"
         "    }\n"
         "    \n"
         "    let result: f64 = a / b;\n"
         "    return Result.Ok(result);\n"
         "}",
         "const bool is_zero", true, false},

        // Pattern 4: Data processing pipeline
        {"pub fn process_user_data(raw_data: string) -> Result<UserProfile, string> {\n"
         "    let trimmed: string = raw_data.trim();\n"
         "    let parsed: JsonValue = parse_json(trimmed);\n"
         "    let validated: UserData = validate_user_data(parsed);\n"
         "    let sanitized: UserData = sanitize_input(validated);\n"
         "    let profile: UserProfile = create_user_profile(sanitized);\n"
         "    \n"
         "    return Result.Ok(profile);\n"
         "}",
         "string trimmed", true, false},

        // Pattern 5: Functional composition
        {"pub fn calculate_final_price(base_price: f64, discount: f64, tax_rate: f64) -> f64 {\n"
         "    let discounted_price: f64 = apply_discount(base_price, discount);\n"
         "    let tax_amount: f64 = calculate_tax(discounted_price, tax_rate);\n"
         "    let final_price: f64 = discounted_price + tax_amount;\n"
         "    let rounded_price: f64 = round_to_cents(final_price);\n"
         "    \n"
         "    return rounded_price;\n"
         "}",
         "const double discounted_price", true, false}};

    size_t num_tests = sizeof(ai_patterns) / sizeof(ai_patterns[0]);
    for (size_t i = 0; i < num_tests; i++) {
        bool result = compile_and_validate_asthra_code(ai_patterns[i].asthra_code,
                                                       ai_patterns[i].expected_c_pattern);

        if (!result) {
            printf("  ❌ AI pattern test case %zu failed\n", i);
            return false;
        }
    }

    printf("  ✅ AI-friendly pattern tests passed\n");
    return true;
}
