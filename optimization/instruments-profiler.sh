#!/bin/bash

# Asthra Compiler Instruments Profiler Script
# Automates launching Instruments with Time Profiler for performance analysis on macOS

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OPTIMIZATION_DIR="$PROJECT_ROOT/optimization"
BUILD_DIR="$PROJECT_ROOT/build"
BIN_DIR="$PROJECT_ROOT/bin"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print colored output
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

print_instruction() {
    echo -e "${CYAN}[INSTRUCTION]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check if we're on macOS
is_macos() {
    [[ "$(uname -s)" == "Darwin" ]]
}

# Function to check if Instruments is available
check_instruments() {
    print_status "Checking Instruments availability..."
    
    if ! is_macos; then
        print_error "Instruments is only available on macOS"
        echo "For other platforms, consider using:"
        echo "  - Clang Sanitizers (make sanitizer-comprehensive)"
        echo "  - Enhanced Gperf (make gperf-keywords)"
        echo "  - Clang PGO (make pgo-optimize)"
        return 1
    fi
    
    if ! command_exists instruments; then
        print_error "Instruments not found"
        echo "Instruments is part of Xcode Command Line Tools"
        echo "Install with: xcode-select --install"
        return 1
    fi
    
    # Check if Xcode is properly configured
    if ! xcode-select -p >/dev/null 2>&1; then
        print_warning "Xcode Command Line Tools path not set"
        echo "Run: sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer"
        echo "Or: sudo xcode-select --switch /Library/Developer/CommandLineTools"
    fi
    
    print_success "Instruments is available"
    
    # Show available templates
    print_status "Available Instruments templates:"
    instruments -s templates | grep -E "(Time Profiler|Allocations|Leaks)" | head -5 || true
    
    return 0
}

# Function to build compiler with debug symbols
build_debug_compiler() {
    print_status "Building Asthra compiler with debug symbols..."
    cd "$PROJECT_ROOT"
    
    # Create debug build with symbols but without sanitizers for profiling
    local DEBUG_PROFILE_CFLAGS
    if [[ "$(uname -s)" == "Darwin" ]]; then
        DEBUG_PROFILE_CFLAGS="-std=c17 -Wall -Wextra -Wpedantic -O2 -g -DDEBUG"
    else
        DEBUG_PROFILE_CFLAGS="-std=c17 -Wall -Wextra -Wpedantic -O2 -g -DDEBUG"
    fi
    
    # Build with debug symbols but optimized for realistic profiling
    if CFLAGS="$DEBUG_PROFILE_CFLAGS" make clean && CFLAGS="$DEBUG_PROFILE_CFLAGS" make all; then
        print_success "Debug compiler built successfully"
        echo "Binary location: $BIN_DIR/asthra"
        return 0
    else
        print_error "Failed to build debug compiler"
        return 1
    fi
}

# Function to create test workload for profiling
create_test_workload() {
    local test_file="$BUILD_DIR/instruments_test.asthra"
    
    print_status "Creating test workload for profiling..."
    mkdir -p "$BUILD_DIR"
    
    # Create a more complex test file to generate meaningful profile data
    cat > "$test_file" << 'EOF'
// Instruments profiling test workload
// This file is designed to exercise various compiler components

package main

import std.io
import std.collections

struct ComplexStruct {
    id: i32,
    name: string,
    values: [i32; 100],
    nested: NestedStruct
}

struct NestedStruct {
    data: [string; 50],
    flags: [bool; 32]
}

fn fibonacci(n: i32) -> i32 {
    if n <= 1 {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

fn process_array(arr: [i32; 1000]) -> i32 {
    let mut sum = 0
    for i in 0..1000 {
        sum += arr[i] * fibonacci(i % 10)
    }
    return sum
}

fn complex_string_operations() -> string {
    let mut result = "Starting complex operations: "
    for i in 0..100 {
        result += "iteration_" + i.to_string() + "_"
        if i % 10 == 0 {
            result += "checkpoint_"
        }
    }
    return result
}

fn main() {
    println("Starting Instruments profiling test...")
    
    // Exercise lexer/parser with complex structures
    let complex = ComplexStruct {
        id: 42,
        name: "test_struct",
        values: [0; 100],
        nested: NestedStruct {
            data: [""; 50],
            flags: [false; 32]
        }
    }
    
    // Exercise arithmetic and function calls
    let fib_result = fibonacci(20)
    println("Fibonacci result: {}", fib_result)
    
    // Exercise array processing
    let test_array = [1; 1000]
    let array_result = process_array(test_array)
    println("Array processing result: {}", array_result)
    
    // Exercise string operations
    let string_result = complex_string_operations()
    println("String operations completed")
    
    // Exercise memory allocation patterns
    for iteration in 0..50 {
        let temp_struct = ComplexStruct {
            id: iteration,
            name: "temp_" + iteration.to_string(),
            values: [iteration; 100],
            nested: NestedStruct {
                data: ["temp"; 50],
                flags: [true; 32]
            }
        }
        
        if iteration % 10 == 0 {
            println("Completed iteration: {}", iteration)
        }
    }
    
    println("Instruments profiling test completed successfully!")
}
EOF
    
    print_success "Test workload created: $test_file"
    echo "This workload exercises:"
    echo "  - Complex struct parsing"
    echo "  - Recursive function calls"
    echo "  - Array processing"
    echo "  - String operations"
    echo "  - Memory allocation patterns"
}

# Function to launch Instruments with Time Profiler
launch_instruments() {
    local mode="${1:-interactive}"
    local timestamp=$(date +%Y%m%d_%H%M%S)
    local trace_file="$OPTIMIZATION_DIR/instruments_trace_$timestamp.trace"
    
    print_status "Launching Instruments with Time Profiler..."
    
    # Ensure optimization directory exists
    mkdir -p "$OPTIMIZATION_DIR"
    
    # Create test workload
    create_test_workload
    
    case "$mode" in
        interactive)
            launch_instruments_interactive "$trace_file"
            ;;
        automated)
            launch_instruments_automated "$trace_file"
            ;;
        *)
            print_error "Unknown mode: $mode"
            return 1
            ;;
    esac
}

