#!/bin/bash

# Asthra Compiler Component Tests Runner (CMake-Based)
# Runs tests in hierarchical order with clean builds
# Stops on first component failure
# Uses modern CMake commands and CTest for all operations

# Prompts 1:
#   I ran this script: ./run-tests.sh > err.txt 2>&1  
#   If you notice this message, 'Initial build had some errors', focus on the build failures.
#   And if no build failures, please check the error log file (Last 50 lines are already visible).
# Prompts 2:
#   Can you scan the code and perform a deep analysis of test failure.
#   Think hard to come up with a plan to fix it.
#   Once the plan is ready, you can start working on it.

set -euo pipefail  # Exit on error, undefined vars, and pipe failures

# Script directory and constants
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}"
BUILD_DIR="${PROJECT_ROOT}/build"
LOGS_DIR="${PROJECT_ROOT}/logs"

# Automatic job detection
JOBS=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 2)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default options
SKIP_BUILD=false
TEST_MODE="all"
VERBOSE=false

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    if [ "$status" = "success" ]; then
        echo -e "${GREEN}✓ ${message}${NC}"
    elif [ "$status" = "error" ]; then
        echo -e "${RED}✗ ${message}${NC}"
    elif [ "$status" = "warning" ]; then
        echo -e "${YELLOW}⚠ ${message}${NC}"
    elif [ "$status" = "info" ]; then
        echo -e "${BLUE}→ ${message}${NC}"
    else
        echo -e "${YELLOW}→ ${message}${NC}"
    fi
}

# Function to check dependencies
check_dependencies() {
    local missing_deps=()
    
    print_status "info" "Checking dependencies..."
    
    # Check for required commands
    local required_commands=("cmake" "ctest" "cc" "c++")
    
    for cmd in "${required_commands[@]}"; do
        if ! command -v "$cmd" &> /dev/null; then
            missing_deps+=("$cmd")
        fi
    done
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_status "error" "Missing required dependencies: ${missing_deps[*]}"
        echo ""
        echo "Please install the missing dependencies:"
        echo "  - cmake: Build system generator"
        echo "  - ctest: CMake testing tool"
        echo "  - cc/c++: C/C++ compilers"
        echo ""
        return 1
    fi
    
    print_status "success" "All dependencies found"
    return 0
}

# Function to show help
show_help() {
    cat << EOF
Asthra Compiler Component Tests Runner

USAGE:
    $0 [OPTIONS] [TEST_MODE]

TEST MODES:
    all            Run all tests (default)
    components     Run only core compilation pipeline tests
    cross_cutting  Run only cross-cutting component tests  
    infrastructure Run only supporting infrastructure tests

OPTIONS:
    --skip-build   Skip the build step (use existing build)
    --verbose      Show verbose output
    --jobs N       Number of parallel jobs (default: $JOBS)
    -h, --help     Show this help message

EXAMPLES:
    $0                          # Run all tests with build
    $0 --skip-build             # Run all tests without rebuilding
    $0 components               # Run only core component tests
    $0 --jobs 4 cross_cutting   # Run cross-cutting tests with 4 jobs

DEBUGGING:
    When tests fail, check the log files in: $LOGS_DIR/
    - Each component has its own timestamped log file
    - The last 50 lines of failure logs are displayed automatically
    - Use --verbose for more detailed output

NOTE:
    The script will exit on the first core component failure as these
    tests are hierarchical. Cross-cutting and infrastructure tests will
    continue even if some fail, reporting all failures at the end.

EOF
}

# Function to configure build
configure_build() {
    if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
        print_status "info" "Configuring build system with CMake..."
        cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug
        if [ $? -ne 0 ]; then
            print_status "error" "CMake configuration failed"
            echo ""
            echo "Debug steps:"
            echo "  1. Check CMakeLists.txt for syntax errors"
            echo "  2. Ensure all required dependencies are installed"
            echo "  3. Check cmake output above for specific errors"
            echo ""
            return 1
        fi
        print_status "success" "CMake configuration successful"
    else
        print_status "info" "Using existing CMake configuration"
    fi
    return 0
}

