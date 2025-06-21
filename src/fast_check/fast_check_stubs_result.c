// This file provides stub implementations for result handling functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "fast_check_engine.h"
#include "../parser/parser_string_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Include the actual header for proper type definitions
#include "../analysis/semantic_analyzer_core.h"
#include "fast_check_internal.h"

// =============================================================================
// RESULT HANDLING
// =============================================================================

FastCheckResult* fast_check_file(FastCheckEngine *engine, const char *filepath) {
    FastCheckResult *result = calloc(1, sizeof(FastCheckResult));
    if (!result) return NULL;
    
    clock_t start_time = clock();
    
    // Read file contents
    FILE *file = fopen(filepath, "r");
    if (!file) {
        result->success = false;
        result->check_time_ms = 0.1;
        result->error_count = 1;
        result->warning_count = 0;
        return result;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        result->success = false;
        result->check_time_ms = 0.1;
        result->error_count = 1;
        result->warning_count = 0;
        return result;
    }
    
    // Read file content
    char *code = malloc(file_size + 1);
    if (!code) {
        fclose(file);
        result->success = false;
        result->check_time_ms = 0.1;
        result->error_count = 1;
        result->warning_count = 0;
        return result;
    }
    
    size_t bytes_read = fread(code, 1, file_size, file);
    fclose(file);
    code[bytes_read] = '\0';
    
    // Use the code snippet checker with the file content
    FastCheckResult *snippet_result = fast_check_code_snippet(engine, code, filepath);
    
    // Copy result and clean up
    *result = *snippet_result;
    free(snippet_result); // Free the wrapper, not the contents
    free(code);
    
    clock_t end_time = clock();
    result->check_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    return result;
}

FastCheckResult* fast_check_code_snippet(FastCheckEngine *engine, const char *code, const char *context) {
    FastCheckResult *result = calloc(1, sizeof(FastCheckResult));
    if (!result) return NULL;
    
    clock_t start_time = clock();
    
    // Real implementation approach
    if (!code || strlen(code) == 0) {
        result->success = false;
        result->check_time_ms = 0.1;
        result->error_count = 1;
        result->warning_count = 0;
        return result;
    }
    
    // Parse the code using real parser
    ParseResult parse_result = parse_string(code);
    if (!parse_result.success) {
        result->success = false;
        result->error_count = parse_result.error_count;
        result->warning_count = 0;
        cleanup_parse_result(&parse_result);
        
        clock_t end_time = clock();
        result->check_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
        
        if (engine) {
            engine->stats.total_checks++;
        }
        return result;
    }
    
    // Perform semantic analysis if we have an analyzer
    if (engine && engine->analyzer) {
        SemanticAnalyzer *analyzer = (SemanticAnalyzer*)engine->analyzer;
        
        // Reset analyzer state for this check
        semantic_analyzer_reset_internal(analyzer);
        
        // Analyze the AST
        bool semantic_success = semantic_analyze_program_internal(analyzer, parse_result.ast);
        
        result->success = semantic_success;
        result->error_count = semantic_success ? 0 : (int)semantic_get_error_count_internal(analyzer);
        result->warning_count = 0; // TODO: Add warning collection
    } else {
        // No semantic analyzer - just report parse success
        result->success = true;
        result->error_count = 0;
        result->warning_count = 0;
    }
    
    cleanup_parse_result(&parse_result);
    
    clock_t end_time = clock();
    result->check_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    if (engine) {
        engine->stats.total_checks++;
        if (result->success) {
            engine->stats.cache_hits++; // Track successful analyses
        } else {
            engine->stats.cache_misses++; // Track failed analyses
        }
        
        // Update average check time
        if (engine->stats.total_checks > 0) {
            engine->stats.average_check_time_ms = 
                (engine->stats.average_check_time_ms * (engine->stats.total_checks - 1) + result->check_time_ms) 
                / engine->stats.total_checks;
        }
    }
    
    return result;
}

void fast_check_result_destroy(FastCheckResult *result) {
    free(result);
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_result_dummy = 0;

#endif // FAST_CHECK_USE_STUBS