#!/bin/bash

# =============================================================================
# Asthra Code Coverage Improvement Script
# =============================================================================

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
COVERAGE_DIR="$PROJECT_ROOT/coverage_output"
COVERAGE_DATA_DIR="$COVERAGE_DIR/data"
COVERAGE_HTML_DIR="$COVERAGE_DIR/html"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging function
log() {
    echo -e "${BLUE}[$(date +'%H:%M:%S')]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to fix common compilation issues
fix_compilation_issues() {
    log "Fixing common compilation issues..."
    
    # Fix missing errno.h includes
    find "$PROJECT_ROOT/tests" -name "*.c" -type f | while read -r file; do
        if grep -q "ENOMEM\|ERANGE\|EINVAL" "$file" && ! grep -q "#include <errno.h>" "$file"; then
            warning "Adding errno.h include to $file"
            # Use portable sed with backup file for macOS compatibility
            sed -i.bak '1i\
#include <errno.h>' "$file" && rm -f "${file}.bak"
        fi
    done
    
    # Fix missing function prototypes by adding static to function definitions
    find "$PROJECT_ROOT/tests" -name "*.c" -type f | while read -r file; do
        # Look for functions without prototypes and add static
        if grep -q "^void test_" "$file" || grep -q "^AsthraTestResult test_" "$file"; then
            warning "Adding static qualifiers to test functions in $file"
            # Use portable sed with backup file for macOS compatibility
            sed -i.bak 's/^void test_/static void test_/g' "$file" && rm -f "${file}.bak"
            sed -i.bak 's/^AsthraTestResult test_/static AsthraTestResult test_/g' "$file" && rm -f "${file}.bak"
        fi
    done
    
    success "Compilation fixes applied"
}

# Function to install required tools
install_coverage_tools() {
    log "Checking coverage tools..."
    
    if ! command_exists lcov; then
        warning "lcov not found, installing..."
        if [[ "$OSTYPE" == "darwin"* ]]; then
            if command_exists brew; then
                brew install lcov
            else
                error "Please install Homebrew first"
                exit 1
            fi
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            if command_exists apt-get; then
                sudo apt-get update && sudo apt-get install -y lcov
            elif command_exists yum; then
                sudo yum install -y lcov
            else
                error "Please install lcov manually"
                exit 1
            fi
        else
            warning "Unknown OS, please install lcov manually"
        fi
    fi
    
    if ! command_exists gcov; then
        error "gcov not found. Please install gcc or clang with gcov support"
        exit 1
    fi
    
    success "Coverage tools are available"
}

# Function to clean previous coverage data
clean_coverage_data() {
    log "Cleaning previous coverage data..."
    
    find "$PROJECT_ROOT" -name "*.gcda" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.gcno" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.gcov" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.profraw" -delete 2>/dev/null || true
    find "$PROJECT_ROOT" -name "*.profdata" -delete 2>/dev/null || true
    
    rm -rf "$COVERAGE_DIR"
    mkdir -p "$COVERAGE_DIR" "$COVERAGE_DATA_DIR" "$COVERAGE_HTML_DIR"
    
    success "Coverage data cleaned"
}

# Function to compile with coverage flags
compile_with_coverage() {
    log "Compiling with coverage instrumentation..."
    
    cd "$PROJECT_ROOT"
    
    # Use gcov-based coverage for maximum compatibility
    export CFLAGS="-O0 -g --coverage -fprofile-arcs -ftest-coverage -DDEBUG -DASTHRA_COVERAGE_ENABLED=1"
    export LDFLAGS="--coverage"
    
    # Build the project
    make clean || true
    if ! make all; then
        error "Compilation with coverage flags failed"
        return 1
    fi
    
    success "Coverage compilation completed"
}

# Function to run tests and collect coverage
run_tests_with_coverage() {
    log "Running tests with coverage collection..."
    
    cd "$PROJECT_ROOT"
    
    # Run all available tests
    local test_results=0
    
    # Run individual test categories that are most likely to work
    local working_categories=("types" "core" "memory" "basic" "parser" "io" "struct")
    
    for category in "${working_categories[@]}"; do
        log "Running tests for category: $category"
        if make "test-$category" 2>/dev/null; then
            success "Tests for $category completed"
        else
            warning "Tests for $category failed or not available"
            test_results=$((test_results + 1))
        fi
    done
    
    # Try to run any standalone test binaries
    if [ -d "$PROJECT_ROOT/bin" ]; then
        for test_binary in "$PROJECT_ROOT/bin"/test_*; do
            if [ -x "$test_binary" ]; then
                log "Running $test_binary"
                if "$test_binary"; then
                    success "$(basename "$test_binary") completed"
                else
                    warning "$(basename "$test_binary") failed"
                    test_results=$((test_results + 1))
                fi
            fi
        done
    fi
    
    if [ $test_results -eq 0 ]; then
        success "All tests completed"
    else
        warning "$test_results test categories had issues"
    fi
}

# Function to generate coverage reports
generate_coverage_reports() {
    log "Generating coverage reports..."
    
    cd "$PROJECT_ROOT"
    
    # Generate gcov files
    log "Generating gcov data..."
    find src -name "*.c" -exec gcov {} \; > "$COVERAGE_DATA_DIR/gcov_raw.txt" 2>&1 || true
    find tests -name "*.c" -exec gcov {} \; >> "$COVERAGE_DATA_DIR/gcov_raw.txt" 2>&1 || true
    
    # Generate lcov report
    log "Generating lcov report..."
    if command_exists lcov; then
        # Capture coverage data
        if lcov --capture --directory . --output-file "$COVERAGE_DATA_DIR/coverage.info" --ignore-errors source 2>/dev/null; then
            # Remove external files
            lcov --remove "$COVERAGE_DATA_DIR/coverage.info" '/usr/*' --output-file "$COVERAGE_DATA_DIR/coverage_filtered.info" 2>/dev/null || true
            lcov --remove "$COVERAGE_DATA_DIR/coverage_filtered.info" '*/tests/*' --output-file "$COVERAGE_DATA_DIR/coverage_src_only.info" 2>/dev/null || true
            
            # Generate HTML report
            if genhtml "$COVERAGE_DATA_DIR/coverage_src_only.info" --output-directory "$COVERAGE_HTML_DIR" --title "Asthra Coverage Report" 2>/dev/null; then
                success "HTML coverage report generated in $COVERAGE_HTML_DIR"
            else
                warning "Failed to generate HTML report, creating summary instead"
                create_summary_report
            fi
        else
            warning "lcov capture failed, creating basic summary"
            create_summary_report
        fi
    else
        warning "lcov not available, creating basic summary"
        create_summary_report
    fi
}

# Function to create a basic summary report
create_summary_report() {
    log "Creating coverage summary..."
    
    local summary_file="$COVERAGE_HTML_DIR/index.html"
    
    cat > "$summary_file" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>Asthra Coverage Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .coverage-table { border-collapse: collapse; width: 100%; }
        .coverage-table th, .coverage-table td { 
            text-align: left; padding: 8px; border: 1px solid #ddd; 
        }
        .coverage-table th { background-color: #f2f2f2; }
        .high-coverage { background-color: #d4edda; }
        .medium-coverage { background-color: #fff3cd; }
        .low-coverage { background-color: #f8d7da; }
        .metric { margin: 10px 0; padding: 10px; border-left: 4px solid #007bff; }
    </style>
</head>
<body>
    <h1>Asthra Code Coverage Report</h1>
    <p>Generated on: <script>document.write(new Date().toLocaleString());</script></p>
    
    <div class="metric">
        <h2>Coverage Metrics</h2>
        <p>This report shows the code coverage analysis for the Asthra programming language implementation.</p>
    </div>
    
    <h2>Source Files Coverage</h2>
    <table class="coverage-table">
        <tr>
            <th>File</th>
            <th>Lines</th>
            <th>Functions</th>
            <th>Branches</th>
            <th>Status</th>
        </tr>
EOF

    # Add coverage data for each source file
    local total_files=0
    local covered_files=0
    
    find "$PROJECT_ROOT/src" -name "*.c" -type f | while read -r file; do
        local filename=$(basename "$file")
        local lines="N/A"
        local functions="N/A"
        local branches="N/A"
        local status="Unknown"
        
        # Check if there's coverage data
        if [ -f "${file}.gcov" ]; then
            status="Covered"
            covered_files=$((covered_files + 1))
        elif find . -name "*.gcda" | grep -q "$(basename "$file" .c)"; then
            status="Instrumented"
        else
            status="Not covered"
        fi
        
        echo "        <tr>" >> "$summary_file"
        echo "            <td>$filename</td>" >> "$summary_file"
        echo "            <td>$lines</td>" >> "$summary_file"
        echo "            <td>$functions</td>" >> "$summary_file"
        echo "            <td>$branches</td>" >> "$summary_file"
        echo "            <td>$status</td>" >> "$summary_file"
        echo "        </tr>" >> "$summary_file"
        
        total_files=$((total_files + 1))
    done
    
    cat >> "$summary_file" << 'EOF'
    </table>
    
    <div class="metric">
        <h2>Coverage Summary</h2>
        <p>For detailed coverage analysis, please ensure all tests compile and run successfully.</p>
        <p>Raw coverage data is available in the data directory.</p>
    </div>
    
    <h2>Recommendations</h2>
    <ul>
        <li>Fix compilation errors in test files to increase coverage accuracy</li>
        <li>Add more unit tests for uncovered code paths</li>
        <li>Implement integration tests for complex scenarios</li>
        <li>Use property-based testing for edge cases</li>
    </ul>
    
</body>
</html>
EOF

    success "Coverage summary created at $summary_file"
}

# Function to analyze coverage and provide recommendations
analyze_coverage() {
    log "Analyzing coverage data..."
    
    local analysis_file="$COVERAGE_DATA_DIR/coverage_analysis.txt"
    
    {
        echo "Asthra Code Coverage Analysis"
        echo "============================"
        echo "Generated on: $(date)"
        echo ""
        
        echo "Source Files:"
        find "$PROJECT_ROOT/src" -name "*.c" -type f | wc -l | xargs echo "  Total C files:"
        
        echo ""
        echo "Test Files:"
        find "$PROJECT_ROOT/tests" -name "*.c" -type f | wc -l | xargs echo "  Total test files:"
        
        echo ""
        echo "Coverage Data Files:"
        find "$PROJECT_ROOT" -name "*.gcda" | wc -l | xargs echo "  GCDA files:"
        find "$PROJECT_ROOT" -name "*.gcno" | wc -l | xargs echo "  GCNO files:"
        find "$PROJECT_ROOT" -name "*.gcov" | wc -l | xargs echo "  GCOV files:"
        
        echo ""
        echo "Compilation Issues Found:"
        if [ -f "$COVERAGE_DATA_DIR/gcov_raw.txt" ]; then
            grep -i "error\|warning" "$COVERAGE_DATA_DIR/gcov_raw.txt" | head -10 || echo "  No major issues found"
        else
            echo "  No gcov output available"
        fi
        
        echo ""
        echo "Recommendations:"
        echo "1. Fix compilation errors in test files"
        echo "2. Add missing function prototypes"
        echo "3. Include missing header files"
        echo "4. Implement missing test functions"
        echo "5. Add more comprehensive test cases"
        
    } > "$analysis_file"
    
    success "Coverage analysis saved to $analysis_file"
}

# Main function
main() {
    log "Starting Asthra Code Coverage Improvement Process"
    
    # Check prerequisites
    install_coverage_tools
    
    # Fix common issues
    fix_compilation_issues
    
    # Clean and prepare
    clean_coverage_data
    
    # Compile with coverage
    if compile_with_coverage; then
        # Run tests
        run_tests_with_coverage
        
        # Generate reports
        generate_coverage_reports
        
        # Analyze results
        analyze_coverage
        
        success "Coverage improvement process completed!"
        log "Results available in: $COVERAGE_DIR"
        log "HTML report: $COVERAGE_HTML_DIR/index.html"
        log "Analysis: $COVERAGE_DATA_DIR/coverage_analysis.txt"
    else
        error "Failed to compile with coverage flags"
        exit 1
    fi
}

# Run the main function
main "$@" 
