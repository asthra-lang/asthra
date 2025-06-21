#!/bin/bash

# Asthra Keyword Extraction - Common Utilities
# Copyright (c) 2024 Asthra Project
# 
# Shared configuration, logging, and utility functions for the
# keyword extraction and Gperf generation system.

set -euo pipefail

# Configuration - can be overridden by sourcing scripts
SCRIPT_DIR="${SCRIPT_DIR:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)}"
PROJECT_ROOT="${PROJECT_ROOT:-$(dirname "$SCRIPT_DIR")}"
LEXER_SOURCE="${LEXER_SOURCE:-$PROJECT_ROOT/src/parser/keyword.c}"
LEXER_HEADER="${LEXER_HEADER:-$PROJECT_ROOT/src/parser/lexer.h}"
OUTPUT_DIR="${OUTPUT_DIR:-$SCRIPT_DIR}"
GPERF_INPUT="${GPERF_INPUT:-$OUTPUT_DIR/asthra_keywords.gperf}"
GPERF_OUTPUT_C="${GPERF_OUTPUT_C:-$OUTPUT_DIR/asthra_keywords_hash.c}"
GPERF_OUTPUT_H="${GPERF_OUTPUT_H:-$OUTPUT_DIR/asthra_keywords_hash.h}"
VALIDATION_LOG="${VALIDATION_LOG:-$OUTPUT_DIR/keyword_validation.log}"
CHECKSUM_FILE="${CHECKSUM_FILE:-$OUTPUT_DIR/.keyword_checksums}"
TIMESTAMP_FILE="${TIMESTAMP_FILE:-$OUTPUT_DIR/.last_generation}"

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

# Save checksums for change detection
save_checksums() {
    log_debug "Saving checksums for change detection..."
    calculate_checksums > "$CHECKSUM_FILE"
    date -u '+%Y-%m-%d %H:%M:%S UTC' > "$TIMESTAMP_FILE"
}

# Utility function to create temporary files with proper cleanup
create_temp_file() {
    local prefix="${1:-keyword_extract}"
    local suffix="${2:-}"
    if [[ -n "$suffix" ]]; then
        local temp_file=$(mktemp "/tmp/${prefix}_XXXXXX${suffix}")
    else
        local temp_file=$(mktemp "/tmp/${prefix}_XXXXXX")
    fi
    echo "$temp_file"
}

# Utility function to get file size in human readable format
get_file_size() {
    local file="$1"
    if [[ -f "$file" ]]; then
        wc -c < "$file"
    else
        echo "0"
    fi
}

# Utility function to count lines in file
count_lines() {
    local file="$1"
    if [[ -f "$file" && -s "$file" ]]; then
        wc -l < "$file"
    else
        echo "0"
    fi
}

# Export functions for use in other scripts
export -f log_info log_success log_warning log_error log_debug
export -f check_dependencies calculate_checksums needs_regeneration save_checksums
export -f create_temp_file get_file_size count_lines 
