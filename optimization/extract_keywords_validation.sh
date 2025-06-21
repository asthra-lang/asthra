#!/bin/bash

# Asthra Keyword Extraction - Validation Module
# Copyright (c) 2024 Asthra Project
# 
# Comprehensive validation and testing of generated hash functions
# including compilation tests, performance analysis, and reporting.

set -euo pipefail

# Source common utilities
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/extract_keywords_common.sh"

# Enhanced validation with compilation test and performance analysis
validate_hash_function() {
    log_info "Validating generated hash function..."
    
    # Create a comprehensive test program
    local test_program=$(create_temp_file "test_keywords" ".c")
    local test_binary=$(create_temp_file "test_keywords")
    
    create_test_program "$test_program"
    
    # Try to compile and run the test program
    local compile_success=false
    if compile_test_program "$test_program" "$test_binary"; then
        compile_success=true
        log_success "Test program compiled successfully"
        
        # Run the test
        if run_test_program "$test_binary"; then
            log_success "Hash function validation passed"
        else
            log_warning "Hash function validation failed"
        fi
    else
        log_warning "Test program compilation failed"
    fi
    
    # Generate comprehensive validation report
    generate_validation_report "$compile_success"
    
    # Clean up
    rm -f "$test_program" "$test_binary"
    
    log_success "Comprehensive validation complete. Report saved to: $VALIDATION_LOG"
}

# Create comprehensive test program
create_test_program() {
    local test_program="$1"
    
    log_debug "Creating test program: $test_program"
    
    cat > "$test_program" << EOF
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Include the generated files
#include "asthra_keywords_hash.h"

// Test keywords (should be found)
static const char *test_keywords[] = {
    "if", "else", "for", "fn", "struct", "let", "return",
    "package", "import", "match", "spawn", "unsafe", "true", "false",
    "enum", "bool", "int", "await"
};

// Test non-keywords (should not be found)
static const char *test_non_keywords[] = {
    "variable", "function", "my_struct", "hello_world", "test123",
    "not_a_keyword", "random_identifier", "xyz"
};

int main() {
    printf("=== Asthra Keyword Hash Function Validation ===\\n");
    
    int total_keywords = asthra_keyword_get_count();
    printf("Total keywords in hash table: %d\\n\\n", total_keywords);
    
    // Test known keywords
    printf("Testing known keywords:\\n");
    int keyword_tests = sizeof(test_keywords) / sizeof(test_keywords[0]);
    int keyword_passed = 0;
    
    for (int i = 0; i < keyword_tests; i++) {
        const char *keyword = test_keywords[i];
        size_t len = strlen(keyword);
        int token_type = asthra_keyword_get_token_type(keyword, len);
        int is_keyword = asthra_keyword_is_keyword(keyword, len);
        
        if (token_type != -1 && is_keyword) {
            printf("  ✓ '%s' -> token_type=%d\\n", keyword, token_type);
            keyword_passed++;
        } else {
            printf("  ✗ '%s' -> not found (expected keyword)\\n", keyword);
        }
    }
    
    // Test non-keywords
    printf("\\nTesting non-keywords:\\n");
    int non_keyword_tests = sizeof(test_non_keywords) / sizeof(test_non_keywords[0]);
    int non_keyword_passed = 0;
    
    for (int i = 0; i < non_keyword_tests; i++) {
        const char *identifier = test_non_keywords[i];
        size_t len = strlen(identifier);
        int token_type = asthra_keyword_get_token_type(identifier, len);
        int is_keyword = asthra_keyword_is_keyword(identifier, len);
        
        if (token_type == -1 && !is_keyword) {
            printf("  ✓ '%s' -> not a keyword (correct)\\n", identifier);
            non_keyword_passed++;
        } else {
            printf("  ✗ '%s' -> found as keyword (unexpected)\\n", identifier);
        }
    }
    
    // Performance test
    printf("\\nPerformance test (1000000 lookups):\\n");
    clock_t start = clock();
    
    for (int i = 0; i < 1000000; i++) {
        const char *test_word = test_keywords[i % keyword_tests];
        asthra_keyword_get_token_type(test_word, strlen(test_word));
    }
    
    clock_t end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("  Time for 1M lookups: %.6f seconds\\n", cpu_time);
    printf("  Average time per lookup: %.2f nanoseconds\\n", (cpu_time * 1000000000) / 1000000);
    
    // Summary
    printf("\\n=== Validation Summary ===\\n");
    printf("Keyword tests: %d/%d passed\\n", keyword_passed, keyword_tests);
    printf("Non-keyword tests: %d/%d passed\\n", non_keyword_passed, non_keyword_tests);
    
    int total_passed = keyword_passed + non_keyword_passed;
    int total_tests = keyword_tests + non_keyword_tests;
    
    if (total_passed == total_tests) {
        printf("✓ All tests passed!\\n");
        return 0;
    } else {
        printf("✗ %d/%d tests failed\\n", total_tests - total_passed, total_tests);
        return 1;
    }
}
EOF
}

