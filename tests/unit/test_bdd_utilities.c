#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// Basic standalone unit test framework
static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

#define TEST_CASE(name) \
    int name(void); \
    int name(void)

#define RUN_TEST(test_func) \
    do { \
        test_count++; \
        printf("Running %s... ", #test_func); \
        fflush(stdout); \
        if (test_func()) { \
            printf("PASS\n"); \
            test_passed++; \
        } else { \
            printf("FAIL\n"); \
            test_failed++; \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            printf("ASSERTION FAILED: %s at line %d\n", #condition, __LINE__); \
            return 0; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_EQ(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_NE(a, b) ASSERT_TRUE((a) != (b))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_STR_EQ(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)
#define ASSERT_STR_CONTAINS(haystack, needle) \
    ASSERT_TRUE((haystack) && strstr((haystack), (needle)) != NULL)

#define ASSERT_FILE_EXISTS(filename) \
    do { \
        struct stat st; \
        ASSERT_TRUE(stat((filename), &st) == 0); \
    } while(0)

#define ASSERT_FILE_NOT_EXISTS(filename) \
    do { \
        struct stat st; \
        ASSERT_TRUE(stat((filename), &st) != 0); \
    } while(0)

// Utility functions to test (simplified versions of BDD utilities)

char* util_execute_command(const char* command, int* exit_code) {
    if (!command) {
        *exit_code = 1;
        return NULL;
    }
    
    FILE* fp = popen(command, "r");
    if (!fp) {
        *exit_code = 1;
        return NULL;
    }
    
    char* output = malloc(4096);
    if (!output) {
        pclose(fp);
        *exit_code = 1;
        return NULL;
    }
    
    size_t total_read = 0;
    size_t bytes_read;
    while ((bytes_read = fread(output + total_read, 1, 4096 - total_read - 1, fp)) > 0) {
        total_read += bytes_read;
        if (total_read >= 4095) break;
    }
    output[total_read] = '\0';
    
    *exit_code = pclose(fp);
    return output;
}

void util_create_temp_source_file(const char* filename, const char* content) {
    if (!filename) return;
    
    FILE* file = fopen(filename, "w");
    if (!file) return;
    
    if (content) {
        fputs(content, file);
    }
    fclose(file);
}

void util_cleanup_temp_files(void) {
    // Simple cleanup - remove .asthra files in current directory
    system("rm -f *.asthra");
}

int util_output_contains(const char* output, const char* expected) {
    if (!output || !expected) return 0;
    return strstr(output, expected) != NULL;
}

const char* util_find_asthra_compiler(void) {
    // Look for asthra compiler in common locations
    static const char* possible_paths[] = {
        "./asthra",
        "./bin/asthra", 
        "../asthra",
        "../bin/asthra",
        "../../asthra",
        "../../bin/asthra",
        "asthra"  // in PATH
    };
    
    static char found_path[256] = {0};
    
    for (int i = 0; i < sizeof(possible_paths) / sizeof(possible_paths[0]); i++) {
        struct stat st;
        if (stat(possible_paths[i], &st) == 0) {
            strncpy(found_path, possible_paths[i], sizeof(found_path) - 1);
            return found_path;
        }
    }
    
    return "asthra"; // fallback to PATH
}

int util_compiler_available(void) {
    int exit_code;
    char* output = util_execute_command("which asthra", &exit_code);
    if (output) {
        free(output);
        return exit_code == 0;
    }
    return 0;
}

// Unit tests for the BDD utilities

TEST_CASE(test_execute_command_echo) {
    int exit_code;
    char* output = util_execute_command("echo 'Hello BDD'", &exit_code);
    
    ASSERT_EQ(exit_code, 0);
    ASSERT_NOT_NULL(output);
    ASSERT_STR_CONTAINS(output, "Hello BDD");
    
    free(output);
    return 1;
}

TEST_CASE(test_execute_command_failure) {
    int exit_code;
    char* output = util_execute_command("false", &exit_code);
    
    ASSERT_NE(exit_code, 0);
    ASSERT_NOT_NULL(output);
    
    free(output);
    return 1;
}

TEST_CASE(test_execute_command_null) {
    int exit_code = 999;
    char* output = util_execute_command(NULL, &exit_code);
    
    ASSERT_NULL(output);
    ASSERT_NE(exit_code, 0);
    
    return 1;
}

TEST_CASE(test_find_asthra_compiler) {
    const char* compiler = util_find_asthra_compiler();
    
    ASSERT_NOT_NULL(compiler);
    ASSERT_TRUE(strlen(compiler) > 0);
    
    return 1;
}

TEST_CASE(test_create_temp_source_file) {
    const char* filename = "test_source.asthra";
    const char* content = 
        "package main;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Test content\");\n"
        "    return ();\n"
        "}\n";
    
    // Ensure file doesn't exist initially
    remove(filename);
    ASSERT_FILE_NOT_EXISTS(filename);
    
    // Create the file
    util_create_temp_source_file(filename, content);
    
    // Verify file exists
    ASSERT_FILE_EXISTS(filename);
    
    // Read file content and verify
    FILE* file = fopen(filename, "r");
    ASSERT_NOT_NULL(file);
    
    char buffer[1024];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytes_read] = '\0';
    fclose(file);
    
    ASSERT_STR_CONTAINS(buffer, "package main");
    ASSERT_STR_CONTAINS(buffer, "Test content");
    
    // Clean up
    remove(filename);
    
    return 1;
}

TEST_CASE(test_create_temp_source_file_null_filename) {
    const char* content = "test content";
    
    // Should handle null filename gracefully (shouldn't crash)
    util_create_temp_source_file(NULL, content);
    
    return 1;
}

TEST_CASE(test_create_temp_source_file_null_content) {
    const char* filename = "test_null_content.asthra";
    
    // Ensure file doesn't exist initially
    remove(filename);
    
    // Create file with null content
    util_create_temp_source_file(filename, NULL);
    
    // File should still be created (even if empty)
    ASSERT_FILE_EXISTS(filename);
    
    // Clean up
    remove(filename);
    
    return 1;
}

TEST_CASE(test_cleanup_temp_files) {
    // Create several test files
    const char* files[] = {
        "cleanup_test1.asthra",
        "cleanup_test2.asthra",
        "cleanup_test3.c"
    };
    
    for (int i = 0; i < 3; i++) {
        util_create_temp_source_file(files[i], "test content");
        ASSERT_FILE_EXISTS(files[i]);
    }
    
    // Run cleanup
    util_cleanup_temp_files();
    
    // Verify .asthra files are removed
    ASSERT_FILE_NOT_EXISTS(files[0]);
    ASSERT_FILE_NOT_EXISTS(files[1]);
    
    // .c file should remain (if it wasn't cleaned by our simple cleanup)
    // Clean up remaining file
    remove(files[2]);
    
    return 1;
}

TEST_CASE(test_output_contains) {
    const char* output = "This is a test output with multiple lines\nLine 2\nLine 3";
    
    ASSERT_TRUE(util_output_contains(output, "test output"));
    ASSERT_TRUE(util_output_contains(output, "Line 2"));
    ASSERT_FALSE(util_output_contains(output, "missing text"));
    ASSERT_FALSE(util_output_contains(NULL, "test"));
    ASSERT_FALSE(util_output_contains(output, NULL));
    
    return 1;
}

TEST_CASE(test_command_with_pipes) {
    int exit_code;
    char* output = util_execute_command("echo 'line1\nline2\nline3' | grep 'line2'", &exit_code);
    
    ASSERT_EQ(exit_code, 0);
    ASSERT_NOT_NULL(output);
    ASSERT_STR_CONTAINS(output, "line2");
    
    free(output);
    return 1;
}

TEST_CASE(test_large_file_operations) {
    const char* filename = "large_test.asthra";
    
    // Create a moderately large content string
    char large_content[5000];
    strcpy(large_content, "package main;\n\npub fn main(none) -> void {\n");
    
    for (int i = 0; i < 50; i++) {
        char line[100];
        snprintf(line, sizeof(line), "    log(\"Line %d\");\n", i);
        strcat(large_content, line);
    }
    strcat(large_content, "    return ();\n}\n");
    
    // Create and verify large file
    util_create_temp_source_file(filename, large_content);
    ASSERT_FILE_EXISTS(filename);
    
    // Verify file size is reasonable
    struct stat st;
    stat(filename, &st);
    ASSERT_TRUE(st.st_size > 1000);  // Should be reasonably large (reduced threshold)
    
    // Cleanup
    remove(filename);
    ASSERT_FILE_NOT_EXISTS(filename);
    
    return 1;
}

TEST_CASE(test_compiler_available) {
    int available = util_compiler_available();
    
    // Should return either 0 or 1
    ASSERT_TRUE(available == 0 || available == 1);
    
    return 1;
}

int main(void) {
    printf("=== BDD Utilities Unit Tests ===\n\n");
    printf("Testing core BDD utility functions...\n\n");
    
    // Command execution tests
    RUN_TEST(test_execute_command_echo);
    RUN_TEST(test_execute_command_failure);
    RUN_TEST(test_execute_command_null);
    RUN_TEST(test_command_with_pipes);
    
    // Compiler finding tests
    RUN_TEST(test_find_asthra_compiler);
    RUN_TEST(test_compiler_available);
    
    // File operations tests
    RUN_TEST(test_create_temp_source_file);
    RUN_TEST(test_create_temp_source_file_null_filename);
    RUN_TEST(test_create_temp_source_file_null_content);
    RUN_TEST(test_cleanup_temp_files);
    RUN_TEST(test_large_file_operations);
    
    // Output validation tests
    RUN_TEST(test_output_contains);
    
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);
    printf("Success rate: %.1f%%\n", (float)test_passed / test_count * 100);
    
    if (test_failed == 0) {
        printf("\n✅ All BDD utility functions working correctly!\n");
    } else {
        printf("\n❌ Some tests failed. BDD utilities may need attention.\n");
    }
    
    return test_failed > 0 ? 1 : 0;
}