# Function to launch Instruments interactively
launch_instruments_interactive() {
    local trace_file="$1"
    
    print_instruction "Launching Instruments in interactive mode..."
    echo ""
    echo "=== Manual Profiling Workflow ==="
    echo ""
    echo "1. Instruments will open with the Time Profiler template"
    echo "2. The target will be set to: $BIN_DIR/asthra"
    echo "3. Arguments will be set to compile the test workload"
    echo ""
    echo "Manual steps to follow:"
    echo "  a) Click the Record button (red circle) to start profiling"
    echo "  b) The compiler will run and process the test file"
    echo "  c) Click Stop when compilation completes"
    echo "  d) Analyze the Time Profiler results:"
    echo "     - Look for functions with high CPU usage"
    echo "     - Check the call tree for bottlenecks"
    echo "     - Focus on user code (not system libraries)"
    echo "  e) Save the trace file when done"
    echo ""
    echo "Trace will be saved to: $trace_file"
}

# Function to launch Instruments in automated mode
launch_instruments_automated() {
    local trace_file="$1"
    
    print_status "Running automated Instruments profiling..."
    
    # Run instruments in command-line mode
    print_status "Executing profiling run..."
    
    if instruments -t "Time Profiler" \
        -D "$trace_file" \
        -l 30000 \
        "$BIN_DIR/asthra" \
        "$BUILD_DIR/instruments_test.asthra" \
        -o "$BUILD_DIR/instruments_test_output"; then
        
        print_success "Automated profiling completed"
        echo "Trace file saved: $trace_file"
        
        # Try to extract basic information from the trace
        extract_trace_summary "$trace_file"
    else
        print_error "Automated profiling failed"
        return 1
    fi
}

# Function to extract summary from trace file
extract_trace_summary() {
    local trace_file="$1"
    local summary_file="${trace_file%.trace}_summary.txt"
    
    print_status "Extracting trace summary..."
    
    # Create a summary report
    cat > "$summary_file" << EOF
Instruments Profiling Summary
Generated: $(date)
Trace File: $trace_file
Target: $BIN_DIR/asthra
Test Workload: $BUILD_DIR/instruments_test.asthra

=== Analysis Instructions ===

To analyze this trace file:

1. Open in Instruments GUI:
   open "$trace_file"

2. Command-line analysis (if available):
   xctrace export --input "$trace_file" --xpath '/trace-toc/run[@number="1"]/data/table[@schema="time-profile"]'

3. Key areas to examine:
   - Functions with highest CPU time
   - Call tree depth and frequency
   - System vs user time distribution
   - Memory allocation patterns (if using Allocations template)

=== Optimization Recommendations ===

Based on typical compiler bottlenecks, focus on:

1. Lexer Performance:
   - Token recognition efficiency
   - String handling and memory allocation
   - Keyword lookup optimization

2. Parser Performance:
   - Recursive descent function calls
   - AST node creation and memory usage
   - Error handling overhead

3. Semantic Analysis:
   - Symbol table lookups
   - Type checking algorithms
   - Scope resolution

4. Code Generation:
   - IR generation efficiency
   - Optimization passes
   - Output formatting

=== Next Steps ===

1. Open trace file in Instruments for detailed analysis
2. Identify top CPU consumers in your code
3. Compare with baseline measurements
4. Implement targeted optimizations
5. Re-profile to verify improvements

EOF
    
    print_success "Summary saved: $summary_file"
    echo ""
    echo "To open the trace file:"
    echo "  open \"$trace_file\""
    echo ""
    echo "To view the summary:"
    echo "  cat \"$summary_file\""
}