# Compile test program with proper flags
compile_test_program() {
    local test_program="$1"
    local test_binary="$2"
    
    log_debug "Compiling test program..."
    
    # Try compilation with various flag combinations
    local compile_flags=(
        "-std=c17 -O2 -Wno-deprecated-non-prototype"
        "-std=c99 -O2 -Wno-deprecated-non-prototype"
        "-std=c11 -O2 -Wno-deprecated-non-prototype"
        "-O2 -Wno-deprecated-non-prototype"
        "-std=c17 -O2"
        "-std=c99 -O2"
        "-std=c11 -O2"
        "-O2"
    )
    
    for flags in "${compile_flags[@]}"; do
        log_debug "Trying compilation with flags: $flags"
        log_debug "Test program: $test_program"
        log_debug "C file: $GPERF_OUTPUT_C"
        log_debug "Binary: $test_binary"
        if gcc $flags -I"$OUTPUT_DIR" -I"$PROJECT_ROOT/src/parser" \
               "$test_program" "$GPERF_OUTPUT_C" -o "$test_binary" 2>/dev/null; then
            log_debug "Compilation successful with flags: $flags"
            return 0
        fi
    done
    
    # If all attempts failed, try with verbose output
    log_debug "All compilation attempts failed. Trying with verbose output..."
    gcc -std=c17 -O2 -I"$OUTPUT_DIR" -I"$PROJECT_ROOT/src/parser" \
        "$test_program" "$GPERF_OUTPUT_C" -o "$test_binary" 2>&1 | head -20
    
    return 1
}

# Run test program and capture output
run_test_program() {
    local test_binary="$1"
    
    log_debug "Running test program..."
    
    if [[ -x "$test_binary" ]]; then
        "$test_binary"
        return $?
    else
        log_error "Test binary is not executable: $test_binary"
        return 1
    fi
}

# Generate comprehensive validation report
generate_validation_report() {
    local compile_success="$1"
    
    log_info "Generating validation report..."
    
    {
        echo "=== Asthra Keyword Hash Generation Report ==="
        echo "Generated at: $(date)"
        echo "Script version: $(grep '^# Enhanced version' "$0" | head -1 || echo "Modular version")"
        echo ""
        echo "=== Input Files ==="
        echo "Lexer source: $LEXER_SOURCE"
        echo "Lexer header: $LEXER_HEADER"
        echo "Extract script: $0"
        echo ""
        echo "=== Output Files ==="
        echo "Gperf input: $GPERF_INPUT"
        echo "Generated C file: $GPERF_OUTPUT_C"
        echo "Generated H file: $GPERF_OUTPUT_H"
        echo ""
        echo "=== Statistics ==="
        echo "Total keywords: $(grep -c '^[^%#].*,' "$GPERF_INPUT" 2>/dev/null || echo "0")"
        echo "C file size: $(get_file_size "$GPERF_OUTPUT_C") bytes"
        echo "H file size: $(get_file_size "$GPERF_OUTPUT_H") bytes"
        echo "Gperf input size: $(get_file_size "$GPERF_INPUT") bytes"
        echo ""
        echo "=== Hash Function Properties ==="
        if [[ -f "$GPERF_OUTPUT_C" ]]; then
            grep -E "MIN_WORD_LENGTH|MAX_WORD_LENGTH|MIN_HASH_VALUE|MAX_HASH_VALUE|TOTAL_KEYWORDS" "$GPERF_OUTPUT_C" || echo "Properties not found"
        else
            echo "C file not found"
        fi
        echo ""
        echo "=== Compilation Test ==="
        if [[ "$compile_success" == true ]]; then
            echo "Status: PASSED"
        else
            echo "Status: FAILED"
        fi
        echo ""
        echo "=== Generated Functions ==="
        echo "--- C file functions ---"
        if [[ -f "$GPERF_OUTPUT_C" ]]; then
            grep -E '^(unsigned int|const struct|int) asthra_keyword_' "$GPERF_OUTPUT_C" || echo "No functions found"
        else
            echo "C file not found"
        fi
        echo ""
        echo "--- H file declarations ---"
        if [[ -f "$GPERF_OUTPUT_H" ]]; then
            grep -E '^(unsigned int|const struct|int) asthra_keyword_' "$GPERF_OUTPUT_H" || echo "No declarations found"
        else
            echo "H file not found"
        fi
        echo ""
        echo "=== Integration Instructions ==="
        echo "1. Include the header in your lexer:"
        echo "   #include \"$GPERF_OUTPUT_H\""
        echo ""
        echo "2. Replace keyword_lookup() function with:"
        echo "   TokenType keyword_lookup(const char *identifier, size_t length) {"
        echo "       int token_type = asthra_keyword_get_token_type(identifier, length);"
        echo "       return (token_type != -1) ? token_type : TOKEN_IDENTIFIER;"
        echo "   }"
        echo ""
        echo "3. Compile with the generated object file:"
        echo "   \$(CC) \$(CFLAGS) -I$OUTPUT_DIR lexer.c $GPERF_OUTPUT_C -o lexer"
        echo ""
        echo "=== Quality Metrics ==="
        analyze_quality_metrics
    } > "$VALIDATION_LOG"
}

