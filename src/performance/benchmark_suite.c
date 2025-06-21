#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SUITE MANAGEMENT
// =============================================================================

AsthraBenchmarkSuite* asthra_benchmark_suite_create(const char *name) {
    AsthraBenchmarkSuite *suite = malloc(sizeof(AsthraBenchmarkSuite));
    if (!suite) {
        return NULL;
    }
    
    suite->benchmarks = NULL;
    suite->benchmark_count = 0;
    suite->capacity = 0;
    suite->parallel_execution = false;
    
    if (name) {
        strncpy(suite->suite_name, name, ASTHRA_BENCHMARK_MAX_NAME_LENGTH - 1);
        suite->suite_name[ASTHRA_BENCHMARK_MAX_NAME_LENGTH - 1] = '\0';
    } else {
        strcpy(suite->suite_name, "unnamed_suite");
    }
    
    return suite;
}

void asthra_benchmark_suite_destroy(AsthraBenchmarkSuite *suite) {
    if (!suite) {
        return;
    }
    
    free(suite->benchmarks);
    free(suite);
}

AsthraBenchmarkStatus asthra_benchmark_suite_add(
    AsthraBenchmarkSuite *suite,
    const AsthraBenchmarkDefinition *benchmark) {
    
    if (!suite || !benchmark) {
        return ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
    }
    
    if (!asthra_benchmark_definition_validate(benchmark)) {
        return ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
    }
    
    // Resize array if needed
    if (suite->benchmark_count >= suite->capacity) {
        size_t new_capacity = suite->capacity == 0 ? 8 : suite->capacity * 2;
        AsthraBenchmarkDefinition *new_benchmarks = realloc(
            suite->benchmarks, 
            new_capacity * sizeof(AsthraBenchmarkDefinition)
        );
        
        if (!new_benchmarks) {
            return ASTHRA_BENCHMARK_STATUS_MEMORY_ERROR;
        }
        
        suite->benchmarks = new_benchmarks;
        suite->capacity = new_capacity;
    }
    
    suite->benchmarks[suite->benchmark_count++] = *benchmark;
    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
}

AsthraBenchmarkStatus asthra_benchmark_execute_suite(
    const AsthraBenchmarkSuite *suite,
    AsthraBenchmarkResult *results,
    size_t *result_count) {
    
    if (!suite || !results || !result_count) {
        return ASTHRA_BENCHMARK_STATUS_INVALID_CONFIG;
    }
    
    *result_count = 0;
    
    for (size_t i = 0; i < suite->benchmark_count; i++) {
        AsthraBenchmarkStatus status = asthra_benchmark_execute(
            &suite->benchmarks[i], 
            &results[*result_count]
        );
        
        (*result_count)++;
        
        // Continue execution even if individual benchmarks fail
        if (status != ASTHRA_BENCHMARK_STATUS_SUCCESS) {
            printf("Benchmark '%s' failed with status %d\n", 
                   suite->benchmarks[i].config.name, status);
        }
    }
    
    return ASTHRA_BENCHMARK_STATUS_SUCCESS;
} 
