#!/bin/bash

# Clang Sanitizer Runner for Asthra Compiler
# Comprehensive script for running all Clang Sanitizers with proper reporting and analysis

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OPTIMIZATION_DIR="$PROJECT_ROOT/optimization"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Sanitizer environment variables
export ASAN_OPTIONS="abort_on_error=1:check_initialization_order=1:strict_init_order=1:detect_stack_use_after_return=1:detect_leaks=1:halt_on_error=1:log_path=$OPTIMIZATION_DIR/asan_log"
export TSAN_OPTIONS="abort_on_error=1:halt_on_error=1:log_path=$OPTIMIZATION_DIR/tsan_log"
export UBSAN_OPTIONS="abort_on_error=1:halt_on_error=1:log_path=$OPTIMIZATION_DIR/ubsan_log:print_stacktrace=1"
export MSAN_OPTIONS="abort_on_error=1:halt_on_error=1:log_path=$OPTIMIZATION_DIR/msan_log:print_stats=1"

print_status() {
    echo -e "${BLUE}[SANITIZER]${NC} $1"
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

print_info() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

print_section() {
    echo -e "${MAGENTA}[SECTION]${NC} $1"
}

# Check if we're on a supported platform
check_platform() {
    local platform=$(uname -s)
    local arch=$(uname -m)
    
    print_info "Platform: $platform $arch"
    
    if [[ "$platform" == "Darwin" ]]; then
        if ! command -v clang >/dev/null 2>&1; then
            print_error "Clang not found. Please install Xcode Command Line Tools:"
            echo "  xcode-select --install"
            exit 1
        fi
        print_info "Using Clang on macOS - all sanitizers supported"
    elif [[ "$platform" == "Linux" ]]; then
        if command -v clang >/dev/null 2>&1; then
            print_info "Using Clang on Linux - all sanitizers supported"
        elif command -v gcc >/dev/null 2>&1; then
            print_warning "Using GCC on Linux - MemorySanitizer not available"
        else
            print_error "Neither Clang nor GCC found"
            exit 1
        fi
    else
        print_warning "Unsupported platform: $platform"
        print_info "Proceeding with basic sanitizer support"
    fi
}

# Create timestamped report directory
create_report_dir() {
    local timestamp=$(date +%Y%m%d_%H%M%S)
    REPORT_DIR="$OPTIMIZATION_DIR/sanitizer_reports_$timestamp"
    mkdir -p "$REPORT_DIR"
    print_info "Reports will be saved to: $REPORT_DIR"
}

# Build with specific sanitizer
build_sanitizer() {
    local sanitizer="$1"
    local target="$2"
    
    print_status "Building with $sanitizer..."
    
    cd "$PROJECT_ROOT"
    
    # Ensure bin directory exists
    mkdir -p bin
    
    case "$sanitizer" in
        asan)
            if make directories-sanitizers >/dev/null 2>&1 && make sanitizer-asan >/dev/null 2>&1; then
                print_success "$sanitizer build completed"
                return 0
            else
                print_error "$sanitizer build failed"
                return 1
            fi
            ;;
        tsan)
            if make directories-sanitizers >/dev/null 2>&1 && make sanitizer-tsan >/dev/null 2>&1; then
                print_success "$sanitizer build completed"
                return 0
            else
                print_error "$sanitizer build failed"
                return 1
            fi
            ;;
        ubsan)
            if make directories-sanitizers >/dev/null 2>&1 && make sanitizer-ubsan >/dev/null 2>&1; then
                print_success "$sanitizer build completed"
                return 0
            else
                print_error "$sanitizer build failed"
                return 1
            fi
            ;;
        msan)
            # Check if MemorySanitizer is available (Clang only)
            if [[ "$(uname -s)" == "Darwin" ]] || command -v clang >/dev/null 2>&1; then
                if make directories-sanitizers >/dev/null 2>&1 && make sanitizer-msan >/dev/null 2>&1; then
                    print_success "$sanitizer build completed"
                    return 0
                else
                    print_warning "$sanitizer build failed (may not be supported on this platform)"
                    return 1
                fi
            else
                print_warning "MemorySanitizer not available with GCC"
                return 1
            fi
            ;;
        asan-ubsan)
            if make directories-sanitizers >/dev/null 2>&1 && make sanitizer-asan-ubsan >/dev/null 2>&1; then
                print_success "$sanitizer build completed"
                return 0
            else
                print_error "$sanitizer build failed"
                return 1
            fi
            ;;
        *)
            print_error "Unknown sanitizer: $sanitizer"
            return 1
            ;;
    esac
}

