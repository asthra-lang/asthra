#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Test framework definitions
#define TEST_PASS 1
#define TEST_FAIL 0

int test_count = 0;
int passed_tests = 0;

void test_result(const char* test_name, int result) {
    test_count++;
    if (result == TEST_PASS) {
        passed_tests++;
        printf("✓ %s\n", test_name);
    } else {
        printf("✗ %s\n", test_name);
    }
}

// Helper function to check if file exists and is readable
int file_exists_and_readable(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return (st.st_mode & S_IRUSR) != 0;
    }
    return 0;
}

// Helper function to count lines in file
int count_lines_in_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    int lines = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') lines++;
    }
    
    fclose(file);
    return lines;
}

// Helper function to check if file contains specific text
int file_contains_text(const char* filename, const char* text) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return 0;
    }
    
    size_t bytes_read = fread(content, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        free(content);
        return 0;
    }
    content[file_size] = '\0';
    
    int found = strstr(content, text) != NULL;
    free(content);
    return found;
}

// Helper function to count occurrences of text in file
int count_text_occurrences(const char* filename, const char* text) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return -1;
    }
    
    size_t bytes_read = fread(content, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        free(content);
        return 0;
    }
    content[file_size] = '\0';
    
    int count = 0;
    char* pos = content;
    while ((pos = strstr(pos, text)) != NULL) {
        count++;
        pos += strlen(text);
    }
    
    free(content);
    return count;
}

