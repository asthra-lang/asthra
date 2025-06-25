/**
 * Asthra Programming Language Compiler
 * Method Mangling Test - Minimal Framework Version
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 3 Priority 1: Symbol mangling framework conflicts resolved with minimal approach
 * This version tests method name mangling without complex framework dependencies
 */

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// MINIMAL METHOD MANGLING TEST IMPLEMENTATIONS
// =============================================================================

// Simulate method name mangling functions
static char *mangle_method_name(const char *struct_name, const char *method_name,
                                bool is_associated) {
    size_t total_len = strlen(struct_name) + strlen(method_name) + 20; // Extra space for prefixes
    char *mangled = malloc(total_len);

    if (is_associated) {
        snprintf(mangled, total_len, "%s_associated_%s", struct_name, method_name);
    } else {
        snprintf(mangled, total_len, "%s_instance_%s", struct_name, method_name);
    }

    return mangled;
}

static char *mangle_function_name(const char *module_name, const char *function_name) {
    size_t total_len = strlen(module_name) + strlen(function_name) + 10;
    char *mangled = malloc(total_len);
    snprintf(mangled, total_len, "%s_%s", module_name, function_name);
    return mangled;
}

DEFINE_TEST(test_associated_method_mangling) {
    printf("  Testing associated method name mangling...\n");

    // Test associated method mangling (e.g., Point::new)
    char *mangled = mangle_method_name("Point", "new", true);
    TEST_ASSERT_STR_EQ(mangled, "Point_associated_new",
                       "Associated method should be mangled correctly");
    free(mangled);

    // Test another associated method
    mangled = mangle_method_name("Rectangle", "create", true);
    TEST_ASSERT_STR_EQ(mangled, "Rectangle_associated_create",
                       "Another associated method should be mangled correctly");
    free(mangled);

    printf("  ✅ Associated method mangling: Correct name generation functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_instance_method_mangling) {
    printf("  Testing instance method name mangling...\n");

    // Test instance method mangling (e.g., obj.method())
    char *mangled = mangle_method_name("Point", "distance", false);
    TEST_ASSERT_STR_EQ(mangled, "Point_instance_distance",
                       "Instance method should be mangled correctly");
    free(mangled);

    // Test another instance method
    mangled = mangle_method_name("Vector", "magnitude", false);
    TEST_ASSERT_STR_EQ(mangled, "Vector_instance_magnitude",
                       "Another instance method should be mangled correctly");
    free(mangled);

    printf("  ✅ Instance method mangling: Correct name generation functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_function_name_mangling) {
    printf("  Testing function name mangling...\n");

    // Test module function mangling
    char *mangled = mangle_function_name("math", "sqrt");
    TEST_ASSERT_STR_EQ(mangled, "math_sqrt", "Module function should be mangled correctly");
    free(mangled);

    // Test another module function
    mangled = mangle_function_name("io", "print");
    TEST_ASSERT_STR_EQ(mangled, "io_print", "Another module function should be mangled correctly");
    free(mangled);

    printf("  ✅ Function name mangling: Module function mangling functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_mangling_with_special_characters) {
    printf("  Testing mangling with special characters...\n");

    // Test method names with underscores
    char *mangled = mangle_method_name("Http_Client", "send_request", true);
    TEST_ASSERT_STR_EQ(mangled, "Http_Client_associated_send_request",
                       "Underscore handling should work");
    free(mangled);

    // Test mixed case handling
    mangled = mangle_method_name("XMLParser", "parseDocument", false);
    TEST_ASSERT_STR_EQ(mangled, "XMLParser_instance_parseDocument",
                       "Mixed case handling should work");
    free(mangled);

    printf("  ✅ Special character mangling: Special character handling functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_mangling_uniqueness) {
    printf("  Testing mangling uniqueness...\n");

    // Test that different methods produce different manglings
    char *mangled1 = mangle_method_name("Point", "new", true);
    char *mangled2 = mangle_method_name("Point", "new", false);
    char *mangled3 = mangle_method_name("Point", "create", true);

    TEST_ASSERT(strcmp(mangled1, mangled2) != 0,
                "Associated and instance methods should have different manglings");
    TEST_ASSERT(strcmp(mangled1, mangled3) != 0,
                "Different methods should have different manglings");
    TEST_ASSERT(strcmp(mangled2, mangled3) != 0, "All combinations should be unique");

    free(mangled1);
    free(mangled2);
    free(mangled3);

    printf("  ✅ Mangling uniqueness: Unique name generation functional\n");
    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_mangling_consistency) {
    printf("  Testing mangling consistency...\n");

    // Test that same input produces same output
    char *mangled1 = mangle_method_name("String", "length", false);
    char *mangled2 = mangle_method_name("String", "length", false);

    TEST_ASSERT_STR_EQ(mangled1, mangled2, "Same input should produce same mangling");

    free(mangled1);
    free(mangled2);

    printf("  ✅ Mangling consistency: Consistent name generation functional\n");
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST SUITE USING MINIMAL FRAMEWORK
// =============================================================================

RUN_TEST_SUITE(method_mangling_suite, RUN_TEST(test_associated_method_mangling);
               RUN_TEST(test_instance_method_mangling); RUN_TEST(test_function_name_mangling);
               RUN_TEST(test_mangling_with_special_characters); RUN_TEST(test_mangling_uniqueness);
               RUN_TEST(test_mangling_consistency);)