# Run sanitizer with specific executable and arguments
run_sanitizer() {
    local sanitizer="$1"
    local executable="$2"
    local exec_args="${@:3}"
    local log_file="$REPORT_DIR/${sanitizer}_$(basename "$executable")_$(date +%H%M%S).log"
    
    print_status "Running $sanitizer on $executable..."
    
    # Set sanitizer-specific environment variables
    case "$sanitizer" in
        asan)
            export ASAN_OPTIONS="$ASAN_OPTIONS:log_path=${log_file%.log}"
            ;;
        tsan)
            export TSAN_OPTIONS="$TSAN_OPTIONS:log_path=${log_file%.log}"
            ;;
        ubsan)
            export UBSAN_OPTIONS="$UBSAN_OPTIONS:log_path=${log_file%.log}"
            ;;
        msan)
            export MSAN_OPTIONS="$MSAN_OPTIONS:log_path=${log_file%.log}"
            ;;
    esac
    
    # Run the executable and capture output
    local exit_code=0
    if "$executable" $exec_args >"$log_file" 2>&1; then
        print_success "$sanitizer run completed successfully"
    else
        exit_code=$?
        print_warning "$sanitizer detected issues (exit code: $exit_code)"
    fi
    
    # Check for sanitizer-specific log files
    local sanitizer_logs=()
    case "$sanitizer" in
        asan)
            # Look for sanitizer-specific logs, but exclude the main log file
            sanitizer_logs=($(find "$(dirname "$log_file")" -name "$(basename "${log_file%.log}").*" -not -name "$(basename "$log_file")" 2>/dev/null || true))
            ;;
        tsan)
            sanitizer_logs=($(find "$(dirname "$log_file")" -name "$(basename "${log_file%.log}").*" -not -name "$(basename "$log_file")" 2>/dev/null || true))
            ;;
        ubsan)
            sanitizer_logs=($(find "$(dirname "$log_file")" -name "$(basename "${log_file%.log}").*" -not -name "$(basename "$log_file")" 2>/dev/null || true))
            ;;
        msan)
            sanitizer_logs=($(find "$(dirname "$log_file")" -name "$(basename "${log_file%.log}").*" -not -name "$(basename "$log_file")" 2>/dev/null || true))
            ;;
    esac
    
    # Analyze results
    local issues_found=false
    if [[ ${#sanitizer_logs[@]} -gt 0 ]]; then
        for log in "${sanitizer_logs[@]}"; do
            # Safety check: never append a file to itself
            if [[ "$log" == "$log_file" ]]; then
                continue
            fi
            
            if [[ -s "$log" ]]; then
                issues_found=true
                print_warning "Issues found in: $log"
                # Copy sanitizer log content to main log
                echo "=== SANITIZER LOG: $log ===" >> "$log_file"
                cat "$log" >> "$log_file"
                echo "=== END SANITIZER LOG ===" >> "$log_file"
            fi
        done
    fi
    
    if [[ -s "$log_file" ]] && ! $issues_found; then
        # Check log content for issues even if no separate sanitizer logs
        if grep -q -E "(ERROR|LEAK|WARNING|runtime error)" "$log_file"; then
            issues_found=true
            print_warning "Issues detected in output log"
        fi
    fi
    
    if ! $issues_found && [[ $exit_code -eq 0 ]]; then
        print_success "No issues detected with $sanitizer"
        echo "No issues detected" > "$log_file"
    fi
    
    return $exit_code
}

# Run AddressSanitizer
run_asan() {
    local executable="${1:-bin/asthra-asan}"
    local exec_args="${2:---version}"
    
    print_section "Running AddressSanitizer (Memory Errors & Leaks)"
    
    if build_sanitizer "asan"; then
        run_sanitizer "asan" "$executable" $exec_args
    else
        print_error "Skipping AddressSanitizer due to build failure"
        return 1
    fi
}

# Run ThreadSanitizer
run_tsan() {
    local executable="${1:-bin/asthra-tsan}"
    local exec_args="${2:---version}"
    
    print_section "Running ThreadSanitizer (Race Conditions)"
    
    if build_sanitizer "tsan"; then
        run_sanitizer "tsan" "$executable" $exec_args
    else
        print_error "Skipping ThreadSanitizer due to build failure"
        return 1
    fi
}

# Run UndefinedBehaviorSanitizer
run_ubsan() {
    local executable="${1:-bin/asthra-ubsan}"
    local exec_args="${2:---version}"
    
    print_section "Running UndefinedBehaviorSanitizer (Undefined Behavior)"
    
    if build_sanitizer "ubsan"; then
        run_sanitizer "ubsan" "$executable" $exec_args
    else
        print_error "Skipping UndefinedBehaviorSanitizer due to build failure"
        return 1
    fi
}

# Run MemorySanitizer
run_msan() {
    local executable="${1:-bin/asthra-msan}"
    local exec_args="${2:---version}"
    
    print_section "Running MemorySanitizer (Uninitialized Memory)"
    
    if build_sanitizer "msan"; then
        run_sanitizer "msan" "$executable" $exec_args
    else
        print_warning "Skipping MemorySanitizer (not available or build failed)"
        return 1
    fi
}

# Run combined AddressSanitizer + UndefinedBehaviorSanitizer
run_asan_ubsan() {
    local executable="${1:-bin/asthra-asan-ubsan}"
    local exec_args="${2:---version}"
    
    print_section "Running Combined AddressSanitizer + UndefinedBehaviorSanitizer"
    
    if build_sanitizer "asan-ubsan"; then
        run_sanitizer "asan-ubsan" "$executable" $exec_args
    else
        print_error "Skipping combined sanitizers due to build failure"
        return 1
    fi
}

# Run comprehensive sanitizer analysis
run_comprehensive() {
    local executable_base="${1:-bin/asthra}"
    local exec_args="${2:---version}"
    
    print_section "Running Comprehensive Sanitizer Analysis"
    
    create_report_dir
    
    local results=()
    local total_issues=0
    
    # Run each sanitizer
    print_info "Starting comprehensive analysis with all available sanitizers..."
    
    # AddressSanitizer
    if run_asan "${executable_base}-asan" "$exec_args"; then
        results+=("AddressSanitizer: PASSED")
    else
        results+=("AddressSanitizer: ISSUES FOUND")
        ((total_issues++))
    fi
    
    # ThreadSanitizer
    if run_tsan "${executable_base}-tsan" "$exec_args"; then
        results+=("ThreadSanitizer: PASSED")
    else
        results+=("ThreadSanitizer: ISSUES FOUND")
        ((total_issues++))
    fi
    
    # UndefinedBehaviorSanitizer
    if run_ubsan "${executable_base}-ubsan" "$exec_args"; then
        results+=("UndefinedBehaviorSanitizer: PASSED")
    else
        results+=("UndefinedBehaviorSanitizer: ISSUES FOUND")
        ((total_issues++))
    fi
    
    # MemorySanitizer (if available)
    if run_msan "${executable_base}-msan" "$exec_args"; then
        results+=("MemorySanitizer: PASSED")
    else
        results+=("MemorySanitizer: SKIPPED OR ISSUES FOUND")
    fi
    
    # Combined AddressSanitizer + UndefinedBehaviorSanitizer
    if run_asan_ubsan "${executable_base}-asan-ubsan" "$exec_args"; then
        results+=("Combined ASAN+UBSAN: PASSED")
    else
        results+=("Combined ASAN+UBSAN: ISSUES FOUND")
        ((total_issues++))
    fi
    
    # Generate summary report
    generate_summary_report "${results[@]}"
    
    print_section "Comprehensive Analysis Complete"
    print_info "Total sanitizers with issues: $total_issues"
    print_info "Detailed reports saved in: $REPORT_DIR"
    
    if [[ $total_issues -eq 0 ]]; then
        print_success "All sanitizer checks passed!"
        return 0
    else
        print_warning "Issues found in $total_issues sanitizer(s)"
        return 1
    fi
}

# Run test suite with sanitizers
run_test_suite() {
    local sanitizer="${1:-asan}"
    
    print_section "Running Test Suite with $sanitizer"
    
    create_report_dir
    
    # Build test suite with sanitizer
    print_status "Building test suite with $sanitizer..."
    cd "$PROJECT_ROOT"
    
    case "$sanitizer" in
        asan)
            if ! make clean >/dev/null 2>&1 || ! make sanitizer-asan >/dev/null 2>&1; then
                print_error "Failed to build test suite with AddressSanitizer"
                return 1
            fi
            ;;
        tsan)
            if ! make clean >/dev/null 2>&1 || ! make sanitizer-tsan >/dev/null 2>&1; then
                print_error "Failed to build test suite with ThreadSanitizer"
                return 1
            fi
            ;;
        ubsan)
            if ! make clean >/dev/null 2>&1 || ! make sanitizer-ubsan >/dev/null 2>&1; then
                print_error "Failed to build test suite with UndefinedBehaviorSanitizer"
                return 1
            fi
            ;;
        *)
            print_error "Unsupported sanitizer for test suite: $sanitizer"
            return 1
            ;;
    esac
    
    # Run tests
    local test_log="$REPORT_DIR/test_suite_${sanitizer}_$(date +%H%M%S).log"
    print_status "Running test suite..."
    
    if make test >"$test_log" 2>&1; then
        print_success "Test suite completed successfully with $sanitizer"
    else
        print_warning "Test suite found issues with $sanitizer"
        print_info "Test log saved to: $test_log"
        return 1
    fi
    
    return 0
}

