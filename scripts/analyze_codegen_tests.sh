#!/bin/bash
# analyze_codegen_tests.sh - Codegen Test Dependency Analysis
# Part of Phase 1: Dependency Analysis and Cleanup

set -e

# Configuration
CODEGEN_DIR="tests/codegen"
RESULTS_FILE="build/codegen_analysis_results.txt"
RESULTS_DIR="build/codegen_analysis"
SRC_DIR="src"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create results directory
mkdir -p "$RESULTS_DIR"
mkdir -p build

echo "Codegen Test Analysis Results" > "$RESULTS_FILE"
echo "============================" >> "$RESULTS_FILE"
echo "Generated: $(date)" >> "$RESULTS_FILE"
echo "Total test files found: $(find $CODEGEN_DIR -name 'test_*.c' | wc -l)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

# Initialize counters
total_tests=0
successful_compilations=0
failed_compilations=0
tests_with_main=0
tests_without_main=0

echo -e "${BLUE}Starting Codegen Test Analysis...${NC}"
echo -e "${BLUE}=================================${NC}"

# Analyze each test file
for test_file in "$CODEGEN_DIR"/test_*.c; do
    if [ ! -f "$test_file" ]; then
        continue
    fi
    
    test_name=$(basename "$test_file" .c)
    total_tests=$((total_tests + 1))
    
    echo -e "${YELLOW}Analyzing $test_name...${NC}"
    echo "Analyzing $test_name..." >> "$RESULTS_FILE"
    
    # Try to compile individually with basic flags
    error_file="/tmp/${test_name}_errors.txt"
    if gcc -c -I. -I"$SRC_DIR" -Iruntime -Itests/framework \
           -DASTHRA_TEST_MODE=1 \
           -o "/tmp/${test_name}.o" "$test_file" 2>"$error_file"; then
        echo -e "  ${GREEN}✅ Compilation: SUCCESS${NC}"
        echo "  ✅ Compilation: SUCCESS" >> "$RESULTS_FILE"
        successful_compilations=$((successful_compilations + 1))
        rm -f "/tmp/${test_name}.o"
    else
        echo -e "  ${RED}❌ Compilation: FAILED${NC}"
        echo "  ❌ Compilation: FAILED" >> "$RESULTS_FILE"
        failed_compilations=$((failed_compilations + 1))
        
        # Save detailed error analysis
        echo "  Errors:" >> "$RESULTS_FILE"
        if [ -s "$error_file" ]; then
            sed 's/^/    /' "$error_file" >> "$RESULTS_FILE"
            
            # Copy full error log to analysis directory
            cp "$error_file" "$RESULTS_DIR/${test_name}_errors.txt"
        else
            echo "    No error output captured" >> "$RESULTS_FILE"
        fi
    fi
    
    # Check dependencies and includes
    includes=$(grep -c '#include' "$test_file" || echo "0")
    echo "  Includes: $includes" >> "$RESULTS_FILE"
    
    # List all includes for dependency mapping
    echo "  Include files:" >> "$RESULTS_FILE"
    grep '#include' "$test_file" | sed 's/^/    /' >> "$RESULTS_FILE" || echo "    None" >> "$RESULTS_FILE"
    
    # Check for main function
    if grep -q "int main\|void main" "$test_file"; then
        echo "  Has main: YES" >> "$RESULTS_FILE"
        tests_with_main=$((tests_with_main + 1))
    else
        echo "  Has main: NO" >> "$RESULTS_FILE"
        tests_without_main=$((tests_without_main + 1))
    fi
    
    # Check for test framework usage
    if grep -q "test_assert\|TEST_\|test_suite" "$test_file"; then
        echo "  Uses test framework: YES" >> "$RESULTS_FILE"
    else
        echo "  Uses test framework: NO" >> "$RESULTS_FILE"
    fi
    
    # Check file size and complexity
    lines=$(wc -l < "$test_file")
    echo "  Lines of code: $lines" >> "$RESULTS_FILE"
    
    # Check for specific codegen dependencies
    if grep -q "code_generator\|CodeGenerator" "$test_file"; then
        echo "  Uses CodeGenerator: YES" >> "$RESULTS_FILE"
    fi
    
    if grep -q "elf_writer\|ElfWriter" "$test_file"; then
        echo "  Uses ElfWriter: YES" >> "$RESULTS_FILE"
    fi
    
    if grep -q "register_allocator\|RegisterAllocator" "$test_file"; then
        echo "  Uses RegisterAllocator: YES" >> "$RESULTS_FILE"
    fi
    
    if grep -q "optimizer\|Optimizer" "$test_file"; then
        echo "  Uses Optimizer: YES" >> "$RESULTS_FILE"
    fi
    
    echo "" >> "$RESULTS_FILE"
    
    # Clean up temporary files
    rm -f "$error_file"