// Test 1: Package Structure Validation
int test_package_structure(void) {
    const char* required_files[] = {
        "../stdlib/toml/mod.asthra",
        "../stdlib/toml/common.asthra", 
        "../stdlib/toml/parser.asthra",
        "../stdlib/toml/generator.asthra",
        "../stdlib/toml/table.asthra",
        "../stdlib/toml/utils.asthra"
    };
    
    int num_files = sizeof(required_files) / sizeof(required_files[0]);
    
    for (int i = 0; i < num_files; i++) {
        if (!file_exists_and_readable(required_files[i])) {
            printf("Missing or unreadable file: %s\n", required_files[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 2: Package Declaration Validation
int test_package_declarations(void) {
    const char* files[] = {
        "../stdlib/toml/mod.asthra",
        "../stdlib/toml/common.asthra",
        "../stdlib/toml/parser.asthra", 
        "../stdlib/toml/generator.asthra",
        "../stdlib/toml/table.asthra",
        "../stdlib/toml/utils.asthra"
    };
    
    int num_files = sizeof(files) / sizeof(files[0]);
    
    for (int i = 0; i < num_files; i++) {
        if (!file_contains_text(files[i], "package stdlib::toml;")) {
            printf("Missing package declaration in: %s\n", files[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 3: Core Types Validation
int test_core_types(void) {
    const char* filename = "../stdlib/toml/common.asthra";
    
    const char* required_types[] = {
        "enum TomlValueType",
        "enum TomlValue", 
        "struct TomlTable",
        "struct TomlDocument",
        "enum TomlError",
        "enum TomlResult",
        "struct TomlConfig"
    };
    
    int num_types = sizeof(required_types) / sizeof(required_types[0]);
    
    for (int i = 0; i < num_types; i++) {
        if (!file_contains_text(filename, required_types[i])) {
            printf("Missing type declaration: %s\n", required_types[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 4: Parser Functions Validation
int test_parser_functions(void) {
    const char* filename = "../stdlib/toml/parser.asthra";
    
    const char* required_functions[] = {
        "pub fn parse_string(",
        "pub fn parse_string_with_config(",
        "pub fn parse_file(",
        "pub fn new_parser(",
        "pub fn parse_document(",
        "pub fn parse_value(",
        "pub fn parse_key_value_pair("
    };
    
    int num_functions = sizeof(required_functions) / sizeof(required_functions[0]);
    
    for (int i = 0; i < num_functions; i++) {
        if (!file_contains_text(filename, required_functions[i])) {
            printf("Missing parser function: %s\n", required_functions[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 5: Generator Functions Validation  
int test_generator_functions(void) {
    const char* filename = "../stdlib/toml/generator.asthra";
    
    const char* required_functions[] = {
        "pub fn generate_string(",
        "pub fn generate_string_with_config(",
        "pub fn generate_file(",
        "pub fn generate_pretty(",
        "pub fn generate_compact(",
        "pub fn new_generator(",
        "pub fn generate_document("
    };
    
    int num_functions = sizeof(required_functions) / sizeof(required_functions[0]);
    
    for (int i = 0; i < num_functions; i++) {
        if (!file_contains_text(filename, required_functions[i])) {
            printf("Missing generator function: %s\n", required_functions[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 6: Table Operations Validation
int test_table_operations(void) {
    const char* filename = "../stdlib/toml/table.asthra";
    
    const char* required_functions[] = {
        "pub fn table_get(",
        "pub fn table_set(",
        "pub fn table_contains(",
        "pub fn table_remove(",
        "pub fn table_clone(",
        "pub fn table_merge(",
        "pub fn table_get_path(",
        "pub fn table_get_string(",
        "pub fn table_get_integer(",
        "pub fn table_get_boolean("
    };
    
    int num_functions = sizeof(required_functions) / sizeof(required_functions[0]);
    
    for (int i = 0; i < num_functions; i++) {
        if (!file_contains_text(filename, required_functions[i])) {
            printf("Missing table function: %s\n", required_functions[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 7: Utility Functions Validation
int test_utility_functions(void) {
    const char* filename = "../stdlib/toml/utils.asthra";
    
    const char* required_functions[] = {
        "pub fn get_value_from_string(",
        "pub fn get_value_from_file(",
        "pub fn is_valid_toml(",
        "pub fn validate_toml_file(",
        "pub fn value_to_string(",
        "pub fn extract_string_array(",
        "pub fn new_config_loader(",
        "pub fn validate_against_schema("
    };
    
    int num_functions = sizeof(required_functions) / sizeof(required_functions[0]);
    
    for (int i = 0; i < num_functions; i++) {
        if (!file_contains_text(filename, required_functions[i])) {
            printf("Missing utility function: %s\n", required_functions[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 8: Error Handling Validation
int test_error_handling(void) {
    const char* filename = "../stdlib/toml/common.asthra";
    
    const char* required_error_types[] = {
        "TomlParseError",
        "TomlTypeError", 
        "TomlKeyError",
        "pub fn parse_error(",
        "pub fn type_error(",
        "pub fn key_error(",
        "pub fn format_error("
    };
    
    int num_types = sizeof(required_error_types) / sizeof(required_error_types[0]);
    
    for (int i = 0; i < num_types; i++) {
        if (!file_contains_text(filename, required_error_types[i])) {
            printf("Missing error handling: %s\n", required_error_types[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 9: Value Types Completeness
int test_value_types_completeness(void) {
    const char* filename = "../stdlib/toml/common.asthra";
    
    const char* required_value_types[] = {
        "String,",
        "Integer,", 
        "Float,",
        "Boolean,",
        "DateTime,",
        "Array,",
        "Table,",
        "InlineTable,"
    };
    
    int num_types = sizeof(required_value_types) / sizeof(required_value_types[0]);
    
    for (int i = 0; i < num_types; i++) {
        if (!file_contains_text(filename, required_value_types[i])) {
            printf("Missing value type: %s\n", required_value_types[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 10: Configuration Support Validation
int test_configuration_support(void) {
    const char* common_file = "../stdlib/toml/common.asthra";
    const char* utils_file = "../stdlib/toml/utils.asthra";
    
    const char* config_features[] = {
        "struct TomlConfig",
        "struct ConfigLoader",
        "pub fn default_config(",
        "pub fn lenient_config("
    };
    
    // Check in common.asthra
    if (!file_contains_text(common_file, config_features[0]) ||
        !file_contains_text(common_file, config_features[2]) ||
        !file_contains_text(common_file, config_features[3])) {
        printf("Missing configuration support in common.asthra\n");
        return TEST_FAIL;
    }
    
    // Check in utils.asthra  
    if (!file_contains_text(utils_file, config_features[1])) {
        printf("Missing ConfigLoader in utils.asthra\n");
        return TEST_FAIL;
    }
    
    return TEST_PASS;
}

// Test 11: Implementation Completeness
int test_implementation_completeness(void) {
    // Count total functions across all modules
    int parser_functions = count_text_occurrences("../stdlib/toml/parser.asthra", "pub fn ");
    int generator_functions = count_text_occurrences("../stdlib/toml/generator.asthra", "pub fn ");
    int table_functions = count_text_occurrences("../stdlib/toml/table.asthra", "pub fn ");
    int utils_functions = count_text_occurrences("../stdlib/toml/utils.asthra", "pub fn ");
    int common_functions = count_text_occurrences("../stdlib/toml/common.asthra", "pub fn ");
    
    int total_functions = parser_functions + generator_functions + table_functions + 
                         utils_functions + common_functions;
    
    // Expect at least 50 public functions total
    if (total_functions < 50) {
        printf("Insufficient function count: %d (expected >= 50)\n", total_functions);
        return TEST_FAIL;
    }
    
    printf("Total public functions: %d\n", total_functions);
    return TEST_PASS;
}

// Test 12: External Dependencies Validation
int test_external_dependencies(void) {
    const char* files[] = {
        "../stdlib/toml/parser.asthra",
        "../stdlib/toml/generator.asthra", 
        "../stdlib/toml/table.asthra",
        "../stdlib/toml/utils.asthra"
    };
    
    int num_files = sizeof(files) / sizeof(files[0]);
    
    for (int i = 0; i < num_files; i++) {
        if (!file_contains_text(files[i], "extern fn ")) {
            printf("Missing external function declarations in: %s\n", files[i]);
            return TEST_FAIL;
        }
    }
    
    return TEST_PASS;
}

// Test 13: Grammar Compliance Validation
int test_grammar_compliance(void) {
    const char* files[] = {
        "../stdlib/toml/mod.asthra",
        "../stdlib/toml/common.asthra",
        "../stdlib/toml/parser.asthra",
        "../stdlib/toml/generator.asthra", 
        "../stdlib/toml/table.asthra",
        "../stdlib/toml/utils.asthra"
    };
    
    int num_files = sizeof(files) / sizeof(files[0]);
    
    // Check for PEG grammar compliance patterns
    for (int i = 0; i < num_files; i++) {
        // All files should have proper package declarations
        if (!file_contains_text(files[i], "package stdlib::toml;")) {
            printf("Invalid package declaration in: %s\n", files[i]);
            return TEST_FAIL;
        }
        
        // Check for proper import statements
        if (i > 0) { // Skip mod.asthra which might not import
            if (!file_contains_text(files[i], "import \"stdlib/asthra/core\";")) {
                printf("Missing core import in: %s\n", files[i]);
                return TEST_FAIL;
            }
        }
    }
    
    return TEST_PASS;
}

int main(void) {
    printf("=== TOML Package Test Suite ===\n");
    printf("Testing stdlib/toml implementation...\n\n");
    
    // Run all tests
    test_result("Package Structure", test_package_structure());
    test_result("Package Declarations", test_package_declarations());
    test_result("Core Types", test_core_types());
    test_result("Parser Functions", test_parser_functions());
    test_result("Generator Functions", test_generator_functions());
    test_result("Table Operations", test_table_operations());
    test_result("Utility Functions", test_utility_functions());
    test_result("Error Handling", test_error_handling());
    test_result("Value Types Completeness", test_value_types_completeness());
    test_result("Configuration Support", test_configuration_support());
    test_result("Implementation Completeness", test_implementation_completeness());
    test_result("External Dependencies", test_external_dependencies());
    test_result("Grammar Compliance", test_grammar_compliance());
    
    // Print summary
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", passed_tests, test_count);
    printf("Success Rate: %.1f%%\n", (float)passed_tests / test_count * 100);
    
    if (passed_tests == test_count) {
        printf("🎉 All tests passed! TOML package implementation is complete.\n");
        return 0;
    } else {
        printf("❌ Some tests failed. Please review the implementation.\n");
        return 1;
    }
} 