# Function to show recent profiling results
show_profiling_results() {
    print_status "Recent Instruments profiling results:"
    echo ""
    
    if [ -d "$OPTIMIZATION_DIR" ]; then
        # Show recent trace files
        echo "=== Instruments Trace Files ==="
        find "$OPTIMIZATION_DIR" -name "instruments_trace_*.trace" -type d -exec ls -la {} \; 2>/dev/null | tail -5 || echo "No trace files found"
        echo ""
        
        # Show recent summary files
        echo "=== Profiling Summaries ==="
        find "$OPTIMIZATION_DIR" -name "instruments_trace_*_summary.txt" -type f -exec ls -la {} \; 2>/dev/null | tail -3 || echo "No summary files found"
        echo ""
        
        # Show most recent trace file
        local latest_trace=$(find "$OPTIMIZATION_DIR" -name "instruments_trace_*.trace" -type d 2>/dev/null | sort | tail -1)
        if [ -n "$latest_trace" ]; then
            echo "=== Latest Trace File ==="
            echo "Location: $latest_trace"
            echo "Open with: open \"$latest_trace\""
        fi
    else
        print_warning "Optimization directory not found. Run profiling first."
    fi
}

# Function to clean profiling artifacts
clean_profiling() {
    print_status "Cleaning Instruments profiling artifacts..."
    
    if [ -d "$OPTIMIZATION_DIR" ]; then
        # Remove trace files and summaries
        find "$OPTIMIZATION_DIR" -name "instruments_trace_*" -exec rm -rf {} \; 2>/dev/null || true
        print_success "Profiling artifacts cleaned"
    else
        print_warning "No profiling artifacts found"
    fi
    
    # Clean test workload
    rm -f "$BUILD_DIR/instruments_test.asthra" "$BUILD_DIR/instruments_test_output" 2>/dev/null || true
}

# Function to show usage
show_usage() {
    echo "Asthra Compiler Instruments Profiler"
    echo ""
    echo "Usage: $0 [COMMAND]"
    echo ""
    echo "Commands:"
    echo "  check       - Check if Instruments is available"
    echo "  build       - Build compiler with debug symbols"
    echo "  profile     - Launch Instruments with Time Profiler (interactive)"
    echo "  auto        - Run automated profiling session"
    echo "  results     - Show recent profiling results"
    echo "  clean       - Clean profiling artifacts"
    echo "  help        - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 check             # Check Instruments availability"
    echo "  $0 profile           # Interactive profiling session"
    echo "  $0 auto              # Automated profiling"
    echo "  $0 results           # View recent results"
    echo ""
    echo "Workflow:"
    echo "  1. $0 check          # Verify Instruments is available"
    echo "  2. $0 build          # Build debug version"
    echo "  3. $0 profile        # Run profiling session"
    echo "  4. $0 results        # Analyze results"
    echo ""
    echo "Note: Instruments is only available on macOS"
}

# Main script logic
main() {
    local command="${1:-profile}"
    
    case "$command" in
        check)
            check_instruments
            ;;
        build)
            check_instruments && build_debug_compiler
            ;;
        profile)
            check_instruments && build_debug_compiler && launch_instruments interactive
            ;;
        auto)
            check_instruments && build_debug_compiler && launch_instruments automated
            ;;
        results)
            show_profiling_results
            ;;
        clean)
            clean_profiling
            ;;
        help|--help|-h)
            show_usage
            ;;
        *)
            print_error "Unknown command: $command"
            echo ""
            show_usage
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@" 