# Function to build project
build_project() {
    print_status "info" "Building compiler and test executables (using $JOBS jobs)..."
    
    # Build main project
    if ! cmake --build "${BUILD_DIR}" -j"${JOBS}"; then
        print_status "error" "Main build failed"
        echo ""
        echo "Debug steps:"
        echo "  1. Check compilation errors above"
        echo "  2. Try cleaning: cmake --build ${BUILD_DIR} --target clean"
        echo "  3. Remove build directory: rm -rf ${BUILD_DIR}"
        echo "  4. Re-run this script"
        echo ""
        return 1
    fi
    
    # Build test executables
    if ! cmake --build "${BUILD_DIR}" --target build-tests; then
        print_status "error" "Test executable build failed"
        echo ""
        echo "This usually happens when:"
        echo "  1. Test source files have compilation errors"
        echo "  2. Test CMakeLists.txt has configuration issues"
        echo "  3. Dependencies for tests are missing"
        echo ""
        echo "Try running: cmake --build ${BUILD_DIR} --target build-tests --verbose"
        echo ""
        return 1
    fi
    
    print_status "success" "Build completed successfully"
    return 0
}

# Create logs directory
create_logs_directory() {
    if ! mkdir -p "${LOGS_DIR}"; then
        print_status "error" "Failed to create logs directory: ${LOGS_DIR}"
        return 1
    fi
    return 0
}

# Function to run tests for a component
run_component_tests() {
    local component=$1
    local component_name=$2
    local timestamp=$(date '+%Y%m%d_%H%M%S')
    local log_file="${LOGS_DIR}/${component}_${timestamp}.log"
    
    print_status "info" "Testing ${component_name}..."
    
    # Check if component test directory exists
    if [ ! -d "${PROJECT_ROOT}/tests/${component}" ]; then
        print_status "info" "Skipping ${component_name} - no test directory found"
        return 0
    fi
    
    # Run component tests from build directory
    local ctest_args=("--test-dir" "${BUILD_DIR}" "-L" "${component}")
    
    if [ "$VERBOSE" = true ]; then
        ctest_args+=("--verbose")
    else
        ctest_args+=("--output-on-failure")
    fi
    
    if ! ctest "${ctest_args[@]}" >"${log_file}" 2>&1; then
        print_status "error" "${component_name} tests failed"
        print_status "info" "Full log: ${log_file}"
        echo ""
        
        # Enhanced error reporting
        print_status "warning" "Test failure summary:"
        echo ""
        
        # Extract failed test names
        grep -E "Failed.*Test #|\\*\\*\\*Failed" "${log_file}" | head -10 || true
        
        echo ""
        print_status "warning" "Last 50 lines of failure log:"
        tail -50 "${log_file}"
        echo ""
        
        # Provide debugging hints
        echo "Debug hints:"
        echo "  1. Check the full log: less ${log_file}"
        echo "  2. Run specific test: ctest --test-dir ${BUILD_DIR} -R <test_name> --verbose"
        echo "  3. Check test source: find tests/${component} -name '*.c'"
        echo ""
        
        return 1
    fi
    
    print_status "success" "${component_name} tests passed"
    if [ "$VERBOSE" = true ]; then
        print_status "info" "Log: ${log_file}"
    fi
    
    return 0
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --jobs)
            if [[ -n "$2" ]] && [[ "$2" =~ ^[0-9]+$ ]]; then
                JOBS="$2"
                shift 2
            else
                print_status "error" "--jobs requires a numeric argument"
                exit 1
            fi
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        components|core)
            TEST_MODE="components"
            shift
            ;;
        cross_cutting|cross-cutting)
            TEST_MODE="cross_cutting"
            shift
            ;;
        infrastructure|infra)
            TEST_MODE="infrastructure"
            shift
            ;;
        all)
            TEST_MODE="all"
            shift
            ;;
        *)
            print_status "error" "Unknown option: $1"
            echo ""
            show_help
            exit 1
            ;;
    esac
done

# Main execution
echo "========================================="
echo "Asthra Compiler Component Tests Runner"
echo "========================================="
if [ "$TEST_MODE" != "all" ]; then
    echo "Mode: $TEST_MODE"
fi
if [ "$SKIP_BUILD" = true ]; then
    echo "Build: SKIPPED"
else
    echo "Build: ENABLED (${JOBS} jobs)"
fi
if [ "$VERBOSE" = true ]; then
    echo "Output: VERBOSE"
fi
echo "========================================="
echo ""

# Check dependencies
if ! check_dependencies; then
    exit 1
fi
echo ""

# Create logs directory
if ! create_logs_directory; then
    exit 1
fi

