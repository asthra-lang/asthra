#!/bin/bash

# Asthra Keyword Extraction and Gperf Generation Script
# Copyright (c) 2024 Asthra Project
# 
# This script automatically extracts keywords from the lexer source code,
# generates optimized Gperf input files, and creates both .c and .h files
# for perfect hash keyword lookup.
#
# Enhanced version with smart change detection and category handling

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
LEXER_SOURCE="$PROJECT_ROOT/src/parser/lexer.c"
LEXER_HEADER="$PROJECT_ROOT/src/parser/lexer.h"
OUTPUT_DIR="$SCRIPT_DIR"
GPERF_INPUT="$OUTPUT_DIR/asthra_keywords.gperf"
GPERF_OUTPUT_C="$OUTPUT_DIR/asthra_keywords_hash.c"
GPERF_OUTPUT_H="$OUTPUT_DIR/asthra_keywords_hash.h"
VALIDATION_LOG="$OUTPUT_DIR/keyword_validation.log"
CHECKSUM_FILE="$OUTPUT_DIR/.keyword_checksums"
TIMESTAMP_FILE="$OUTPUT_DIR/.last_generation"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1" >&2
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" >&2
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1" >&2
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

log_debug() {
    if [[ "${GPERF_DEBUG:-}" == "1" ]]; then
        echo -e "${CYAN}[DEBUG]${NC} $1" >&2
    fi
}

# Check dependencies
check_dependencies() {
    log_info "Checking dependencies..."
    
    if ! command -v gperf >/dev/null 2>&1; then
        log_error "gperf not found. Install with:"
        log_error "  macOS: brew install gperf"
        log_error "  Ubuntu/Debian: sudo apt-get install gperf"
        log_error "  CentOS/RHEL: sudo yum install gperf"
        exit 1
    fi
    
    local gperf_version=$(gperf --version | head -1)
    log_debug "Found gperf: $gperf_version"
    
    if [[ ! -f "$LEXER_SOURCE" ]]; then
        log_error "Lexer source file not found: $LEXER_SOURCE"
        exit 1
    fi
    
    if [[ ! -f "$LEXER_HEADER" ]]; then
        log_error "Lexer header file not found: $LEXER_HEADER"
        exit 1
    fi
    
    # Check for required tools
    for tool in grep sed sort wc sha256sum; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            log_error "Required tool not found: $tool"
            exit 1
        fi
    done
    
    log_success "All dependencies found"
}

# Calculate checksums for change detection
calculate_checksums() {
    local checksum_data=""
    
    # Calculate checksums of source files
    if [[ -f "$LEXER_SOURCE" ]]; then
        checksum_data+="lexer_source:$(sha256sum "$LEXER_SOURCE" | cut -d' ' -f1)\n"
    fi
    
    if [[ -f "$LEXER_HEADER" ]]; then
        checksum_data+="lexer_header:$(sha256sum "$LEXER_HEADER" | cut -d' ' -f1)\n"
    fi
    
    # Calculate checksum of this script itself
    checksum_data+="extract_script:$(sha256sum "$0" | cut -d' ' -f1)\n"
    
    echo -e "$checksum_data"
}

# Check if regeneration is needed
needs_regeneration() {
    # If output files don't exist, we need regeneration
    if [[ ! -f "$GPERF_OUTPUT_C" ]] || [[ ! -f "$GPERF_OUTPUT_H" ]]; then
        log_debug "Output files missing, regeneration needed"
        return 0
    fi
    
    # If checksum file doesn't exist, we need regeneration
    if [[ ! -f "$CHECKSUM_FILE" ]]; then
        log_debug "Checksum file missing, regeneration needed"
        return 0
    fi
    
    # Calculate current checksums
    local current_checksums
    current_checksums=$(calculate_checksums)
    
    # Compare with stored checksums
    if [[ -f "$CHECKSUM_FILE" ]]; then
        local stored_checksums
        stored_checksums=$(cat "$CHECKSUM_FILE")
        
        if [[ "$current_checksums" == "$stored_checksums" ]]; then
            log_debug "Checksums match, no regeneration needed"
            return 1
        else
            log_debug "Checksums differ, regeneration needed"
            log_debug "Current checksums:\n$current_checksums"
            log_debug "Stored checksums:\n$stored_checksums"
            return 0
        fi
    fi
    
    return 0
}

