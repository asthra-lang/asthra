#!/bin/bash
# fix_codegen_tests.sh - Systematic Codegen Test Fixes
# Part of Phase 2: Core Test Stabilization

set -e

# Configuration
CODEGEN_DIR="tests/codegen"
BACKUP_DIR="build/codegen_backups"
RESULTS_DIR="build/codegen_analysis"
LOG_FILE="build/codegen_fix_log.txt"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Create necessary directories
mkdir -p "$BACKUP_DIR"
mkdir -p "$RESULTS_DIR"
mkdir -p build

echo "Codegen Test Systematic Fixes" > "$LOG_FILE"
echo "=============================" >> "$LOG_FILE"
echo "Started: $(date)" >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

# Initialize counters
total_tests=0
tests_fixed=0
tests_failed=0
tests_skipped=0

echo -e "${BLUE}Starting Systematic Codegen Test Fixes...${NC}"
echo -e "${BLUE}=========================================${NC}"

# Function to backup a test file
backup_test_file() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    local backup_file="$BACKUP_DIR/${test_name}_$(date +%Y%m%d_%H%M%S).c"
    
    cp "$test_file" "$backup_file"
    echo "  Backed up to: $backup_file" >> "$LOG_FILE"
}

# Function to fix test framework redefinition issues
fix_framework_redefinition() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    
    echo -e "${YELLOW}Fixing framework redefinition in $test_name...${NC}"
    echo "Fixing framework redefinition in $test_name..." >> "$LOG_FILE"
    
    # Backup original file
    backup_test_file "$test_file"
    
    # Create temporary file for modifications
    local temp_file="/tmp/${test_name}_fix.c"
    
    # Apply fixes:
    # 1. Add TEST_FRAMEWORK_MINIMAL define at the top
    # 2. Fix include paths if needed
    # 3. Update function signatures for minimal framework compatibility
    
    cat > "$temp_file" << 'EOF'
/**
 * Fixed for minimal test framework compatibility
 */
#define TEST_FRAMEWORK_MINIMAL 1

EOF
    
    # Add the rest of the file, skipping any existing framework defines
    grep -v "^#define TEST_FRAMEWORK_MINIMAL" "$test_file" >> "$temp_file"
    
    # Replace the original file
    mv "$temp_file" "$test_file"
    
    echo "  Applied minimal framework fix" >> "$LOG_FILE"
}

# Function to fix missing header dependencies
fix_header_dependencies() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    
    echo -e "${YELLOW}Fixing header dependencies in $test_name...${NC}"
    echo "Fixing header dependencies in $test_name..." >> "$LOG_FILE"
    
    # Common header fixes based on error analysis
    sed -i.bak \
        -e 's|#include "elf_symbol_manager.h"|#include "elf_writer_symbols.h"|g' \
        -e 's|#include "elf_relocation_manager.h"|#include "elf_writer_relocations.h"|g' \
        -e 's|#include "../src/|#include "src/|g' \
        "$test_file"
    
    echo "  Applied header dependency fixes" >> "$LOG_FILE"
}

# Function to fix function prototypes
fix_function_prototypes() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    
    echo -e "${YELLOW}Fixing function prototypes in $test_name...${NC}"
    echo "Fixing function prototypes in $test_name..." >> "$LOG_FILE"
    
    # Fix common function prototype issues
    sed -i.bak2 \
        -e 's|int main()|int main(void)|g' \
        -e 's|void main()|int main(void)|g' \
        -e 's|static int test_\([^(]*\)()|static AsthraTestResult test_\1(void)|g' \
        "$test_file"
    
    echo "  Applied function prototype fixes" >> "$LOG_FILE"
}

# Function to test if a file compiles after fixes
test_compilation() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    local error_file="/tmp/${test_name}_test_errors.txt"
    
    echo -e "${BLUE}Testing compilation of $test_name...${NC}"
    
    if gcc -c -I. -Isrc -Iruntime -Itests/framework \
           -DASTHRA_TEST_MODE=1 -DTEST_FRAMEWORK_MINIMAL=1 \
           -o "/tmp/${test_name}_test.o" "$test_file" 2>"$error_file"; then
        echo -e "  ${GREEN}✅ Compilation: SUCCESS${NC}"
        echo "  ✅ Compilation: SUCCESS after fixes" >> "$LOG_FILE"
        rm -f "/tmp/${test_name}_test.o"
        return 0
    else
        echo -e "  ${RED}❌ Compilation: STILL FAILED${NC}"
        echo "  ❌ Compilation: STILL FAILED after fixes" >> "$LOG_FILE"
        echo "  Remaining errors:" >> "$LOG_FILE"
        sed 's/^/    /' "$error_file" >> "$LOG_FILE"
        return 1
    fi
}

