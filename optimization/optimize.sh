#!/bin/bash

# Modern Optimization Script for Asthra Compiler
# Uses Clang-native tools for performance analysis and optimization

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"
OPTIMIZATION_DIR="$PROJECT_ROOT/optimization"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Utility functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check for required modern tools
check_dependencies() {
    local missing_tools=()
    
    # Check Clang
    if ! command_exists clang; then
        missing_tools+=("clang")
    fi
    
    # Check LLVM tools for PGO
    if ! command_exists llvm-profdata; then
        print_warning "llvm-profdata not found - PGO may not work properly"
    fi
    
    # Check for Instruments on macOS
    if [[ "$OSTYPE" == "darwin"* ]] && ! command_exists instruments; then
        print_warning "Instruments not found - install Xcode command line tools for profiling"
    fi
    
    if [ ${#missing_tools[@]} -ne 0 ]; then
        print_error "Missing required tools:"
        for tool in "${missing_tools[@]}"; do
            case $tool in
                clang)
                    echo "  - clang: Install Xcode command line tools (macOS) or clang package (Linux)"
                    ;;
            esac
        done
        return 1
    fi
    
    print_success "All required modern optimization tools are available"
    return 0
}

# Modern profiling using PGO
run_profiling() {
    print_status "Running Profile-Guided Optimization (PGO)..."
    
    cd "$PROJECT_ROOT"
    
    # Run PGO workflow
    if make pgo-optimize; then
        print_success "PGO optimization completed successfully"
        
        # Show results
        if [ -f "$BUILD_DIR/asthra_pgo_optimized" ]; then
            print_status "Optimized binary created: $BUILD_DIR/asthra_pgo_optimized"
            
            # Suggest Instruments usage on macOS
            if [[ "$OSTYPE" == "darwin"* ]] && command_exists instruments; then
                print_status "For detailed profiling, run:"
                echo "  instruments -t 'Time Profiler' $BUILD_DIR/asthra_pgo_optimized [args]"
            fi
        fi
    else
        print_error "PGO optimization failed"
        return 1
    fi
}

# Modern memory analysis using Clang Sanitizers
run_memory_analysis() {
    print_status "Running memory analysis with Clang Sanitizers..."
    
    cd "$PROJECT_ROOT"
    
    # Run comprehensive sanitizer analysis
    if make sanitizer-comprehensive; then
        print_success "Sanitizer analysis completed successfully"
        
        # Show available sanitized binaries
        print_status "Available sanitized binaries:"
        [ -f "$BUILD_DIR/asthra_asan" ] && echo "  - AddressSanitizer: $BUILD_DIR/asthra_asan"
        [ -f "$BUILD_DIR/asthra_tsan" ] && echo "  - ThreadSanitizer: $BUILD_DIR/asthra_tsan"
        [ -f "$BUILD_DIR/asthra_ubsan" ] && echo "  - UBSan: $BUILD_DIR/asthra_ubsan"
        [ -f "$BUILD_DIR/asthra_asan_ubsan" ] && echo "  - Combined ASan+UBSan: $BUILD_DIR/asthra_asan_ubsan"
        
        # Show logs location
        if [ -d "$OPTIMIZATION_DIR" ]; then
            print_status "Analysis logs saved in: $OPTIMIZATION_DIR/"
        fi
    else
        print_error "Sanitizer analysis failed"
        return 1
    fi
}

# Enhanced Gperf optimization
run_gperf_optimization() {
    print_status "Running enhanced Gperf optimization..."
    
    cd "$PROJECT_ROOT"
    
    if make gperf-enhanced; then
        print_success "Gperf optimization completed successfully"
    else
        print_error "Gperf optimization failed"
        return 1
    fi
}

# Show help
show_help() {
    cat << EOF
Modern Optimization Script for Asthra Compiler

USAGE:
    $0 [COMMAND]

COMMANDS:
    profile     Run Profile-Guided Optimization (PGO)
    memory      Run memory analysis with Clang Sanitizers
    gperf       Run enhanced Gperf optimization
    all         Run all optimization analyses
    check       Check for required tools
    help        Show this help message

EXAMPLES:
    $0 profile              # Run PGO optimization
    $0 memory               # Run sanitizer analysis
    $0 all                  # Run complete optimization workflow

MODERN TOOLS USED:
    - Clang PGO for performance optimization
    - Clang Sanitizers for memory/thread analysis
    - Enhanced Gperf for hash optimization
    - Instruments (macOS) for detailed profiling

For more information, see: optimization/README.md
EOF
}

# Main execution
main() {
    case "${1:-help}" in
        profile)
            check_dependencies && run_profiling
            ;;
        memory)
            check_dependencies && run_memory_analysis
            ;;
        gperf)
            check_dependencies && run_gperf_optimization
            ;;
        all)
            check_dependencies || exit 1
            print_status "Running complete modern optimization workflow..."
            run_profiling
            run_memory_analysis
            run_gperf_optimization
            print_success "Complete optimization workflow finished"
            ;;
        check)
            check_dependencies
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "Unknown command: $1"
            echo
            show_help
            exit 1
            ;;
    esac
}

main "$@" 
