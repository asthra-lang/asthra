#!/bin/bash

# =============================================================================
# Simple Asthra Code Coverage Script
# =============================================================================

set -e

PROJECT_ROOT="$(dirname "$(dirname "$(realpath "$0")")")"
COVERAGE_DIR="$PROJECT_ROOT/coverage_output"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${BLUE}[INFO]${NC} $1"; }
success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }

log "Starting simple coverage analysis for Asthra"

# Clean previous coverage data
log "Cleaning previous coverage data..."
find "$PROJECT_ROOT" -name "*.gcda" -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.gcno" -delete 2>/dev/null || true
rm -rf "$COVERAGE_DIR"
mkdir -p "$COVERAGE_DIR/data" "$COVERAGE_DIR/html"

cd "$PROJECT_ROOT"

# Try to run coverage with the existing makefile
log "Running coverage with existing makefile..."
if make coverage-html-gcov; then
    success "Coverage generated successfully!"
else
    warning "Standard coverage failed, trying alternative approach..."
    
    # Try manual approach
    log "Attempting manual coverage collection..."
    
    # Compile with coverage flags
    export CFLAGS="-O0 -g --coverage -DDEBUG"
    export LDFLAGS="--coverage"
    
    if make clean && make all; then
        log "Compilation with coverage flags successful"
        
        # Run some basic tests
        log "Running available tests..."
        make test-types || warning "test-types failed"
        make test-core || warning "test-core failed"
        make test-basic || warning "test-basic failed"
        
        # Generate coverage data
        log "Generating coverage data..."
        find src -name "*.c" -exec gcov {} \; > "$COVERAGE_DIR/data/gcov_output.txt" 2>&1 || true
        
        # Try to generate HTML report
        if command -v lcov >/dev/null 2>&1; then
            lcov --capture --directory . --output-file "$COVERAGE_DIR/data/coverage.info" --ignore-errors source || true
            lcov --remove "$COVERAGE_DIR/data/coverage.info" '/usr/*' --output-file "$COVERAGE_DIR/data/filtered.info" || true
            genhtml "$COVERAGE_DIR/data/filtered.info" --output-directory "$COVERAGE_DIR/html" --title "Asthra Coverage" || true
        fi
        
        success "Coverage analysis completed"
    else
        warning "Compilation failed, generating summary only"
    fi
fi

# Create a simple analysis report
cat > "$COVERAGE_DIR/summary.txt" << EOF
Asthra Code Coverage Summary
===========================
Generated: $(date)

Source Files: $(find src -name "*.c" | wc -l)
Test Files: $(find tests -name "*.c" | wc -l)
Coverage Data Files: $(find . -name "*.gcda" 2>/dev/null | wc -l)

$(if [ -f "$COVERAGE_DIR/html/index.html" ]; then
    echo "HTML Report: Available at $COVERAGE_DIR/html/index.html"
else
    echo "HTML Report: Not generated (check for compilation errors)"
fi)

Recommendations:
1. Fix compilation errors in test files
2. Ensure all test dependencies are available
3. Add more unit tests for better coverage
4. Use 'make coverage-html-gcov' for full coverage reports
EOF

log "Coverage summary saved to $COVERAGE_DIR/summary.txt"

if [ -f "$COVERAGE_DIR/html/index.html" ]; then
    success "Coverage report available at: $COVERAGE_DIR/html/index.html"
else
    warning "HTML report not generated. Check compilation errors and dependencies."
fi

success "Simple coverage analysis completed!" 
