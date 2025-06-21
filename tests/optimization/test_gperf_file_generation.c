/**
 * Asthra Programming Language
 * Gperf File Generation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_gperf_common.h"

static AsthraTestResult test_gperf_file_structure(AsthraTestContext *context) {
    const char *gperf_file = "optimization/asthra_keywords.gperf";
    
    // Check if gperf input file exists
    if (!ASTHRA_TEST_ASSERT(context, gperf_file_exists(gperf_file), 
                           "Gperf input file should exist: %s", gperf_file)) {
        return ASTHRA_TEST_FAIL;
    }
    
    FILE *file = fopen(gperf_file, "r");
    if (!ASTHRA_TEST_ASSERT(context, file != NULL, 
                           "Should be able to open gperf file for reading")) {
        return ASTHRA_TEST_FAIL;
    }
    
    char line[256];
    bool found_struct_declaration = false;
    bool found_keywords_section = false;
    bool found_delimiter = false;
    int keyword_count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Check for proper gperf structure
        if (strstr(line, "struct") && strstr(line, "keyword")) {
            found_struct_declaration = true;
        }
        
        if (strstr(line, "%%")) {
            if (!found_delimiter) {
                found_delimiter = true;
                found_keywords_section = true;
            }
        }
        
        // Count keywords (lines that don't start with % or # and contain comma)
        if (found_keywords_section && !found_delimiter && 
            line[0] != '%' && line[0] != '#' && strstr(line, ",")) {
            keyword_count++;
        }
    }
    
    fclose(file);
    
    if (!ASTHRA_TEST_ASSERT(context, found_struct_declaration, 
                           "Gperf file should contain struct declaration")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, found_keywords_section, 
                           "Gperf file should contain keywords section")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, keyword_count >= 20, 
                           "Gperf file should contain at least 20 keywords, found %d", keyword_count)) {
        return ASTHRA_TEST_FAIL;
    }
    
    printf("Gperf file structure validation passed (%d keywords found)\n", keyword_count);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_gperf_generated_files(AsthraTestContext *context) {
    const char *generated_files[] = {
        "optimization/asthra_keywords_hash.c",
        "optimization/asthra_keywords_hash.h"
    };
    
    for (size_t i = 0; i < sizeof(generated_files) / sizeof(generated_files[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, gperf_file_exists(generated_files[i]), 
                               "Generated file should exist: %s", generated_files[i])) {
            return ASTHRA_TEST_FAIL;
        }
        
        // Check file size (should be substantial)
        if (!ASTHRA_TEST_ASSERT(context, check_file_substantial(generated_files[i], 500), 
                               "Generated file should be substantial: %s", generated_files[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_gperf_header_file_content(AsthraTestContext *context) {
    const char *header_file = "optimization/asthra_keywords_hash.h";
    FILE *file = fopen(header_file, "r");
    
    if (!ASTHRA_TEST_ASSERT(context, file != NULL, 
                           "Should be able to open header file")) {
        return ASTHRA_TEST_FAIL;
    }
    
    char line[512];
    bool found_include_guard = false;
    bool found_function_declaration = false;
    bool found_struct_definition = false;
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "#ifndef") || strstr(line, "#define")) {
            found_include_guard = true;
        }
        
        if (strstr(line, "asthra_keyword_lookup") || strstr(line, "in_word_set")) {
            found_function_declaration = true;
        }
        
        if (strstr(line, "struct") && strstr(line, "keyword")) {
            found_struct_definition = true;
        }
    }
    
    fclose(file);
    
    if (!ASTHRA_TEST_ASSERT(context, found_include_guard, 
                           "Header file should have include guards")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, found_function_declaration, 
                           "Header file should declare lookup function")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!ASTHRA_TEST_ASSERT(context, found_struct_definition, 
                           "Header file should define keyword struct")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Public test suite interface
AsthraTestFunction gperf_file_generation_tests[] = {
    test_gperf_file_structure,
    test_gperf_generated_files,
    test_gperf_header_file_content
};

AsthraTestMetadata gperf_file_generation_metadata[] = {
    {"Gperf File Structure", __FILE__, __LINE__, "test_gperf_file_structure", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Gperf Generated Files", __FILE__, __LINE__, "test_gperf_generated_files", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Gperf Header File Content", __FILE__, __LINE__, "test_gperf_header_file_content", ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}
};

const size_t gperf_file_generation_test_count = sizeof(gperf_file_generation_tests) / sizeof(gperf_file_generation_tests[0]); 