# Analyze quality metrics for the generated hash function
analyze_quality_metrics() {
    if [[ ! -f "$GPERF_OUTPUT_C" ]]; then
        echo "Cannot analyze quality metrics - C file not found"
        return
    fi
    
    # Count lines of code
    local c_lines=$(count_lines "$GPERF_OUTPUT_C")
    local h_lines=$(count_lines "$GPERF_OUTPUT_H")
    
    echo "Code metrics:"
    echo "  C file lines: $c_lines"
    echo "  H file lines: $h_lines"
    echo "  Total lines: $((c_lines + h_lines))"
    
    # Check for common quality indicators
    local has_const_tables=$(grep -c "const.*\[\]" "$GPERF_OUTPUT_C" || echo "0")
    local has_inline_funcs=$(grep -c "inline" "$GPERF_OUTPUT_C" || echo "0")
    local has_switch_stmt=$(grep -c "switch" "$GPERF_OUTPUT_C" || echo "0")
    
    echo "Quality indicators:"
    echo "  Const tables: $has_const_tables"
    echo "  Inline functions: $has_inline_funcs"
    echo "  Switch statements: $has_switch_stmt"
    
    # Estimate memory usage
    local estimated_memory=$((has_const_tables * 100 + c_lines * 10))
    echo "  Estimated memory usage: ~${estimated_memory} bytes"
}

# Performance benchmark for the hash function
benchmark_hash_function() {
    log_info "Running performance benchmark..."
    
    if [[ ! -f "$GPERF_OUTPUT_C" ]] || [[ ! -f "$GPERF_OUTPUT_H" ]]; then
        log_warning "Generated files not found, skipping benchmark"
        return
    fi
    
    local benchmark_program=$(create_temp_file "benchmark" ".c")
    local benchmark_binary=$(create_temp_file "benchmark")
    
    create_benchmark_program "$benchmark_program"
    
    if compile_test_program "$benchmark_program" "$benchmark_binary"; then
        log_info "Running benchmark..."
        if "$benchmark_binary"; then
            log_success "Benchmark completed successfully"
        else
            log_warning "Benchmark execution failed"
        fi
    else
        log_warning "Benchmark compilation failed"
    fi
    
    rm -f "$benchmark_program" "$benchmark_binary"
}

# Create performance benchmark program
create_benchmark_program() {
    local benchmark_program="$1"
    
    cat > "$benchmark_program" << 'EOF'
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// Include the generated files
#include "asthra_keywords_hash.h"

// Extended test keywords
static const char *keywords[] = {
    "if", "else", "for", "while", "fn", "struct", "let", "return",
    "package", "import", "match", "spawn", "unsafe", "true", "false",
    "const", "mut", "ref", "move", "copy", "async", "await"
};

static const char *non_keywords[] = {
    "variable", "function", "my_struct", "hello_world", "test123",
    "not_a_keyword", "random_identifier", "xyz", "foo", "bar",
    "baz", "qux", "identifier_name", "very_long_identifier_name"
};

int main() {
    printf("=== Asthra Keyword Hash Performance Benchmark ===\n");
    
    int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    int non_keyword_count = sizeof(non_keywords) / sizeof(non_keywords[0]);
    
    // Warm up
    for (int i = 0; i < 1000; i++) {
        asthra_keyword_get_token_type(keywords[i % keyword_count], strlen(keywords[i % keyword_count]));
    }
    
    // Benchmark keyword lookups
    clock_t start = clock();
    for (int i = 0; i < 1000000; i++) {
        const char *word = keywords[i % keyword_count];
        asthra_keyword_get_token_type(word, strlen(word));
    }
    clock_t end = clock();
    
    double keyword_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Keyword lookups (1M): %.6f seconds (%.2f ns/lookup)\n", 
           keyword_time, (keyword_time * 1000000000) / 1000000);
    
    // Benchmark non-keyword lookups
    start = clock();
    for (int i = 0; i < 1000000; i++) {
        const char *word = non_keywords[i % non_keyword_count];
        asthra_keyword_get_token_type(word, strlen(word));
    }
    end = clock();
    
    double non_keyword_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Non-keyword lookups (1M): %.6f seconds (%.2f ns/lookup)\n", 
           non_keyword_time, (non_keyword_time * 1000000000) / 1000000);
    
    // Mixed benchmark
    start = clock();
    for (int i = 0; i < 1000000; i++) {
        const char *word = (i % 2 == 0) ? keywords[i % keyword_count] : non_keywords[i % non_keyword_count];
        asthra_keyword_get_token_type(word, strlen(word));
    }
    end = clock();
    
    double mixed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Mixed lookups (1M): %.6f seconds (%.2f ns/lookup)\n", 
           mixed_time, (mixed_time * 1000000000) / 1000000);
    
    printf("\nPerformance summary:\n");
    printf("  Average lookup time: %.2f nanoseconds\n", 
           ((keyword_time + non_keyword_time + mixed_time) * 1000000000) / 3000000);
    
    return 0;
}
EOF
}

# Export functions for use in other scripts
export -f validate_hash_function create_test_program compile_test_program
export -f run_test_program generate_validation_report analyze_quality_metrics
export -f benchmark_hash_function create_benchmark_program 