done

# Generate summary statistics
echo "" >> "$RESULTS_FILE"
echo "=====================================" >> "$RESULTS_FILE"
echo "           SUMMARY STATISTICS         " >> "$RESULTS_FILE"
echo "=====================================" >> "$RESULTS_FILE"
echo "Total test files analyzed: $total_tests" >> "$RESULTS_FILE"
echo "Successful compilations: $successful_compilations" >> "$RESULTS_FILE"
echo "Failed compilations: $failed_compilations" >> "$RESULTS_FILE"
echo "Tests with main function: $tests_with_main" >> "$RESULTS_FILE"
echo "Tests without main function: $tests_without_main" >> "$RESULTS_FILE"

if [ $total_tests -gt 0 ]; then
    success_rate=$(echo "scale=2; $successful_compilations * 100 / $total_tests" | bc -l 2>/dev/null || echo "0")
    echo "Success rate: ${success_rate}%" >> "$RESULTS_FILE"
else
    echo "Success rate: 0%" >> "$RESULTS_FILE"
fi

echo "" >> "$RESULTS_FILE"

# Generate recommendations
echo "=====================================" >> "$RESULTS_FILE"
echo "           RECOMMENDATIONS            " >> "$RESULTS_FILE"
echo "=====================================" >> "$RESULTS_FILE"

if [ $failed_compilations -gt 0 ]; then
    echo "Priority Actions:" >> "$RESULTS_FILE"
    echo "1. Fix compilation errors in $failed_compilations test files" >> "$RESULTS_FILE"
    echo "2. Review missing includes and dependencies" >> "$RESULTS_FILE"
    echo "3. Standardize test framework usage" >> "$RESULTS_FILE"
    echo "4. Create dependency mapping for complex tests" >> "$RESULTS_FILE"
    echo "" >> "$RESULTS_FILE"
    echo "Detailed error logs saved in: $RESULTS_DIR/" >> "$RESULTS_FILE"
fi

if [ $tests_without_main -gt 0 ]; then
    echo "Note: $tests_without_main tests don't have main functions" >> "$RESULTS_FILE"
    echo "These may be unit test files requiring test framework integration" >> "$RESULTS_FILE"
fi

# Display summary to console
echo ""
echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}         ANALYSIS COMPLETE           ${NC}"
echo -e "${BLUE}=====================================${NC}"
echo -e "Total test files: ${YELLOW}$total_tests${NC}"
echo -e "Successful compilations: ${GREEN}$successful_compilations${NC}"
echo -e "Failed compilations: ${RED}$failed_compilations${NC}"

if [ $total_tests -gt 0 ]; then
    success_rate=$(echo "scale=2; $successful_compilations * 100 / $total_tests" | bc -l 2>/dev/null || echo "0")
    echo -e "Success rate: ${YELLOW}${success_rate}%${NC}"
fi

echo ""
echo -e "${BLUE}Results saved to:${NC} $RESULTS_FILE"
echo -e "${BLUE}Error logs saved to:${NC} $RESULTS_DIR/"

# Generate next steps
if [ $failed_compilations -gt 10 ]; then
    echo ""
    echo -e "${RED}⚠️  WARNING: High failure rate detected!${NC}"
    echo -e "${YELLOW}Recommended next steps:${NC}"
    echo "1. Review the most common error patterns"
    echo "2. Fix missing header dependencies first"
    echo "3. Create simplified test variants for complex tests"
    echo "4. Implement incremental build strategy"
fi

echo ""
echo -e "${GREEN}Phase 1 dependency analysis complete!${NC}"
echo -e "${BLUE}Next: Run 'make build-codegen-tests-robust' to attempt fixes${NC}" 