# Configure and build if not skipping
if [ "$SKIP_BUILD" = false ]; then
    # Configure
    if ! configure_build; then
        exit 1
    fi
    echo ""
    
    # Build
    if ! build_project; then
        exit 1
    fi
    echo ""
else
    # Verify build exists when skipping
    if [ ! -f "${BUILD_DIR}/CMakeCache.txt" ]; then
        print_status "error" "No build found. Cannot skip build on first run."
        echo "Please run without --skip-build first."
        exit 1
    fi
    print_status "info" "Skipping build step as requested"
fi

# Core Compilation Pipeline Components (in hierarchical order)
components=(
    "lexer:Lexer"
    "parser:Parser"
    "semantic:Semantic Analysis"
    "codegen:Code Generation"
    "optimization:Optimization"
    "linker:Linker"
    "platform:Platform"
    "runtime:Runtime System"
    "stdlib:Standard Library"
    "integration:Integration"
)

# Track overall test status
overall_exit_code=0

# Run tests for each component in order
if [ "$TEST_MODE" = "all" ] || [ "$TEST_MODE" = "components" ]; then
    for component_pair in "${components[@]}"; do
        IFS=':' read -r component name <<< "$component_pair"
        
        run_component_tests "$component" "$name"
        test_exit_code=$?
        
        if [ $test_exit_code -ne 0 ]; then
            echo ""
            print_status "error" "Test suite stopped due to failure in ${name}"
            echo "========================================="
            overall_exit_code=1
            # For core components, we still exit early as they are hierarchical
            exit $overall_exit_code
        fi
        
        echo ""
    done
    
    # If we get here, all core components passed
    print_status "success" "All core component tests passed!"
    echo ""
fi

# Cross-cutting components (optional - continue even if these fail)
if [ "$TEST_MODE" = "all" ] || [ "$TEST_MODE" = "cross_cutting" ]; then
    echo "Testing cross-cutting components..."
    echo ""

    cross_cutting=(
        "ffi:FFI"
        "concurrency:Concurrency"
        "memory:Memory Management"
        "security:Security"
        "performance:Performance"
        "patterns:Patterns"
        "pipeline:Pipeline"
    )

    failed_cross_cutting=()

    for component_pair in "${cross_cutting[@]}"; do
        IFS=':' read -r component name <<< "$component_pair"
        
        run_component_tests "$component" "$name"
        test_exit_code=$?
        
        if [ $test_exit_code -ne 0 ]; then
            failed_cross_cutting+=("$name")
            overall_exit_code=1
        fi
        
        echo ""
    done
fi

# Supporting Infrastructure components
if [ "$TEST_MODE" = "all" ] || [ "$TEST_MODE" = "infrastructure" ]; then
    echo "Testing supporting infrastructure..."
    echo ""

    infrastructure=(
        "framework:Test Framework"
        "ai_annotations:AI Annotations"
        "ai_api:AI API"
        "ai_linter:AI Linter"
        "diagnostics:Diagnostics"
    )

    failed_infrastructure=()

    for component_pair in "${infrastructure[@]}"; do
        IFS=':' read -r component name <<< "$component_pair"
        
        run_component_tests "$component" "$name"
        test_exit_code=$?
        
        if [ $test_exit_code -ne 0 ]; then
            failed_infrastructure+=("$name")
            overall_exit_code=1
        fi
        
        echo ""
    done
fi

# Summary
echo "========================================="
echo "Test Suite Summary"
echo "========================================="

if [ "$TEST_MODE" = "all" ] || [ "$TEST_MODE" = "components" ]; then
    print_status "success" "Core compilation pipeline: PASSED"
fi

if [ "$TEST_MODE" = "all" ] || [ "$TEST_MODE" = "cross_cutting" ]; then
    if [ ${#failed_cross_cutting[@]} -eq 0 ]; then
        print_status "success" "Cross-cutting components: PASSED"
    else
        print_status "error" "Cross-cutting components failed: ${failed_cross_cutting[*]}"
    fi
fi

if [ "$TEST_MODE" = "all" ] || [ "$TEST_MODE" = "infrastructure" ]; then
    if [ ${#failed_infrastructure[@]} -eq 0 ]; then
        print_status "success" "Supporting infrastructure: PASSED"
    else
        print_status "error" "Supporting infrastructure failed: ${failed_infrastructure[*]}"
    fi
fi

echo "========================================="

# Exit with overall status - 0 if all tests passed, 1 if any failed
exit $overall_exit_code