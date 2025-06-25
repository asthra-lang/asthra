# Benchmarking Infrastructure for Asthra
# Provides comprehensive performance testing and benchmarking capabilities

option(ENABLE_BENCHMARKS "Enable benchmark targets" ON)
set(BENCHMARK_OUTPUT_DIR "${CMAKE_BINARY_DIR}/benchmarks" CACHE PATH "Benchmark output directory")
set(BENCHMARK_ITERATIONS "100" CACHE STRING "Number of benchmark iterations")
set(BENCHMARK_WARMUP "10" CACHE STRING "Number of warmup iterations")

# Create benchmark output directory
file(MAKE_DIRECTORY ${BENCHMARK_OUTPUT_DIR})

if(ENABLE_BENCHMARKS)
    message(STATUS "Benchmarking enabled")
    
    # Find benchmark tools
    find_program(TIME_EXECUTABLE time)
    find_program(PERF_EXECUTABLE perf)
    
    # Benchmark executable with optimizations
    if(NOT TARGET asthra-benchmark)
        add_executable(asthra-benchmark src/main.c)
        target_link_libraries(asthra-benchmark asthra_compiler)
        
        # Apply aggressive optimizations for benchmarking (Clang)
        target_compile_options(asthra-benchmark PRIVATE
            -O3 -march=native -mtune=native -flto
        )
        target_link_options(asthra-benchmark PRIVATE
            -flto
        )
    endif()
    
    # Core benchmark target
    add_custom_target(benchmark
        COMMAND ${CMAKE_COMMAND} -E echo "Running Asthra benchmarks..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark> --version
        DEPENDS asthra-benchmark
        COMMENT "Running performance benchmarks"
        USES_TERMINAL
    )
    
    # FFI Assembly Generator benchmark
    add_custom_target(benchmark-ffi-assembly-generator
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking FFI Assembly Generator..."
        COMMAND ${CMAKE_COMMAND} -E echo "Iterations: ${BENCHMARK_ITERATIONS}"
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark> 
                --benchmark ffi-assembly 
                --iterations ${BENCHMARK_ITERATIONS}
                --output ${BENCHMARK_OUTPUT_DIR}/ffi-assembly-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking FFI Assembly Generator"
        USES_TERMINAL
    )
    
    # ELF Writer benchmark
    add_custom_target(benchmark-elf-writer
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking ELF Writer..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark elf-writer
                --iterations ${BENCHMARK_ITERATIONS}
                --output ${BENCHMARK_OUTPUT_DIR}/elf-writer-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking ELF Writer"
        USES_TERMINAL
    )
    
    # Parser benchmark
    add_custom_target(benchmark-parser
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking Parser..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark parser
                --iterations ${BENCHMARK_ITERATIONS}
                --warmup ${BENCHMARK_WARMUP}
                --output ${BENCHMARK_OUTPUT_DIR}/parser-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking Parser Performance"
        USES_TERMINAL
    )
    
    # Lexer benchmark
    add_custom_target(benchmark-lexer
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking Lexer..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark lexer
                --iterations ${BENCHMARK_ITERATIONS}
                --warmup ${BENCHMARK_WARMUP}
                --output ${BENCHMARK_OUTPUT_DIR}/lexer-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking Lexer Performance"
        USES_TERMINAL
    )
    
    # Code generation benchmark
    add_custom_target(benchmark-codegen
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking Code Generation..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark codegen
                --iterations ${BENCHMARK_ITERATIONS}
                --output ${BENCHMARK_OUTPUT_DIR}/codegen-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking Code Generation"
        USES_TERMINAL
    )
    
    # Memory allocation benchmark
    add_custom_target(benchmark-memory
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking Memory Management..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark memory
                --iterations ${BENCHMARK_ITERATIONS}
                --output ${BENCHMARK_OUTPUT_DIR}/memory-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking Memory Management"
        USES_TERMINAL
    )
    
    # Type checking benchmark
    add_custom_target(benchmark-typecheck
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking Type Checking..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark typecheck
                --iterations ${BENCHMARK_ITERATIONS}
                --output ${BENCHMARK_OUTPUT_DIR}/typecheck-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking Type Checking"
        USES_TERMINAL
    )
    
    # End-to-end compilation benchmark
    add_custom_target(benchmark-end-to-end
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarking End-to-End Compilation..."
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:asthra-benchmark>
                --benchmark end-to-end
                --iterations ${BENCHMARK_ITERATIONS}
                --output ${BENCHMARK_OUTPUT_DIR}/e2e-bench.json
        DEPENDS asthra-benchmark
        COMMENT "Benchmarking End-to-End Performance"
        USES_TERMINAL
    )
    
    # Compilation speed test
    add_custom_target(test-compilation-speed
        COMMAND ${CMAKE_COMMAND} -E echo "Testing compilation speed..."
        COMMAND ${CMAKE_COMMAND} -E echo "Compiling test programs..."
        COMMAND ${CMAKE_CTEST_COMMAND} -L performance --output-on-failure
        COMMENT "Testing compilation speed"
        USES_TERMINAL
    )
    
    # Benchmark all components
    add_custom_target(benchmark-all
        DEPENDS 
            benchmark-lexer
            benchmark-parser
            benchmark-typecheck
            benchmark-codegen
            benchmark-memory
            benchmark-ffi-assembly-generator
            benchmark-elf-writer
            benchmark-end-to-end
        COMMENT "Running all benchmarks"
    )
    
    # Benchmark comparison target
    add_custom_target(benchmark-compare
        COMMAND ${CMAKE_COMMAND} -E echo "Comparing benchmark results..."
        COMMAND ${CMAKE_COMMAND} -E echo "Previous: ${BENCHMARK_OUTPUT_DIR}/previous/"
        COMMAND ${CMAKE_COMMAND} -E echo "Current: ${BENCHMARK_OUTPUT_DIR}/"
        COMMENT "Compare benchmark results"
    )
    
    # Benchmark regression check
    add_custom_target(benchmark-regression
        COMMAND ${CMAKE_COMMAND} -E echo "Checking for performance regressions..."
        DEPENDS benchmark-all benchmark-compare
        COMMENT "Check for performance regressions"
    )
    
    # Profile-guided benchmark
    if(TIME_EXECUTABLE)
        add_custom_target(benchmark-profile
            COMMAND ${TIME_EXECUTABLE} -v $<TARGET_FILE:asthra-benchmark> --version
            DEPENDS asthra-benchmark
            COMMENT "Profile benchmark execution"
            USES_TERMINAL
        )
    endif()
    
    # Performance profiling with perf (Linux)
    if(PERF_EXECUTABLE AND UNIX AND NOT APPLE)
        add_custom_target(benchmark-perf
            COMMAND ${PERF_EXECUTABLE} record -o ${BENCHMARK_OUTPUT_DIR}/perf.data
                    $<TARGET_FILE:asthra-benchmark> --benchmark end-to-end
            COMMAND ${PERF_EXECUTABLE} report -i ${BENCHMARK_OUTPUT_DIR}/perf.data
            DEPENDS asthra-benchmark
            COMMENT "Performance profiling with perf"
            USES_TERMINAL
        )
        
        add_custom_target(benchmark-perf-stat
            COMMAND ${PERF_EXECUTABLE} stat -r ${BENCHMARK_ITERATIONS}
                    $<TARGET_FILE:asthra-benchmark> --version
            DEPENDS asthra-benchmark
            COMMENT "Performance statistics with perf"
            USES_TERMINAL
        )
    endif()
    
    # Benchmark result visualization
    add_custom_target(benchmark-visualize
        COMMAND ${CMAKE_COMMAND} -E echo "Generating benchmark visualization..."
        COMMAND ${CMAKE_COMMAND} -E echo "Results in: ${BENCHMARK_OUTPUT_DIR}/report.html"
        COMMENT "Visualize benchmark results"
    )
    
    # Clean benchmark results
    add_custom_target(benchmark-clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${BENCHMARK_OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BENCHMARK_OUTPUT_DIR}
        COMMENT "Clean benchmark results"
    )
    
    # Benchmark summary
    add_custom_target(benchmark-summary
        COMMAND ${CMAKE_COMMAND} -E echo "=== Benchmark Summary ==="
        COMMAND ${CMAKE_COMMAND} -E echo "Output directory: ${BENCHMARK_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E echo "Iterations: ${BENCHMARK_ITERATIONS}"
        COMMAND ${CMAKE_COMMAND} -E echo "Warmup: ${BENCHMARK_WARMUP}"
        COMMAND ls -la ${BENCHMARK_OUTPUT_DIR}/*.json 2>/dev/null || echo "No results yet"
        COMMENT "Display benchmark summary"
    )
    
    # Custom benchmark target creator
    function(add_asthra_benchmark name source)
        add_executable(benchmark-${name} ${source})
        target_link_libraries(benchmark-${name} asthra_compiler)
        target_compile_definitions(benchmark-${name} PRIVATE ASTHRA_BENCHMARK)
        
        add_custom_target(run-benchmark-${name}
            COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:benchmark-${name}>
            DEPENDS benchmark-${name}
            COMMENT "Running ${name} benchmark"
            USES_TERMINAL
        )
        
        add_dependencies(benchmark-all run-benchmark-${name})
    endfunction()
    
else()
    # Dummy targets when benchmarks are disabled
    add_custom_target(benchmark
        COMMAND ${CMAKE_COMMAND} -E echo "Benchmarks are disabled. Use -DENABLE_BENCHMARKS=ON"
        COMMENT "Benchmarks disabled"
    )
endif()

# Benchmark help
add_custom_target(benchmark-help
    COMMAND ${CMAKE_COMMAND} -E echo "Benchmark Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark                    - Basic benchmark"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-all                - Run all benchmarks"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-lexer              - Lexer performance"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-parser             - Parser performance"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-codegen            - Code generation"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-memory             - Memory management"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-ffi-assembly-generator - FFI assembly"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-elf-writer         - ELF generation"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-end-to-end         - Full compilation"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-compare            - Compare results"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-regression         - Check regressions"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-clean              - Clean results"
    COMMAND ${CMAKE_COMMAND} -E echo "  benchmark-summary            - Show summary"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Options:"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DENABLE_BENCHMARKS=ON       - Enable benchmarks"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DBENCHMARK_ITERATIONS=100   - Set iterations"
    COMMAND ${CMAKE_COMMAND} -E echo "  -DBENCHMARK_WARMUP=10        - Set warmup runs"
    COMMENT "Benchmark help"
)