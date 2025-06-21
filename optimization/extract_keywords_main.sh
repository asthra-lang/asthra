#!/bin/bash

# Asthra Keyword Extraction - Main Orchestrator
# Copyright (c) 2024 Asthra Project
# 
# Main script that coordinates all keyword extraction modules
# and provides the command-line interface for the complete
# keyword extraction and Gperf generation system.

set -euo pipefail

# Source all modules
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/extract_keywords_common.sh"
source "$SCRIPT_DIR/extract_keywords_parser.sh"
source "$SCRIPT_DIR/extract_keywords_gperf.sh"
source "$SCRIPT_DIR/extract_keywords_validation.sh"

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
    
    # Analyze keyword patterns (optional)
    if [[ "${ANALYZE_KEYWORDS:-}" == "1" ]]; then
        analyze_keywords "$keywords_file"
    fi
    
    # Generate Gperf input
    generate_gperf_input "$keywords_file"
    
    # Generate hash files
    generate_hash_files
    
    # Analyze Gperf output (optional)
    if [[ "${ANALYZE_GPERF:-}" == "1" ]]; then
        analyze_gperf_output
    fi
    
    # Validate results
    validate_hash_function
    
    # Run benchmark (optional)
    if [[ "${BENCHMARK:-}" == "1" ]]; then
        benchmark_hash_function
    fi
    
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

# Show usage information
show_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Asthra Keyword Extraction and Gperf Generation System

This script automatically extracts keywords from the lexer source code,
generates optimized Gperf input files, and creates both .c and .h files
for perfect hash keyword lookup.

OPTIONS:
  --force         Force regeneration even if files are up-to-date
  --debug         Enable debug output
  --check         Check if regeneration is needed (exit 1 if needed)
  --analyze       Enable keyword pattern analysis
  --benchmark     Run performance benchmark after generation
  --gperf-only    Only run Gperf generation (skip extraction)
  --validate-only Only run validation (skip generation)
  --help, -h      Show this help message

ENVIRONMENT VARIABLES:
  GPERF_DEBUG=1         Enable debug output
  FORCE_REGENERATION=1  Force regeneration
  ANALYZE_KEYWORDS=1    Enable keyword analysis
  ANALYZE_GPERF=1       Enable Gperf output analysis
  BENCHMARK=1           Run performance benchmark

EXAMPLES:
  $0                    # Normal operation with change detection
  $0 --force --debug    # Force regeneration with debug output
  $0 --check            # Check if regeneration is needed
  $0 --benchmark        # Generate and benchmark performance
  
  # Environment variable examples:
  GPERF_DEBUG=1 $0                    # Enable debug output
  FORCE_REGENERATION=1 $0             # Force regeneration
  ANALYZE_KEYWORDS=1 BENCHMARK=1 $0   # Full analysis and benchmark

FILES GENERATED:
  asthra_keywords.gperf       - Gperf input file
  asthra_keywords_hash.c      - Generated C implementation
  asthra_keywords_hash.h      - Generated header file
  keyword_validation.log      - Comprehensive validation report

INTEGRATION:
  Include asthra_keywords_hash.h in your lexer and link with
  asthra_keywords_hash.c for O(1) keyword lookup performance.

EOF
}

# Show version information
show_version() {
    echo "Asthra Keyword Extraction System - Modular Version"
    echo "Copyright (c) 2024 Asthra Project"
    echo ""
    echo "Modules:"
    echo "  extract_keywords_common.sh    - Common utilities and configuration"
    echo "  extract_keywords_parser.sh    - Keyword extraction and validation"
    echo "  extract_keywords_gperf.sh     - Gperf generation and optimization"
    echo "  extract_keywords_validation.sh - Testing and performance analysis"
    echo "  extract_keywords_main.sh      - Main orchestrator (this script)"
    echo ""
    echo "Dependencies:"
    gperf --version 2>/dev/null | head -1 || echo "  gperf: not found"
    gcc --version 2>/dev/null | head -1 || echo "  gcc: not found"
}

# Quick status check
show_status() {
    echo "=== Asthra Keyword Extraction Status ==="
    echo ""
    echo "Configuration:"
    echo "  Project root: $PROJECT_ROOT"
    echo "  Lexer source: $LEXER_SOURCE"
    echo "  Lexer header: $LEXER_HEADER"
    echo "  Output directory: $OUTPUT_DIR"
    echo ""
    echo "Generated files:"
    if [[ -f "$GPERF_INPUT" ]]; then
        echo "  ✓ Gperf input: $GPERF_INPUT ($(get_file_size "$GPERF_INPUT") bytes)"
    else
        echo "  ✗ Gperf input: not found"
    fi
    
    if [[ -f "$GPERF_OUTPUT_C" ]]; then
        echo "  ✓ C file: $GPERF_OUTPUT_C ($(get_file_size "$GPERF_OUTPUT_C") bytes)"
    else
        echo "  ✗ C file: not found"
    fi
    
    if [[ -f "$GPERF_OUTPUT_H" ]]; then
        echo "  ✓ H file: $GPERF_OUTPUT_H ($(get_file_size "$GPERF_OUTPUT_H") bytes)"
    else
        echo "  ✗ H file: not found"
    fi
    
    if [[ -f "$VALIDATION_LOG" ]]; then
        echo "  ✓ Validation log: $VALIDATION_LOG ($(get_file_size "$VALIDATION_LOG") bytes)"
    else
        echo "  ✗ Validation log: not found"
    fi
    
    echo ""
    echo "Regeneration status:"
    if needs_regeneration; then
        echo "  Status: NEEDED (source files have changed)"
    else
        echo "  Status: UP-TO-DATE (no changes detected)"
    fi
    
    if [[ -f "$TIMESTAMP_FILE" ]]; then
        echo "  Last generation: $(cat "$TIMESTAMP_FILE")"
    else
        echo "  Last generation: never"
    fi
}

# Gperf-only mode (skip extraction, use existing keywords)
gperf_only_mode() {
    log_info "Running Gperf-only mode..."
    
    # Check if we have a keywords file or need to extract
    local keywords_file="$OUTPUT_DIR/.last_keywords"
    if [[ ! -f "$keywords_file" ]]; then
        log_info "No cached keywords found, extracting..."
        keywords_file=$(extract_keywords)
        cp "$keywords_file" "$OUTPUT_DIR/.last_keywords"
    else
        log_info "Using cached keywords from previous extraction"
    fi
    
    # Generate Gperf files
    generate_gperf_input "$keywords_file"
    generate_hash_files
    
    # Validate
    validate_hash_function
    
    log_success "Gperf-only generation complete!"
}

# Validation-only mode (skip generation, validate existing files)
validate_only_mode() {
    log_info "Running validation-only mode..."
    
    if [[ ! -f "$GPERF_OUTPUT_C" ]] || [[ ! -f "$GPERF_OUTPUT_H" ]]; then
        log_error "Generated files not found. Run generation first."
        exit 1
    fi
    
    # Run validation and benchmarks
    validate_hash_function
    
    if [[ "${BENCHMARK:-}" == "1" ]]; then
        benchmark_hash_function
    fi
    
    log_success "Validation-only mode complete!"
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
    --analyze)
        export ANALYZE_KEYWORDS=1
        export ANALYZE_GPERF=1
        main
        ;;
    --benchmark)
        export BENCHMARK=1
        main
        ;;
    --gperf-only)
        gperf_only_mode
        ;;
    --validate-only)
        validate_only_mode
        ;;
    --status)
        show_status
        ;;
    --version)
        show_version
        ;;
    --help|-h)
        show_usage
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