# Extract keywords from lexer source with category detection
extract_keywords() {
    log_info "Extracting keywords from lexer source..."
    
    # Create temporary file for extracted keywords
    local temp_keywords=$(mktemp)
    local temp_categories=$(mktemp)
    
    # Extract keywords from the KeywordEntry array in lexer.c
    # Look for lines like: {"keyword", TOKEN_TYPE},
    # Also capture comments that might indicate categories
    
    local in_keywords_array=false
    local current_category="general"
    
    while IFS= read -r line; do
        # Detect start of keywords array
        if [[ "$line" =~ ^[[:space:]]*static[[:space:]]+const[[:space:]]+KeywordEntry[[:space:]]+keywords\[\] ]]; then
            in_keywords_array=true
            log_debug "Found keywords array start"
            continue
        fi
        
        # Detect end of keywords array
        if [[ "$in_keywords_array" == true ]] && [[ "$line" =~ ^[[:space:]]*\}\; ]]; then
            in_keywords_array=false
            log_debug "Found keywords array end"
            break
        fi
        
        # Extract category from comments
        if [[ "$in_keywords_array" == true ]] && [[ "$line" =~ ^[[:space:]]*//[[:space:]]*(.+) ]]; then
            current_category=$(echo "${BASH_REMATCH[1]}" | tr '[:upper:]' '[:lower:]' | sed 's/[^a-z0-9]/_/g')
            log_debug "Found category: $current_category"
            continue
        fi
        
        # Extract keyword entries
        if [[ "$in_keywords_array" == true ]] && [[ "$line" =~ ^[[:space:]]*\{\"([^\"]+)\",[[:space:]]*TOKEN_([^}]+)\}, ]]; then
            local keyword="${BASH_REMATCH[1]}"
            local token_type="${BASH_REMATCH[2]}"
            
            echo "$keyword $token_type" >> "$temp_keywords"
            echo "$keyword $current_category" >> "$temp_categories"
            
            log_debug "Extracted keyword: $keyword -> TOKEN_$token_type (category: $current_category)"
        fi
    done < "$LEXER_SOURCE"
    
    # Also extract from enum definitions in header file for additional validation
    if [[ -f "$LEXER_HEADER" ]]; then
        log_debug "Extracting token types from header for validation..."
        grep -E '^[[:space:]]*TOKEN_[A-Z_]+,' "$LEXER_HEADER" | \
            sed 's/^[[:space:]]*TOKEN_\([A-Z_]*\),.*$/\1/' | \
            sort > "${temp_keywords}.tokens"
    fi
    
    if [[ ! -s "$temp_keywords" ]]; then
        log_error "No keywords extracted from lexer source"
        log_error "Expected to find KeywordEntry array in $LEXER_SOURCE"
        rm -f "$temp_keywords" "$temp_categories" "${temp_keywords}.tokens"
        exit 1
    fi
    
    # Sort keywords for consistent output
    sort "$temp_keywords" > "${temp_keywords}.sorted"
    mv "${temp_keywords}.sorted" "$temp_keywords"
    
    local keyword_count=$(wc -l < "$temp_keywords")
    log_success "Extracted $keyword_count keywords"
    
    # Show category breakdown
    if [[ -s "$temp_categories" ]]; then
        log_info "Keyword categories found:"
        cut -d' ' -f2 "$temp_categories" | sort | uniq -c | while read -r count category; do
            log_info "  $category: $count keywords"
        done
    fi
    
    # Debug: show first few keywords
    if [[ "${GPERF_DEBUG:-}" == "1" ]]; then
        log_debug "First 10 extracted keywords:"
        head -10 "$temp_keywords" | while read -r line; do
            log_debug "  $line"
        done
    fi
    
    # Clean up temporary files
    rm -f "$temp_categories" "${temp_keywords}.tokens"
    
    echo "$temp_keywords"
}

# Validate extracted keywords against token definitions
validate_keywords() {
    local keywords_file="$1"
    
    log_info "Validating extracted keywords..."
    
    local validation_errors=0
    local temp_validation=$(mktemp)
    
    # Check each keyword has a corresponding token definition
    while IFS=' ' read -r keyword token_type; do
        if [[ -n "$keyword" && -n "$token_type" ]]; then
            # Check if token type exists in header
            if ! grep -q "TOKEN_${token_type}," "$LEXER_HEADER"; then
                echo "ERROR: Token type TOKEN_${token_type} not found in header for keyword '$keyword'" >> "$temp_validation"
                ((validation_errors++))
            fi
            
            # Check for suspicious characters in keyword
            if [[ "$keyword" =~ [^a-zA-Z0-9_] ]]; then
                echo "WARNING: Keyword '$keyword' contains non-alphanumeric characters" >> "$temp_validation"
            fi
            
            # Check for reasonable keyword length
            if [[ ${#keyword} -gt 20 ]]; then
                echo "WARNING: Keyword '$keyword' is unusually long (${#keyword} characters)" >> "$temp_validation"
            fi
        fi
    done < "$keywords_file"
    
    # Check for duplicate keywords
    local duplicates
    duplicates=$(cut -d' ' -f1 "$keywords_file" | sort | uniq -d)
    if [[ -n "$duplicates" ]]; then
        echo "ERROR: Duplicate keywords found:" >> "$temp_validation"
        echo "$duplicates" | while read -r dup; do
            echo "  $dup" >> "$temp_validation"
        done
        ((validation_errors++))
    fi
    
    # Report validation results
    if [[ -s "$temp_validation" ]]; then
        log_warning "Validation issues found:"
        cat "$temp_validation" >&2
    fi
    
    if [[ $validation_errors -gt 0 ]]; then
        log_error "Validation failed with $validation_errors errors"
        rm -f "$temp_validation"
        exit 1
    else
        log_success "Keyword validation passed"
    fi
    
    rm -f "$temp_validation"
}

# Generate Gperf input file with proper structure and metadata
generate_gperf_input() {
    local keywords_file="$1"
    
    log_info "Generating Gperf input file..."
    
    # Verify the keywords file exists and is readable
    if [[ ! -f "$keywords_file" ]]; then
        log_error "Keywords file not found: $keywords_file"
        exit 1
    fi
    
    if [[ ! -s "$keywords_file" ]]; then
        log_error "Keywords file is empty: $keywords_file"
        exit 1
    fi
    
    local keyword_count=$(wc -l < "$keywords_file")
    local generation_time=$(date -u '+%Y-%m-%d %H:%M:%S UTC')
    
    cat > "$GPERF_INPUT" << EOF
%{
/* Asthra Programming Language - Perfect Hash Keyword Lookup
 * Generated automatically by extract-keywords.sh
 * DO NOT EDIT MANUALLY - This file is auto-generated
 * 
 * Generation time: $generation_time
 * Total keywords: $keyword_count
 * Source files:
 *   - $LEXER_SOURCE
 *   - $LEXER_HEADER
 */

#include <string.h>
#include <stddef.h>

/* Include lexer header for token type definitions */
#include "../src/parser/lexer.h"

%}

struct keyword_entry {
    const char *name;
    int token_type;
};

%%
EOF

    # Add keywords to the input file with correct Gperf format
    local added_count=0
    while IFS=' ' read -r keyword token_type; do
        # Skip empty lines
        if [[ -n "$keyword" && -n "$token_type" ]]; then
            # Use correct Gperf format: keyword, TOKEN_TYPE
            echo "${keyword}, TOKEN_${token_type}" >> "$GPERF_INPUT"
            ((added_count++))
        fi
    done < "$keywords_file"
    
    cat >> "$GPERF_INPUT" << 'EOF'
%%

/* Additional C code can go here if needed */
EOF

    log_success "Generated Gperf input file with $added_count keywords: $GPERF_INPUT"
    
    # Debug: show the generated file
    if [[ "${GPERF_DEBUG:-}" == "1" ]]; then
        log_debug "Generated Gperf input file content:"
        cat "$GPERF_INPUT"
    fi
}

# Generate optimized C and H files using Gperf with enhanced options
generate_hash_files() {
    log_info "Generating optimized hash files with Gperf..."
    
    # Generate C file with optimized settings
    local gperf_cmd=(
        gperf
        --output-file="$GPERF_OUTPUT_C"
        --language=C
        --struct-type
        --readonly-tables
        --enum
        --includes
        --global-table
        --hash-function-name=asthra_keyword_hash
        --lookup-function-name=asthra_keyword_lookup
        --word-array-name=asthra_keyword_wordlist
        --length-table-name=asthra_keyword_lengthtable
        --switch=1
        --compare-strncmp
        "$GPERF_INPUT"
    )
    
    log_debug "Running gperf command: ${gperf_cmd[*]}"
    
    if ! "${gperf_cmd[@]}"; then
        log_error "Gperf failed to generate C file"
        exit 1
    fi
    
    # Generate enhanced header file with better documentation
    local generation_time=$(date -u '+%Y-%m-%d %H:%M:%S UTC')
    local keyword_count=$(grep -c '^[^%#].*,' "$GPERF_INPUT" || echo "0")
    
    cat > "$GPERF_OUTPUT_H" << EOF
/**
 * Asthra Programming Language - Perfect Hash Keyword Lookup Header
 * Generated automatically by extract-keywords.sh
 * DO NOT EDIT MANUALLY - This file is auto-generated
 * 
 * Generation time: $generation_time
 * Total keywords: $keyword_count
 * 
 * This header provides a perfect hash function for O(1) keyword lookup
 * in the Asthra lexer, replacing the original linear search implementation.
 * 
 * Usage:
 *   #include "asthra_keywords_hash.h"
 *   
 *   // Check if string is a keyword and get token type
 *   int token_type = asthra_keyword_get_token_type("if", 2);
 *   if (token_type != -1) {
 *       // It's a keyword with token type 'token_type'
 *   }
 */

#ifndef ASTHRA_KEYWORDS_HASH_H
#define ASTHRA_KEYWORDS_HASH_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration - actual struct is in the generated C file */
struct keyword_entry;

/**
 * Perfect hash function for Asthra keywords
 * 
 * @param str The string to hash
 * @param len The length of the string
 * @return Hash value for the string
 */
unsigned int asthra_keyword_hash(const char *str, size_t len);

/**
 * Keyword lookup function using perfect hash
 * 
 * @param str The string to look up
 * @param len The length of the string
 * @return Pointer to keyword entry if found, NULL otherwise
 */
const struct keyword_entry *asthra_keyword_lookup(const char *str, size_t len);

/**
 * Convenience function that returns token type or -1 if not found
 * 
 * @param str The string to look up
 * @param len The length of the string
 * @return Token type if keyword found, -1 otherwise
 */
int asthra_keyword_get_token_type(const char *str, size_t len);

/**
 * Validate that a string is a keyword
 * 
 * @param str The string to check
 * @param len The length of the string
 * @return 1 if it's a keyword, 0 otherwise
 */
int asthra_keyword_is_keyword(const char *str, size_t len);

/**
 * Get the total number of keywords in the hash table
 * 
 * @return Total number of keywords
 */
int asthra_keyword_get_count(void);

#ifdef __cplusplus
}
#endif

#endif /* ASTHRA_KEYWORDS_HASH_H */
EOF

    # Add enhanced convenience functions to the C file
    cat >> "$GPERF_OUTPUT_C" << EOF

/* Enhanced convenience function implementations */

int asthra_keyword_get_token_type(const char *str, size_t len) {
    const struct keyword_entry *entry = asthra_keyword_lookup(str, len);
    return entry ? entry->token_type : -1;
}

int asthra_keyword_is_keyword(const char *str, size_t len) {
    return asthra_keyword_lookup(str, len) != NULL;
}

int asthra_keyword_get_count(void) {
    return TOTAL_KEYWORDS;
}

/* Integration helper functions for lexer */

/**
 * Drop-in replacement for the original keyword_lookup function
 * This function maintains the same signature as the original lexer function
 */
int asthra_keyword_lookup_compatible(const char *identifier, size_t length) {
    int token_type = asthra_keyword_get_token_type(identifier, length);
    return (token_type != -1) ? token_type : 0; /* TOKEN_IDENTIFIER equivalent */
}

/**
 * Drop-in replacement for the original is_reserved_keyword function
 */
int asthra_is_reserved_keyword_compatible(const char *identifier, size_t length) {
    return asthra_keyword_is_keyword(identifier, length);
}
EOF

    log_success "Generated enhanced hash files:"
    log_success "  C file: $GPERF_OUTPUT_C ($(wc -c < "$GPERF_OUTPUT_C") bytes)"
    log_success "  H file: $GPERF_OUTPUT_H ($(wc -c < "$GPERF_OUTPUT_H") bytes)"
}

# Enhanced validation with compilation test and performance analysis
validate_hash_function() {
    log_info "Validating generated hash function..."
    
    # Create a comprehensive test program
    local test_program=$(mktemp /tmp/test_keywords_XXXXXX.c)
    local test_binary=$(mktemp /tmp/test_keywords_XXXXXX)
    
    cat > "$test_program" << EOF
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Include the generated files
#include "$GPERF_OUTPUT_H"

// Test keywords (should be found)
static const char *test_keywords[] = {
    "if", "else", "for", "while", "fn", "struct", "let", "return",
    "package", "import", "match", "spawn", "unsafe", "true", "false"
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

    # Try to compile and run the test program
    local compile_success=false
    if gcc -std=c17 -O2 -I"$OUTPUT_DIR" -I"$PROJECT_ROOT/src/parser" \
           "$test_program" "$GPERF_OUTPUT_C" -o "$test_binary" 2>/dev/null; then
        compile_success=true
        log_success "Test program compiled successfully"
        
        # Run the test
        if "$test_binary"; then
            log_success "Hash function validation passed"
        else
            log_warning "Hash function validation failed"
        fi
    else
        log_warning "Test program compilation failed"
    fi
    
    # Generate comprehensive validation report
    {
        echo "=== Asthra Keyword Hash Generation Report ==="
        echo "Generated at: $(date)"
        echo "Script version: $(grep '^# Enhanced version' "$0" | head -1)"
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
        echo "Total keywords: $(grep -c '^[^%#].*,' "$GPERF_INPUT" || echo "0")"
        echo "C file size: $(wc -c < "$GPERF_OUTPUT_C") bytes"
        echo "H file size: $(wc -c < "$GPERF_OUTPUT_H") bytes"
        echo "Gperf input size: $(wc -c < "$GPERF_INPUT") bytes"
        echo ""
        echo "=== Hash Function Properties ==="
        grep -E "MIN_WORD_LENGTH|MAX_WORD_LENGTH|MIN_HASH_VALUE|MAX_HASH_VALUE|TOTAL_KEYWORDS" "$GPERF_OUTPUT_C" || echo "Properties not found"
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
        grep -E '^(unsigned int|const struct|int) asthra_keyword_' "$GPERF_OUTPUT_C" || echo "No functions found"
        echo ""
        echo "--- H file declarations ---"
        grep -E '^(unsigned int|const struct|int) asthra_keyword_' "$GPERF_OUTPUT_H" || echo "No declarations found"
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
    } > "$VALIDATION_LOG"
    
    # Clean up
    rm -f "$test_program" "$test_binary"
    
    log_success "Comprehensive validation complete. Report saved to: $VALIDATION_LOG"
}

# Save checksums for change detection
save_checksums() {
    log_debug "Saving checksums for change detection..."
    calculate_checksums > "$CHECKSUM_FILE"
    date -u '+%Y-%m-%d %H:%M:%S UTC' > "$TIMESTAMP_FILE"
}

# Main execution with smart change detection
main() {
    log_info "Starting Asthra keyword extraction and Gperf generation..."
    
    # Check if we need to regenerate
    if ! needs_regeneration; then
        log_success "Generated files are up-to-date, no regeneration needed"
        log_info "Use FORCE_REGENERATION=1 to force regeneration"
        
        if [[ "${FORCE_REGENERATION:-}" != "1" ]]; then
            exit 0
        else
            log_info "Forcing regeneration due to FORCE_REGENERATION=1"
        fi
    fi
    
    check_dependencies
    
    # Extract keywords
    local keywords_file
    keywords_file=$(extract_keywords)
    
    # Validate extracted keywords
    validate_keywords "$keywords_file"
    
    # Generate Gperf input
    generate_gperf_input "$keywords_file"
    
    # Generate hash files
    generate_hash_files
    
    # Validate results
    validate_hash_function
    
    # Save checksums for future change detection
    save_checksums
    
    # Clean up temporary files
    rm -f "$keywords_file"
    
    log_success "Keyword extraction and hash generation complete!"
    log_info "Generated files:"
    log_info "  Input: $GPERF_INPUT"
    log_info "  C file: $GPERF_OUTPUT_C"
    log_info "  H file: $GPERF_OUTPUT_H"
    log_info "  Report: $VALIDATION_LOG"
    log_info ""
    log_info "Next steps:"
    log_info "  1. Run 'make gperf-test' to test the generated hash function"
    log_info "  2. Run 'make gperf-benchmark' to benchmark performance"
    log_info "  3. Run 'make gperf-integrate' for integration instructions"
}

# Handle command line arguments
case "${1:-}" in
    --force)
        export FORCE_REGENERATION=1
        main
        ;;
    --debug)
        export GPERF_DEBUG=1
        main
        ;;
    --check)
        if needs_regeneration; then
            echo "Regeneration needed"
            exit 1
        else
            echo "Up-to-date"
            exit 0
        fi
        ;;
    --help|-h)
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  --force    Force regeneration even if files are up-to-date"
        echo "  --debug    Enable debug output"
        echo "  --check    Check if regeneration is needed (exit 1 if needed)"
        echo "  --help     Show this help message"
        echo ""
        echo "Environment variables:"
        echo "  GPERF_DEBUG=1         Enable debug output"
        echo "  FORCE_REGENERATION=1  Force regeneration"
        exit 0
        ;;
    "")
        main
        ;;
    *)
        log_error "Unknown option: $1"
        log_error "Use --help for usage information"
        exit 1
        ;;
esac 
