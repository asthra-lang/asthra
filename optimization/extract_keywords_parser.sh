#!/bin/bash

# Asthra Keyword Extraction - Parser Module
# Copyright (c) 2024 Asthra Project
# 
# Handles extraction of keywords from lexer source code with
# category detection and comprehensive validation.

set -euo pipefail

# Source common utilities
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/extract_keywords_common.sh"

# Extract keywords from lexer source with category detection
extract_keywords() {
    log_info "Extracting keywords from lexer source..."
    
    # Create temporary file for extracted keywords
    local temp_keywords=$(create_temp_file "keywords")
    local temp_categories=$(create_temp_file "categories")
    
    # Extract keywords from the KeywordEntry array in lexer.c
    # Look for lines like: {"keyword", TOKEN_TYPE},
    # Also capture comments that might indicate categories
    
    local in_keywords_array=false
    local current_category="general"
    
    while IFS= read -r line; do
        # Detect start of keywords array
        if [[ "$line" =~ ^[[:space:]]*static[[:space:]]+const[[:space:]]+KeywordEntry[[:space:]]+keywords\[\][[:space:]]*= ]] || [[ "$line" =~ ^[[:space:]]*KeywordEntry[[:space:]]+keywords\[\][[:space:]]*= ]]; then
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
        if [[ "$in_keywords_array" == true ]] && [[ "$line" =~ ^[[:space:]]*\{\"([^\"]+)\",[[:space:]]*TOKEN_([^}]+)\}[,]? ]]; then
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
    
    local keyword_count=$(count_lines "$temp_keywords")
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
    local temp_validation=$(create_temp_file "validation")
    
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

# Enhanced keyword analysis and statistics
analyze_keywords() {
    local keywords_file="$1"
    
    log_info "Analyzing keyword patterns..."
    
    local total_keywords=$(count_lines "$keywords_file")
    local avg_length=0
    local min_length=999
    local max_length=0
    local total_chars=0
    
    # Calculate statistics
    while IFS=' ' read -r keyword token_type; do
        if [[ -n "$keyword" ]]; then
            local len=${#keyword}
            total_chars=$((total_chars + len))
            
            if [[ $len -lt $min_length ]]; then
                min_length=$len
            fi
            
            if [[ $len -gt $max_length ]]; then
                max_length=$len
            fi
        fi
    done < "$keywords_file"
    
    if [[ $total_keywords -gt 0 ]]; then
        avg_length=$((total_chars / total_keywords))
    fi
    
    log_info "Keyword statistics:"
    log_info "  Total keywords: $total_keywords"
    log_info "  Average length: $avg_length characters"
    log_info "  Length range: $min_length - $max_length characters"
    log_info "  Total characters: $total_chars"
    
    # Analyze keyword patterns
    log_info "Keyword patterns:"
    
    # Count by first letter
    log_info "  By first letter:"
    cut -d' ' -f1 "$keywords_file" | cut -c1 | sort | uniq -c | sort -nr | head -5 | while read -r count letter; do
        log_info "    $letter: $count keywords"
    done
    
    # Count by length
    log_info "  By length:"
    while IFS=' ' read -r keyword token_type; do
        if [[ -n "$keyword" ]]; then
            echo ${#keyword}
        fi
    done < "$keywords_file" | sort -n | uniq -c | while read -r count length; do
        log_info "    $length chars: $count keywords"
    done
}

# Export functions for use in other scripts
export -f extract_keywords validate_keywords analyze_keywords 
