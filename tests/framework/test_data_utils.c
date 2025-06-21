/**
 * Asthra Programming Language
 * Test Data Management Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of test data management functions
 */

#include "test_data_utils.h"
#include "performance_test_utils.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// =============================================================================
// TEST DATA MANAGEMENT
// =============================================================================

char* load_test_file(const char* filename) {
    if (!filename) {
        return NULL;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    char* content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(content, 1, size, file);
    if (read_size != (size_t)size && !feof(file)) {
        free(content);
        fclose(file);
        track_memory_deallocation(size + 1);
        return NULL;
    }
    content[read_size] = '\0';

    fclose(file);
    track_memory_allocation(size + 1);
    return content;
}

bool save_test_output(const char* filename, const char* content) {
    if (!filename || !content) {
        return false;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        return false;
    }

    size_t content_len = strlen(content);
    size_t written = fwrite(content, 1, content_len, file);
    fclose(file);

    return written == content_len;
}

bool compare_test_files(const char* expected_file, const char* actual_file) {
    char* expected_content = load_test_file(expected_file);
    char* actual_content = load_test_file(actual_file);

    if (!expected_content || !actual_content) {
        free(expected_content);
        free(actual_content);
        return false;
    }

    bool equal = strcmp(expected_content, actual_content) == 0;

    free(expected_content);
    free(actual_content);
    return equal;
}

char* create_temp_test_file(const char* content, const char* suffix) {
    if (!content || !suffix) {
        return NULL;
    }

    // Create temporary filename
    char* template = malloc(256);
    if (!template) {
        return NULL;
    }

    snprintf(template, 256, "/tmp/asthra_test_XXXXXX%s", suffix);

    int fd = mkstemps(template, strlen(suffix));
    if (fd == -1) {
        free(template);
        return NULL;
    }

    // Write content to file
    size_t content_len = strlen(content);
    ssize_t written = write(fd, content, content_len);
    close(fd);

    if (written != (ssize_t)content_len) {
        unlink(template);
        free(template);
        return NULL;
    }

    track_memory_allocation(256);
    return template;
}

void cleanup_temp_test_file(const char* filename) {
    if (filename) {
        unlink(filename);
        track_memory_deallocation(256); // Approximate
    }
}
