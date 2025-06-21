#!/bin/bash

# Asthra Keyword Extraction - Gperf Generation Module
# Copyright (c) 2024 Asthra Project
# 
# Handles generation of Gperf input files and creation of optimized
# C and H files for perfect hash keyword lookup.

set -euo pipefail

# Source common utilities
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/extract_keywords_common.sh"

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
    
    local keyword_count=$(count_lines "$keywords_file")
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
    generate_enhanced_header
    
    # Add enhanced convenience functions to the C file
    add_convenience_functions
    
    log_success "Generated enhanced hash files:"
    log_success "  C file: $GPERF_OUTPUT_C ($(get_file_size "$GPERF_OUTPUT_C") bytes)"
    log_success "  H file: $GPERF_OUTPUT_H ($(get_file_size "$GPERF_OUTPUT_H") bytes)"
}

# Generate enhanced header file with comprehensive documentation
generate_enhanced_header() {
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
}

# Add enhanced convenience functions to the C file
add_convenience_functions() {
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
}

# Analyze Gperf output for optimization metrics
analyze_gperf_output() {
    log_info "Analyzing Gperf optimization metrics..."
    
    if [[ ! -f "$GPERF_OUTPUT_C" ]]; then
        log_warning "Gperf output file not found for analysis"
        return
    fi
    
    # Extract key metrics from the generated C file
    local min_word_length=$(grep -o 'MIN_WORD_LENGTH = [0-9]*' "$GPERF_OUTPUT_C" | cut -d' ' -f3 || echo "unknown")
    local max_word_length=$(grep -o 'MAX_WORD_LENGTH = [0-9]*' "$GPERF_OUTPUT_C" | cut -d' ' -f3 || echo "unknown")
    local min_hash_value=$(grep -o 'MIN_HASH_VALUE = [0-9]*' "$GPERF_OUTPUT_C" | cut -d' ' -f3 || echo "unknown")
    local max_hash_value=$(grep -o 'MAX_HASH_VALUE = [0-9]*' "$GPERF_OUTPUT_C" | cut -d' ' -f3 || echo "unknown")
    local total_keywords=$(grep -o 'TOTAL_KEYWORDS = [0-9]*' "$GPERF_OUTPUT_C" | cut -d' ' -f3 || echo "unknown")
    
    log_info "Gperf optimization metrics:"
    log_info "  Word length range: $min_word_length - $max_word_length"
    log_info "  Hash value range: $min_hash_value - $max_hash_value"
    log_info "  Total keywords: $total_keywords"
    
    # Calculate hash table efficiency
    if [[ "$min_hash_value" != "unknown" && "$max_hash_value" != "unknown" && "$total_keywords" != "unknown" ]]; then
        local hash_range=$((max_hash_value - min_hash_value + 1))
        local efficiency=$((total_keywords * 100 / hash_range))
        log_info "  Hash table efficiency: $efficiency% ($total_keywords/$hash_range slots used)"
    fi
    
    # Check for collisions (perfect hash should have none)
    if grep -q "collision" "$GPERF_OUTPUT_C"; then
        log_warning "Potential hash collisions detected in generated code"
    else
        log_success "Perfect hash generated with no collisions"
    fi
}

# Export functions for use in other scripts
export -f generate_gperf_input generate_hash_files generate_enhanced_header
export -f add_convenience_functions analyze_gperf_output 
