/**
 * Backend Performance Benchmarks
 * Comprehensive benchmarks comparing compilation speed and output quality across backends
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../../../src/compiler.h"
#include "../../../src/codegen/backend_interface.h"

// Benchmark configuration
#define BENCHMARK_ITERATIONS 5
#define MAX_BACKENDS 3
#define MAX_TEST_PROGRAMS 4

// Test program definitions
typedef struct {
    const char* name;
    const char* description;
    const char* code;
    int expected_complexity; // Relative complexity score
} TestProgram;

// Benchmark results
typedef struct {
    AsthraBackendType backend_type;
    const char* backend_name;
    double avg_compile_time;
    double min_compile_time;
    double max_compile_time;
    size_t avg_output_size;
    size_t avg_lines_generated;
    size_t avg_functions_processed;
    int success_rate; // Percentage of successful compilations
    bool available; // Whether backend is available for testing
} BackendBenchmarkResult;

typedef struct {
    TestProgram program;
    BackendBenchmarkResult results[MAX_BACKENDS];
    int num_backends;
} ProgramBenchmarkResult;

// Test programs of varying complexity
static TestProgram test_programs[MAX_TEST_PROGRAMS] = {
    {
        .name = "simple",
        .description = "Simple main function returning constant",
        .expected_complexity = 1,
        .code = 
            "fn main() -> i32 {\n"
            "    return 42;\n"
            "}\n"
    },
    {
        .name = "arithmetic",
        .description = "Basic arithmetic operations",
        .expected_complexity = 2,
        .code = 
            "fn main() -> i32 {\n"
            "    let a: i32 = 10;\n"
            "    let b: i32 = 20;\n"
            "    let c: i32 = 15;\n"
            "    return (a + b) * c - 5;\n"
            "}\n"
    },
    {
        .name = "functions",
        .description = "Multiple functions with calls",
        .expected_complexity = 3,
        .code = 
            "fn add(x: i32, y: i32) -> i32 {\n"
            "    return x + y;\n"
            "}\n"
            "\n"
            "fn multiply(x: i32, y: i32) -> i32 {\n"
            "    return x * y;\n"
            "}\n"
            "\n"
            "fn calculate(a: i32, b: i32, c: i32) -> i32 {\n"
            "    let sum: i32 = add(a, b);\n"
            "    return multiply(sum, c);\n"
            "}\n"
            "\n"
            "fn main() -> i32 {\n"
            "    return calculate(5, 10, 3);\n"
            "}\n"
    },
    {
        .name = "control_flow",
        .description = "Control flow with loops and conditionals",
        .expected_complexity = 4,
        .code = 
            "fn fibonacci(n: i32) -> i32 {\n"
            "    if n <= 1 {\n"
            "        return n;\n"
            "    }\n"
            "    \n"
            "    let a: i32 = 0;\n"
            "    let b: i32 = 1;\n"
            "    let i: i32 = 2;\n"
            "    \n"
            "    while i <= n {\n"
            "        let temp: i32 = a + b;\n"
            "        a = b;\n"
            "        b = temp;\n"
            "        i = i + 1;\n"
            "    }\n"
            "    \n"
            "    return b;\n"
            "}\n"
            "\n"
            "fn main() -> i32 {\n"
            "    return fibonacci(10);\n"
            "}\n"
    }
};

// Utility functions
static bool write_test_file(const char* filename, const char* content) {
    FILE *f = fopen(filename, "w");
    if (!f) return false;
    
    fprintf(f, "%s", content);
    fclose(f);
    return true;
}

static size_t get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return 0;
}

static double get_time_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

static bool run_backend_compilation(const char* input_file, const char* output_file, 
                                   AsthraBackendType backend_type, 
                                   double* compile_time, 
                                   size_t* output_size,
                                   size_t* lines_generated,
                                   size_t* functions_processed) {
    struct timespec start, end;
    
    // Create compiler options
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.input_file = input_file;
    options.output_file = output_file;
    options.backend_type = backend_type;
    options.verbose = false;
    
    // Create compiler context
    AsthraCompilerContext *ctx = asthra_compiler_create(&options);
    if (!ctx) return false;
    
    // Time the compilation
    clock_gettime(CLOCK_MONOTONIC, &start);
    int result = asthra_compile_file(ctx, input_file, output_file);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    *compile_time = get_time_diff(start, end);
    
    if (result == 0) {
        *output_size = get_file_size(output_file);
        
        // Try to get backend statistics if available
        // Note: This would require accessing the backend from the context
        // For now, we'll use placeholder values
        *lines_generated = *output_size / 20; // Rough estimate
        *functions_processed = 1; // Minimum estimate
    } else {
        *output_size = 0;
        *lines_generated = 0;
        *functions_processed = 0;
    }
    
    asthra_compiler_destroy(ctx);
    return result == 0;
}

static const char* get_backend_name(AsthraBackendType backend) {
    switch (backend) {
        case ASTHRA_BACKEND_C: return "C Transpiler";
        case ASTHRA_BACKEND_LLVM_IR: return "LLVM IR";
        case ASTHRA_BACKEND_ASSEMBLY: return "Assembly";
        default: return "Unknown";
    }
}

static bool is_backend_available(AsthraBackendType backend) {
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.backend_type = backend;
    
    AsthraBackend *test_backend = asthra_backend_create(&options);
    if (!test_backend) return false;
    
    int result = asthra_backend_initialize(test_backend, &options);
    bool available = (result == 0);
    
    asthra_backend_destroy(test_backend);
    return available;
}

static void benchmark_program_with_backend(TestProgram* program, 
                                         AsthraBackendType backend_type,
                                         BackendBenchmarkResult* result) {
    char input_file[256];
    char output_file[256];
    snprintf(input_file, sizeof(input_file), "/tmp/benchmark_%s.asthra", program->name);
    
    // Determine output file extension
    const char* ext = asthra_backend_get_file_extension(backend_type);
    snprintf(output_file, sizeof(output_file), "/tmp/benchmark_%s_output.%s", 
             program->name, ext);
    
    // Initialize result
    result->backend_type = backend_type;
    result->backend_name = get_backend_name(backend_type);
    result->available = is_backend_available(backend_type);
    
    if (!result->available) {
        result->success_rate = 0;
        result->avg_compile_time = 0.0;
        result->min_compile_time = 0.0;
        result->max_compile_time = 0.0;
        result->avg_output_size = 0;
        result->avg_lines_generated = 0;
        result->avg_functions_processed = 0;
        return;
    }
    
    // Write test file
    if (!write_test_file(input_file, program->code)) {
        result->available = false;
        return;
    }
    
    // Run benchmark iterations
    double times[BENCHMARK_ITERATIONS];
    size_t sizes[BENCHMARK_ITERATIONS];
    size_t lines[BENCHMARK_ITERATIONS];
    size_t functions[BENCHMARK_ITERATIONS];
    int successes = 0;
    
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        double compile_time;
        size_t output_size, lines_generated, functions_processed;
        
        bool success = run_backend_compilation(input_file, output_file, backend_type,
                                              &compile_time, &output_size, 
                                              &lines_generated, &functions_processed);
        
        if (success) {
            times[successes] = compile_time;
            sizes[successes] = output_size;
            lines[successes] = lines_generated;
            functions[successes] = functions_processed;
            successes++;
            
            // Clean up output file for next iteration
            unlink(output_file);
        }
    }
    
    // Calculate statistics
    result->success_rate = (successes * 100) / BENCHMARK_ITERATIONS;
    
    if (successes > 0) {
        // Calculate averages
        double total_time = 0.0;
        size_t total_size = 0;
        size_t total_lines = 0;
        size_t total_functions = 0;
        
        result->min_compile_time = times[0];
        result->max_compile_time = times[0];
        
        for (int i = 0; i < successes; i++) {
            total_time += times[i];
            total_size += sizes[i];
            total_lines += lines[i];
            total_functions += functions[i];
            
            if (times[i] < result->min_compile_time) {
                result->min_compile_time = times[i];
            }
            if (times[i] > result->max_compile_time) {
                result->max_compile_time = times[i];
            }
        }
        
        result->avg_compile_time = total_time / successes;
        result->avg_output_size = total_size / successes;
        result->avg_lines_generated = total_lines / successes;
        result->avg_functions_processed = total_functions / successes;
    }
    
    // Cleanup
    unlink(input_file);
    unlink(output_file);
}

static void print_benchmark_header(void) {
    printf("=== Backend Performance Benchmarks ===\n\n");
    printf("Configuration:\n");
    printf("  Iterations per test: %d\n", BENCHMARK_ITERATIONS);
    printf("  Test programs: %d\n", MAX_TEST_PROGRAMS);
    printf("  Backends tested: C, Assembly, LLVM IR\n");
    printf("\n");
}

static void print_program_results(ProgramBenchmarkResult* result) {
    printf("Program: %s (%s)\n", result->program.name, result->program.description);
    printf("Expected complexity: %d/5\n", result->program.expected_complexity);
    printf("\n");
    
    printf("%-15s %-10s %-12s %-12s %-12s %-10s %-8s\n",
           "Backend", "Available", "Avg Time(s)", "Min Time(s)", "Max Time(s)", 
           "Size(B)", "Success%");
    printf("%-15s %-10s %-12s %-12s %-12s %-10s %-8s\n",
           "===============", "=========", "===========", "===========", "===========",
           "=======", "========");
    
    for (int i = 0; i < result->num_backends; i++) {
        BackendBenchmarkResult* backend = &result->results[i];
        
        if (backend->available) {
            printf("%-15s %-10s %-12.3f %-12.3f %-12.3f %-10zu %-8d\n",
                   backend->backend_name, "Yes", 
                   backend->avg_compile_time, backend->min_compile_time, backend->max_compile_time,
                   backend->avg_output_size, backend->success_rate);
        } else {
            printf("%-15s %-10s %-12s %-12s %-12s %-10s %-8s\n",
                   backend->backend_name, "No", "N/A", "N/A", "N/A", "N/A", "N/A");
        }
    }
    printf("\n");
}

static void print_summary_results(ProgramBenchmarkResult results[], int num_programs) {
    printf("=== Performance Summary ===\n\n");
    
    // Calculate averages across all programs
    double c_avg_time = 0.0, asm_avg_time = 0.0, llvm_avg_time = 0.0;
    int c_available = 0, asm_available = 0, llvm_available = 0;
    
    for (int i = 0; i < num_programs; i++) {
        for (int j = 0; j < results[i].num_backends; j++) {
            BackendBenchmarkResult* backend = &results[i].results[j];
            
            if (backend->available && backend->success_rate > 0) {
                switch (backend->backend_type) {
                    case ASTHRA_BACKEND_C:
                        c_avg_time += backend->avg_compile_time;
                        c_available++;
                        break;
                    case ASTHRA_BACKEND_ASSEMBLY:
                        asm_avg_time += backend->avg_compile_time;
                        asm_available++;
                        break;
                    case ASTHRA_BACKEND_LLVM_IR:
                        llvm_avg_time += backend->avg_compile_time;
                        llvm_available++;
                        break;
                }
            }
        }
    }
    
    printf("Average compilation times across all test programs:\n");
    if (c_available > 0) {
        printf("  C Backend:        %.3f seconds\n", c_avg_time / c_available);
    } else {
        printf("  C Backend:        Not available or failed\n");
    }
    
    if (asm_available > 0) {
        printf("  Assembly Backend: %.3f seconds\n", asm_avg_time / asm_available);
    } else {
        printf("  Assembly Backend: Not available or failed\n");
    }
    
    if (llvm_available > 0) {
        printf("  LLVM Backend:     %.3f seconds\n", llvm_avg_time / llvm_available);
    } else {
        printf("  LLVM Backend:     Not available or failed\n");
    }
    
    printf("\n");
    
    // Performance recommendations
    printf("=== Recommendations ===\n");
    
    if (c_available > 0) {
        printf("✓ C Backend: Stable and reliable for production use\n");
    }
    
    if (asm_available > 0) {
        printf("✓ Assembly Backend: Available for direct assembly output\n");
    } else {
        printf("⚠ Assembly Backend: Not available or needs implementation\n");
    }
    
    if (llvm_available > 0) {
        printf("✓ LLVM Backend: Available for advanced optimizations\n");
    } else {
        printf("⚠ LLVM Backend: Not compiled in or needs implementation\n");
    }
}

int main(void) {
    print_benchmark_header();
    
    ProgramBenchmarkResult program_results[MAX_TEST_PROGRAMS];
    
    // Benchmark each test program
    for (int i = 0; i < MAX_TEST_PROGRAMS; i++) {
        printf("Benchmarking: %s\n", test_programs[i].name);
        
        program_results[i].program = test_programs[i];
        program_results[i].num_backends = 3;
        
        // Test all backends
        AsthraBackendType backends[] = {
            ASTHRA_BACKEND_C,
            ASTHRA_BACKEND_ASSEMBLY,
            ASTHRA_BACKEND_LLVM_IR
        };
        
        for (int j = 0; j < 3; j++) {
            printf("  Testing %s backend...\n", get_backend_name(backends[j]));
            benchmark_program_with_backend(&test_programs[i], backends[j], 
                                         &program_results[i].results[j]);
        }
        
        printf("  Completed benchmarking %s\n\n", test_programs[i].name);
    }
    
    // Print detailed results
    printf("\n=== Detailed Results ===\n\n");
    for (int i = 0; i < MAX_TEST_PROGRAMS; i++) {
        print_program_results(&program_results[i]);
    }
    
    // Print summary
    print_summary_results(program_results, MAX_TEST_PROGRAMS);
    
    printf("=== Benchmark Complete ===\n");
    return 0;
}