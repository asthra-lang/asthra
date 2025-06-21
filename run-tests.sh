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

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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
    else
        echo -e "${YELLOW}→ ${message}${NC}"
    fi
}

# Create logs directory if it doesn't exist
mkdir -p logs

# Function to run tests for a component
run_component_tests() {
    local component=$1
    local component_name=$2
    local timestamp=$(date '+%Y%m%d_%H%M%S')
    local log_file="logs/${component}_${timestamp}.log"
    
    print_status "info" "Testing ${component_name}..."
    
    # Run component tests from build directory
    ctest --test-dir build -L ${component} --output-on-failure >"${log_file}" 2>&1
    local test_result=$?
    
    if [ $test_result -eq 0 ]; then
        print_status "success" "${component_name} tests passed"
        print_status "info" "Log: ${log_file}"
    else
        print_status "error" "${component_name} tests failed"
        print_status "info" "Log: ${log_file}"
        echo ""
        print_status "warning" "Last 50 lines of failure log:"
        tail -50 "${log_file}"
        echo ""
    fi
    
    return $test_result
}

# Parse command line arguments
TEST_MODE="all"
if [ $# -gt 0 ]; then
    case "$1" in
        "components"|"core")
            TEST_MODE="components"
            ;;
        "cross_cutting"|"cross-cutting")
            TEST_MODE="cross_cutting"
            ;;
        "infrastructure"|"infra")
            TEST_MODE="infrastructure"
            ;;
        "all")
            TEST_MODE="all"
            ;;
        *)
            echo "Usage: $0 [components|cross_cutting|infrastructure|all]"
            echo "  components     - Run only core compilation pipeline tests"
            echo "  cross_cutting  - Run only cross-cutting component tests"
            echo "  infrastructure - Run only supporting infrastructure tests"
            echo "  all           - Run all tests (default)"
            exit 1
            ;;
    esac
fi

# Main execution
echo "========================================="
echo "Asthra Compiler Component Tests Runner"
if [ "$TEST_MODE" != "all" ]; then
    echo "Mode: $TEST_MODE"
fi
echo "========================================="
echo ""

# Configure with CMake (only if needed)
if [ ! -f "build/CMakeCache.txt" ]; then
    print_status "info" "Configuring build system with CMake..."
    cmake -B build -DCMAKE_BUILD_TYPE=Debug
    if [ $? -ne 0 ]; then
        print_status "error" "CMake configuration failed"
        exit 1
    fi
else
    print_status "info" "Using existing CMake configuration..."
fi

# Initial build to ensure all test executables exist
print_status "info" "Building compiler and test executables..."
cmake --build build -j8 && cmake --build build --target build-tests
build_result=$?
if [ $build_result -ne 0 ]; then
    print_status "error" "Build failed with errors"
    echo ""
    print_status "error" "Cannot run tests when build fails"
    echo "========================================="
    exit 1
fi
echo ""

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
        
        # Check if component test directory exists
        if [ -d "tests/${component}" ]; then
            set +e  # Temporarily disable exit on error
            run_component_tests "$component" "$name"
            test_exit_code=$?
            set -e  # Re-enable exit on error
            
            if [ $test_exit_code -ne 0 ]; then
                echo ""
                print_status "error" "Test suite stopped due to failure in ${name}"
                echo "========================================="
                overall_exit_code=1
                # For core components, we still exit early as they are hierarchical
                exit $overall_exit_code
            fi
        else
            print_status "info" "Skipping ${name} - no test directory found"
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
        
        if [ -d "tests/${component}" ]; then
            set +e  # Temporarily disable exit on error
            run_component_tests "$component" "$name"
            test_exit_code=$?
            set -e  # Re-enable exit on error
            
            if [ $test_exit_code -ne 0 ]; then
                failed_cross_cutting+=("$name")
                overall_exit_code=1
            fi
        else
            print_status "info" "Skipping ${name} - no test directory found"
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
        
        if [ -d "tests/${component}" ]; then
            set +e  # Temporarily disable exit on error
            run_component_tests "$component" "$name"
            test_exit_code=$?
            set -e  # Re-enable exit on error
            
            if [ $test_exit_code -ne 0 ]; then
                failed_infrastructure+=("$name")
                overall_exit_code=1
            fi
        else
            print_status "info" "Skipping ${name} - no test directory found"
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