# Function to apply all fixes to a test file
apply_all_fixes() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    
    echo -e "${YELLOW}Applying all fixes to $test_name...${NC}"
    echo "Applying all fixes to $test_name..." >> "$LOG_FILE"
    
    # Apply fixes in order
    fix_framework_redefinition "$test_file"
    fix_header_dependencies "$test_file"
    fix_function_prototypes "$test_file"
    
    # Test if fixes worked
    if test_compilation "$test_file"; then
        tests_fixed=$((tests_fixed + 1))
        echo -e "  ${GREEN}✅ $test_name: FIXED SUCCESSFULLY${NC}"
        echo "  ✅ $test_name: FIXED SUCCESSFULLY" >> "$LOG_FILE"
        return 0
    else
        tests_failed=$((tests_failed + 1))
        echo -e "  ${RED}❌ $test_name: FIXES FAILED${NC}"
        echo "  ❌ $test_name: FIXES FAILED" >> "$LOG_FILE"
        return 1
    fi
}

# Main fix loop - process all failing test files
echo "Processing failing codegen test files..." >> "$LOG_FILE"
echo "" >> "$LOG_FILE"

# Get list of failing tests from previous analysis
failing_tests=(
    "test_arithmetic_expressions"
    "test_assignment_statements"
    "test_bitwise_expressions"
    "test_call_expressions"
    "test_comparison_expressions"
    "test_control_flow_analysis"
    "test_control_flow_statements"
    "test_data_flow_analysis"
    "test_expression_generation_suite"
    "test_function_call_statements"
    "test_function_calls"
    "test_function_generation"
    "test_logical_expressions"
    "test_method_mangling"
    "test_optimization_passes"
    "test_return_statements"
    "test_statement_generation_suite"
    "test_struct_access"
    "test_struct_assignment"
    "test_struct_complex_types"
    "test_struct_generation_suite"
    "test_struct_instantiation"
    "test_struct_methods"
    "test_unary_expressions"
    "test_variable_declarations"
)

# Process each failing test
for test_name in "${failing_tests[@]}"; do
    test_file="$CODEGEN_DIR/${test_name}.c"
    
    if [ ! -f "$test_file" ]; then
        echo -e "${YELLOW}⚠️  Skipping $test_name (file not found)${NC}"
        echo "⚠️  Skipping $test_name (file not found)" >> "$LOG_FILE"
        tests_skipped=$((tests_skipped + 1))
        continue
    fi
    
    total_tests=$((total_tests + 1))
    
    echo ""
    echo -e "${BLUE}Processing $test_name...${NC}"
    echo "Processing $test_name..." >> "$LOG_FILE"
    
    apply_all_fixes "$test_file"
    
    echo "" >> "$LOG_FILE"
done

# Generate summary
echo "" >> "$LOG_FILE"
echo "=====================================" >> "$LOG_FILE"
echo "           FIX SUMMARY                " >> "$LOG_FILE"
echo "=====================================" >> "$LOG_FILE"
echo "Total tests processed: $total_tests" >> "$LOG_FILE"
echo "Tests fixed successfully: $tests_fixed" >> "$LOG_FILE"
echo "Tests still failing: $tests_failed" >> "$LOG_FILE"
echo "Tests skipped: $tests_skipped" >> "$LOG_FILE"

if [ $total_tests -gt 0 ]; then
    fix_rate=$(echo "scale=2; $tests_fixed * 100 / $total_tests" | bc -l 2>/dev/null || echo "0")
    echo "Fix success rate: ${fix_rate}%" >> "$LOG_FILE"
fi

echo "Completed: $(date)" >> "$LOG_FILE"

# Display summary to console
echo ""
echo -e "${BLUE}=====================================${NC}"
echo -e "${BLUE}         FIX SUMMARY                 ${NC}"
echo -e "${BLUE}=====================================${NC}"
echo -e "Total tests processed: ${YELLOW}$total_tests${NC}"
echo -e "Tests fixed successfully: ${GREEN}$tests_fixed${NC}"
echo -e "Tests still failing: ${RED}$tests_failed${NC}"
echo -e "Tests skipped: ${YELLOW}$tests_skipped${NC}"

if [ $total_tests -gt 0 ]; then
    fix_rate=$(echo "scale=2; $tests_fixed * 100 / $total_tests" | bc -l 2>/dev/null || echo "0")
    echo -e "Fix success rate: ${YELLOW}${fix_rate}%${NC}"
fi

echo ""
echo -e "${BLUE}Fix log saved to:${NC} $LOG_FILE"
echo -e "${BLUE}Backups saved to:${NC} $BACKUP_DIR/"

# Provide next steps
if [ $tests_fixed -gt 0 ]; then
    echo ""
    echo -e "${GREEN}✅ Successfully fixed $tests_fixed tests!${NC}"
    echo -e "${BLUE}Next steps:${NC}"
    echo "1. Run './scripts/analyze_codegen_tests.sh' to verify improvements"
    echo "2. Run 'make build-codegen-tests-robust' to test build system"
    echo "3. Continue with remaining failing tests if any"
fi

if [ $tests_failed -gt 0 ]; then
    echo ""
    echo -e "${RED}⚠️  $tests_failed tests still need manual fixes${NC}"
    echo -e "${YELLOW}Recommended actions:${NC}"
    echo "1. Review error logs for remaining issues"
    echo "2. Check for complex dependency problems"
    echo "3. Consider creating simplified test variants"
fi

echo ""
echo -e "${GREEN}Phase 2 systematic fixes complete!${NC}" 