# Generate comprehensive summary report
generate_summary_report() {
    local results=("$@")
    local summary_file="$REPORT_DIR/sanitizer_summary.txt"
    
    print_status "Generating summary report..."
    
    {
        echo "Asthra Compiler Sanitizer Analysis Summary"
        echo "=========================================="
        echo "Timestamp: $(date)"
        echo "Platform: $(uname -s) $(uname -m)"
        echo "Compiler: $(cc --version | head -n1)"
        echo ""
        echo "Results:"
        echo "--------"
        for result in "${results[@]}"; do
            echo "  $result"
        done
        echo ""
        echo "Detailed Reports:"
        echo "----------------"
        for log_file in "$REPORT_DIR"/*.log; do
            if [[ -f "$log_file" ]]; then
                local basename=$(basename "$log_file")
                local size=$(wc -l < "$log_file" 2>/dev/null || echo "0")
                echo "  $basename ($size lines)"
                
                # Show first few lines of issues if any
                if grep -q -E "(ERROR|LEAK|WARNING|runtime error)" "$log_file" 2>/dev/null; then
                    echo "    Issues detected - see full log for details"
                fi
            fi
        done
        echo ""
        echo "Environment Variables Used:"
        echo "--------------------------"
        echo "ASAN_OPTIONS: $ASAN_OPTIONS"
        echo "TSAN_OPTIONS: $TSAN_OPTIONS"
        echo "UBSAN_OPTIONS: $UBSAN_OPTIONS"
        echo "MSAN_OPTIONS: $MSAN_OPTIONS"
    } > "$summary_file"
    
    print_success "Summary report saved to: $summary_file"
    
    # Display summary to console
    echo ""
    print_section "SANITIZER ANALYSIS SUMMARY"
    for result in "${results[@]}"; do
        if [[ "$result" == *"PASSED"* ]]; then
            print_success "$result"
        elif [[ "$result" == *"SKIPPED"* ]]; then
            print_warning "$result"
        else
            print_error "$result"
        fi
    done
}

# Show usage information
show_usage() {
    echo "Clang Sanitizer Runner for Asthra Compiler"
    echo ""
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  asan [exe] [args]      - Run AddressSanitizer (memory errors, leaks)"
    echo "  tsan [exe] [args]      - Run ThreadSanitizer (race conditions)"
    echo "  ubsan [exe] [args]     - Run UndefinedBehaviorSanitizer (undefined behavior)"
    echo "  msan [exe] [args]      - Run MemorySanitizer (uninitialized memory)"
    echo "  asan-ubsan [exe] [args] - Run combined AddressSanitizer + UndefinedBehaviorSanitizer"
    echo "  comprehensive          - Run all available sanitizers"
    echo "  test-suite [sanitizer] - Run test suite with specified sanitizer (asan/tsan/ubsan)"
    echo "  check                  - Check platform and compiler support"
    echo "  help                   - Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 check                           # Check platform support"
    echo "  $0 comprehensive                   # Run all sanitizers"
    echo "  $0 asan bin/asthra --version       # Run AddressSanitizer on compiler"
    echo "  $0 test-suite asan                 # Run test suite with AddressSanitizer"
    echo "  $0 ubsan bin/asthra examples/test.asthra  # Run UBSan on specific file"
    echo ""
    echo "Environment Variables:"
    echo "  ASAN_OPTIONS, TSAN_OPTIONS, UBSAN_OPTIONS, MSAN_OPTIONS are automatically set"
    echo "  Override them before running this script if needed"
    echo ""
    echo "Notes:"
    echo "  - MemorySanitizer (msan) is only available with Clang"
    echo "  - All sanitizers require debug builds (-O1 or -O0)"
    echo "  - Reports are saved with timestamps in optimization/ directory"
    echo "  - Use 'comprehensive' for complete analysis of all sanitizers"
}

# Main function
main() {
    local command="${1:-comprehensive}"
    
    # Ensure optimization directory exists
    mkdir -p "$OPTIMIZATION_DIR"
    
    case "$command" in
        check)
            check_platform
            ;;
        asan)
            check_platform
            create_report_dir
            run_asan "${2:-bin/asthra-asan}" "${3:---version}"
            ;;
        tsan)
            check_platform
            create_report_dir
            run_tsan "${2:-bin/asthra-tsan}" "${3:---version}"
            ;;
        ubsan)
            check_platform
            create_report_dir
            run_ubsan "${2:-bin/asthra-ubsan}" "${3:---version}"
            ;;
        msan)
            check_platform
            create_report_dir
            run_msan "${2:-bin/asthra-msan}" "${3:---version}"
            ;;
        asan-ubsan)
            check_platform
            create_report_dir
            run_asan_ubsan "${2:-bin/asthra-asan-ubsan}" "${3:---version}"
            ;;
        comprehensive)
            check_platform
            run_comprehensive "${2:-bin/asthra}" "${3:---version}"
            ;;
        test-suite)
            check_platform
            run_test_suite "${2:-asan}"
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
