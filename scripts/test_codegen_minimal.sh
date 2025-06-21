#!/bin/bash
# test_codegen_minimal.sh - Test codegen tests with minimal framework
# Part of Phase 2: Core Test Stabilization

set -e

# Configuration
CODEGEN_DIR="tests/codegen"
RESULTS_FILE="build/codegen_minimal_results.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create results directory
mkdir -p build

echo "Codegen Test Results with Minimal Framework" > "$RESULTS_FILE"
echo "===========================================" >> "$RESULTS_FILE"
echo "Generated: $(date)" >> "$RESULTS_FILE"
echo "Total test files found: $(find $CODEGEN_DIR -name 'test_*.c' | wc -l)" >> "$RESULTS_FILE"
echo "" >> "$RESULTS_FILE"

# Initialize counters
total_tests=0
successful_compilations=0
failed_compilations=0

echo -e "${BLUE}Testing Codegen with Minimal Framework...${NC}"
echo -e "${BLUE}=========================================${NC}"

# Test each test file with minimal framework
for test_file in "$CODEGEN_DIR"/test_*.c; do
    if [ ! -f "$test_file" ]; then
        continue
    fi
    
    test_name=$(basename "$test_file" .c)
    total_tests=$((total_tests + 1))
    
    echo -e "${YELLOW}Testing $test_name...${NC}"
    echo "Testing $test_name..." >> "$RESULTS_FILE"
    
    # Try to compile with minimal framework
    if gcc -c -I. -Isrc -Iruntime -Itests/framework \
           -DASTHRA_TEST_MODE=1 -DTEST_FRAMEWORK_MINIMAL=1 \
           -o "/tmp/${test_name}.o" "$test_file" 2>/dev/null; then
        echo -e "  ${GREEN}✅ SUCCESS${NC}"
        echo "  ✅ SUCCESS" >> "$RESULTS_FILE"
        successful_compilations=$((successful_compilations + 1))
        rm -f "/tmp/${test_name}.o"
    else
        echo -e "  ${RED}❌ FAILED${NC}"
        echo "  ❌ FAILED" >> "$RESULTS_FILE"
        failed_compilations=$((failed_compilations + 1))
    fi
    
    echo "" >> "$RESULTS_FILE"
done

# Generate summary statistics
echo "" >> "$RESULTS_FILE"
echo "=====================================" >> "$RESULTS_FILE"
echo "           SUMMARY STATISTICS         " >> "$RESULTS_FILE"
echo "=====================================" >> "$RESULTS_FILE"
echo "Total test files analyzed: $total_tests" >> "$RESULTS_FILE"
echo "Successful compilations: $successful_compilations" >> "$RESULTS_FILE"
echo "Failed compilations: $failed_compilations" >> "$RESULTS_FILE"

if [ $total_tests -gt 0 ]; then
    success_rate=$(echo "scale=2; $successful_compilations * 100 / $total_tests" | bc -l 2>/dev/null || echo "0")
    echo "Success rate: ${success_rate}%" >> "$RESULTS_FILE"
else
    echo "Success rate: 0%" >> "$RESULTS_FILE"
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

# Generate progress assessment
if [ $successful_compilations -ge 40 ]; then
    echo ""
    echo -e "${GREEN}🎉 EXCELLENT PROGRESS!${NC}"
    echo -e "${GREEN}Phase 2 target (60-80%) within reach!${NC}"
elif [ $successful_compilations -ge 25 ]; then
    echo ""
    echo -e "${YELLOW}📈 GOOD PROGRESS!${NC}"
    echo -e "${YELLOW}Approaching Phase 2 target range${NC}"
elif [ $successful_compilations -ge 15 ]; then
    echo ""
    echo -e "${BLUE}📊 STEADY PROGRESS${NC}"
    echo -e "${BLUE}Continue systematic fixes${NC}"
else
    echo ""
    echo -e "${RED}⚠️  MORE WORK NEEDED${NC}"
    echo -e "${RED}Focus on core framework issues${NC}"
fi

echo ""
echo -e "${GREEN}Minimal framework testing complete!${NC}" 
