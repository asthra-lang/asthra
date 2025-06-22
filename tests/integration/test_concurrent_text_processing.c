/**
 * Asthra Programming Language v1.2 Integration Tests
 * Concurrent Text Processing with FFI and Error Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_integration_common.h"

// =============================================================================
// CONCURRENT TEXT PROCESSING INTEGRATION TEST
// =============================================================================

// Scenario 1: Concurrent string processing with FFI and error handling
typedef struct {
    char *input_text;
    char *processed_text;
    int processing_id;
    bool success;
    const char *error_message;
} TextProcessingTask;

// Mock C library function for text processing
static char* c_text_processor(const char *input, int mode) {
    if (!input) return NULL;
    
    size_t len = strlen(input);
    char *result = malloc(len + 50);
    if (!result) return NULL;
    
    switch (mode) {
        case 1: // Uppercase
            strncpy(result, input, len + 20);
            result[len + 19] = '\0';
            for (char *p = result; *p; p++) {
                if (*p >= 'a' && *p <= 'z') {
                    *p = *p - 'a' + 'A';
                }
            }
            break;
            
        case 2: // Add prefix
            snprintf(result, len + 20, "PROCESSED: %s", input);
            break;
            
        case 3: // Reverse
            strncpy(result, input, len + 20);
            result[len + 19] = '\0';
            for (size_t i = 0; i < len / 2; i++) {
                char temp = result[i];
                result[i] = result[len - 1 - i];
                result[len - 1 - i] = temp;
            }
            break;
            
        default:
            free(result);
            return NULL;
    }
    
    return result;
}

static void* concurrent_text_processing(void *arg) {
    TextProcessingTask *task = (TextProcessingTask*)arg;
    
    // Simulate pattern matching on processing mode
    int mode = (task->processing_id % 3) + 1;
    
    // Call FFI function
    char *result = c_text_processor(task->input_text, mode);
    
    if (result) {
        task->processed_text = result;
        task->success = true;
        task->error_message = NULL;
    } else {
        task->processed_text = NULL;
        task->success = false;
        task->error_message = "Processing failed";
    }
    
    return task;
}

AsthraTestResult test_integration_concurrent_text_processing(AsthraV12TestContext *ctx) {
    // Integration test: Concurrency + FFI + Error Handling + String Operations
    
    const int num_tasks = 6;
    TextProcessingTask tasks[num_tasks];
    pthread_t threads[num_tasks];
    
    // Initialize tasks
    const char *test_inputs[] = {
        "hello world",
        "asthra language",
        "concurrent processing",
        "ffi integration",
        "error handling",
        "string operations"
    };
    
    for (int i = 0; i < num_tasks; i++) {
        tasks[i].input_text = strdup(test_inputs[i]);
        tasks[i].processed_text = NULL;
        tasks[i].processing_id = i;
        tasks[i].success = false;
        tasks[i].error_message = NULL;
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, tasks[i].input_text != NULL,
                               "Task %d input allocation should succeed", i)) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                free(tasks[j].input_text);
            }
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Start concurrent processing
    for (int i = 0; i < num_tasks; i++) {
        int result = pthread_create(&threads[i], NULL, concurrent_text_processing, &tasks[i]);
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, result == 0,
                               "Thread %d creation should succeed", i)) {
            // Cleanup
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            for (int j = 0; j < num_tasks; j++) {
                free(tasks[j].input_text);
            }
            return ASTHRA_TEST_FAIL;
        }
    }
    
    // Wait for completion
    for (int i = 0; i < num_tasks; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify results
    int success_count = 0;
    for (int i = 0; i < num_tasks; i++) {
        if (!ASTHRA_TEST_ASSERT(&ctx->base, tasks[i].success,
                               "Task %d should succeed", i)) {
            // Cleanup
            for (int j = 0; j < num_tasks; j++) {
                free(tasks[j].input_text);
                if (tasks[j].processed_text) free(tasks[j].processed_text);
            }
            return ASTHRA_TEST_FAIL;
        }
        
        if (!ASTHRA_TEST_ASSERT(&ctx->base, tasks[i].processed_text != NULL,
                               "Task %d should have processed text", i)) {
            // Cleanup
            for (int j = 0; j < num_tasks; j++) {
                free(tasks[j].input_text);
                if (tasks[j].processed_text) free(tasks[j].processed_text);
            }
            return ASTHRA_TEST_FAIL;
        }
        
        // Verify processing based on mode
        int mode = (i % 3) + 1;
        switch (mode) {
            case 1: // Uppercase
                if (!ASTHRA_TEST_ASSERT(&ctx->base, strstr(tasks[i].processed_text, "HELLO") != NULL || 
                                       strstr(tasks[i].processed_text, "ASTHRA") != NULL ||
                                       strstr(tasks[i].processed_text, "CONCURRENT") != NULL,
                                       "Task %d should contain uppercase text", i)) {
                    // Cleanup
                    for (int j = 0; j < num_tasks; j++) {
                        free(tasks[j].input_text);
                        if (tasks[j].processed_text) free(tasks[j].processed_text);
                    }
                    return ASTHRA_TEST_FAIL;
                }
                break;
                
            case 2: // Prefix
                if (!ASTHRA_TEST_ASSERT(&ctx->base, strstr(tasks[i].processed_text, "PROCESSED:") != NULL,
                                       "Task %d should have PROCESSED prefix", i)) {
                    // Cleanup
                    for (int j = 0; j < num_tasks; j++) {
                        free(tasks[j].input_text);
                        if (tasks[j].processed_text) free(tasks[j].processed_text);
                    }
                    return ASTHRA_TEST_FAIL;
                }
                break;
                
            case 3: // Reverse - just check it's different from input
                if (!ASTHRA_TEST_ASSERT(&ctx->base, strcmp(tasks[i].processed_text, tasks[i].input_text) != 0,
                                       "Task %d should have different text after reversal", i)) {
                    // Cleanup
                    for (int j = 0; j < num_tasks; j++) {
                        free(tasks[j].input_text);
                        if (tasks[j].processed_text) free(tasks[j].processed_text);
                    }
                    return ASTHRA_TEST_FAIL;
                }
                break;
        }
        
        success_count++;
    }
    
    if (!ASTHRA_TEST_ASSERT(&ctx->base, success_count == num_tasks,
                           "All %d tasks should succeed", num_tasks)) {
        // Cleanup
        for (int i = 0; i < num_tasks; i++) {
            free(tasks[i].input_text);
            if (tasks[i].processed_text) free(tasks[i].processed_text);
        }
        return ASTHRA_TEST_FAIL;
    }
    
    // Cleanup
    for (int i = 0; i < num_tasks; i++) {
        free(tasks[i].input_text);
        if (tasks[i].processed_text) free(tasks[i].processed_text);
    }
    
    return ASTHRA_TEST_PASS;
} 
