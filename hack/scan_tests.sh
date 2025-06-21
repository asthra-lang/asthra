#!/bin/bash

# Script to scan the tests folder and generate a comprehensive list of all tests
# Usage: ./scan_tests.sh [options] [test_directory]
#        ./scan_tests.sh (uses 'tests' directory if no arguments)

# Function to display help
show_help() {
    echo "Usage: $0 [options] [test_directory]"
    echo ""
    echo "Scan and analyze test files in the Asthra programming language test suite."
    echo "If no directory is specified, scans the 'tests' directory."
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -o, --output FILE       Output results to file (default: stdout)"
    echo "  -f, --format FORMAT     Output format: text, csv, json, markdown (default: text)"
    echo "  -s, --stats             Include detailed statistics"
    echo "  -c, --categories-only   Show only test categories (directories)"
    echo "  -d, --detailed          Show detailed test function analysis"
    echo "  -e, --exclude DIR       Exclude specific test category (can be used multiple times)"
    echo "  -i, --include PATTERN   Include files matching pattern (can be used multiple times)"
    echo "  --no-color              Disable colored output"
    echo ""
    echo "Examples:"
    echo "  $0                              # Scan default 'tests' directory"
    echo "  $0 tests                        # Explicitly scan 'tests' directory"
    echo "  $0 -f csv -o test_report.csv    # Generate CSV report"
    echo "  $0 -s --detailed                # Show detailed statistics and analysis"
    echo "  $0 -c                           # Show only test categories"
    echo "  $0 -e integration -e performance # Exclude integration and performance tests"
    echo "  $0 -i 'test_*.c' --detailed     # Include only C test files with details"
    echo "  $0 -f markdown -o README_TESTS.md  # Generate markdown test documentation"
    echo ""
    echo "Output Formats:"
    echo "  text     - Human-readable text output (default)"
    echo "  csv      - Comma-separated values for spreadsheet analysis"
    echo "  json     - JSON format for programmatic processing"
    echo "  markdown - Markdown format for documentation"
    echo ""
    echo "Test Analysis Features:"
    echo "  - Test category organization (directories)"
    echo "  - Test file discovery and analysis"
    echo "  - Test function counting and naming patterns"
    echo "  - Build artifact detection (.o files)"
    echo "  - File size and line count analysis"
    echo "  - Test coverage statistics"
    exit 0
}

# Initialize variables
TEST_DIR=""
OUTPUT_FILE=""
OUTPUT_FORMAT="text"
SHOW_STATS=false
CATEGORIES_ONLY=false
DETAILED=false
USE_COLOR=true
EXCLUDE_DIRS=()
INCLUDE_PATTERNS=()

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            ;;
        -o|--output)
            if [[ -z "$2" ]]; then
                echo "Error: --output requires a filename"
                exit 1
            fi
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -f|--format)
            if [[ -z "$2" ]]; then
                echo "Error: --format requires a format (text, csv, json, markdown)"
                exit 1
            fi
            case "$2" in
                text|csv|json|markdown)
                    OUTPUT_FORMAT="$2"
                    ;;
                *)
                    echo "Error: Invalid format '$2'. Use: text, csv, json, markdown"
                    exit 1
                    ;;
            esac
            shift 2
            ;;
        -s|--stats)
            SHOW_STATS=true
            shift
            ;;
        -c|--categories-only)
            CATEGORIES_ONLY=true
            shift
            ;;
        -d|--detailed)
            DETAILED=true
            shift
            ;;
        -e|--exclude)
            if [[ -z "$2" ]]; then
                echo "Error: --exclude requires a directory name"
                exit 1
            fi
            EXCLUDE_DIRS+=("$2")
            shift 2
            ;;
        -i|--include)
            if [[ -z "$2" ]]; then
                echo "Error: --include requires a pattern"
                exit 1
            fi
            INCLUDE_PATTERNS+=("$2")
            shift 2
            ;;
        --no-color)
            USE_COLOR=false
            shift
            ;;
        -*)
            echo "Error: Unknown option '$1'"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
        *)
            if [[ -z "$TEST_DIR" ]]; then
                TEST_DIR="$1"
            else
                echo "Error: Multiple test directories not supported"
                exit 1
            fi
            shift
            ;;
    esac
done

# Set default test directory
if [[ -z "$TEST_DIR" ]]; then
    TEST_DIR="tests"
fi

# Check if test directory exists
if [[ ! -d "$TEST_DIR" ]]; then
    echo "Error: Test directory '$TEST_DIR' does not exist."
    exit 1
fi

# Color definitions
if [[ "$USE_COLOR" == true ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    PURPLE='\033[0;35m'
    CYAN='\033[0;36m'
    WHITE='\033[1;37m'
    NC='\033[0m' # No Color
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    PURPLE=''
    CYAN=''
    WHITE=''
    NC=''
fi

# Function to output content based on format
output() {
    if [[ -n "$OUTPUT_FILE" ]]; then
        echo "$1" >> "$OUTPUT_FILE"
    else
        echo -e "$1"
    fi
}

# Function to check if directory should be excluded
is_excluded() {
    local dir="$1"
    for exclude in "${EXCLUDE_DIRS[@]}"; do
        if [[ "$dir" == "$exclude" ]]; then
            return 0
        fi
    done
    return 1
}

# Function to check if file matches include patterns
matches_include() {
    local file="$1"
    
    # If no include patterns specified, include all test files
    if [[ ${#INCLUDE_PATTERNS[@]} -eq 0 ]]; then
        case "$file" in
            test_*.c|*_test.c|*.test.c|test_*.h|*_test.h|*.test.h)
                return 0
                ;;
            *)
                return 1
                ;;
        esac
    fi
    
    # Check against include patterns
    for pattern in "${INCLUDE_PATTERNS[@]}"; do
        if [[ "$file" == $pattern ]]; then
            return 0
        fi
    done
    return 1
}

# Function to count test functions in a C file
count_test_functions() {
    local file="$1"
    if [[ -f "$file" ]]; then
        # Count functions that start with "test_" or end with "_test"
        # Support multiple patterns for test framework standardization:
        # - Legacy: "void test_*" and "static int test_*" patterns
        # - Standardized: "AsthraTestResult test_*" and "static AsthraTestResult test_*" patterns
        # Only count function definitions (with {), not prototypes (with ;)
        local count=$(grep -c "^[[:space:]]*\(void\|static[[:space:]]\+int\|\(static[[:space:]]\+\)\?AsthraTestResult\)[[:space:]]\+\(test_.*\|.*_test\)([^)]*)[[:space:]]*{" "$file" 2>/dev/null)
        echo "${count:-0}"
    else
        echo "0"
    fi
}

# Function to get file stats
get_file_stats() {
    local file="$1"
    local size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
    local lines=$(wc -l < "$file" 2>/dev/null || echo "0")
    echo "$size:$lines"
}

# Function to format file size
format_size() {
    local size=$1
    if [[ $size -gt 1048576 ]]; then
        printf "%.1fMB" $(echo "$size" | awk '{print $1/1048576}')
    elif [[ $size -gt 1024 ]]; then
        printf "%.1fKB" $(echo "$size" | awk '{print $1/1024}')
    else
        echo "${size}B"
    fi
}

# Function to check if a directory is a single test category
is_single_category() {
    local dir="$1"
    # Check if the directory contains test files directly
    local test_file_count=$(find "$dir" -maxdepth 1 -name "test_*.c" -o -name "*_test.c" -o -name "*.test.c" | wc -l)
    local subdir_count=$(find "$dir" -maxdepth 1 -type d ! -path "$dir" | wc -l)
    
    # If it has test files and few/no subdirectories, it's likely a single category
    if [[ $test_file_count -gt 0 && $subdir_count -le 2 ]]; then
        return 0
    else
        return 1
    fi
}

# Initialize output file if specified
if [[ -n "$OUTPUT_FILE" ]]; then
    > "$OUTPUT_FILE"
fi

# Start processing
current_time=$(date '+%Y-%m-%d %H:%M:%S')

case "$OUTPUT_FORMAT" in
    "text")
        output "${WHITE}==================================================================${NC}"
        output "${WHITE}                  ASTHRA TEST SUITE ANALYSIS${NC}"
        output "${WHITE}==================================================================${NC}"
        output "Generated: $current_time"
        output "Test Directory: $TEST_DIR"
        output ""
        ;;
    "csv")
        output "category,test_file,test_functions,file_size_bytes,line_count,build_artifacts"
        ;;
    "json")
        output "{"
        output "  \"timestamp\": \"$current_time\","
        output "  \"test_directory\": \"$TEST_DIR\","
        output "  \"categories\": ["
        ;;
    "markdown")
        output "# Asthra Test Suite Analysis"
        output ""
        output "**Generated:** $current_time  "
        output "**Test Directory:** \`$TEST_DIR\`"
        output ""
        output "## Test Categories"
        output ""
        ;;
esac

# Collect test data
total_categories=0
total_test_files=0
total_test_functions=0
total_build_artifacts=0
total_file_size=0
total_line_count=0

# Process each category directory
first_json_category=true

# Check if TEST_DIR is a single test category or contains multiple categories
if is_single_category "$TEST_DIR"; then
    # Handle single category
    category=$(basename "$TEST_DIR")
    category_dir="$TEST_DIR"
    
    # Skip if excluded
    if ! is_excluded "$category"; then
        total_categories=$((total_categories + 1))
        
        # Initialize category stats
        category_test_files=0
        category_test_functions=0
        category_build_artifacts=0
        category_file_size=0
        category_line_count=0
        
        # For JSON format, handle comma separation
        if [[ "$OUTPUT_FORMAT" == "json" ]]; then
            output "    {"
            output "      \"name\": \"$category\","
            output "      \"files\": ["
        fi
        
        if [[ "$CATEGORIES_ONLY" == true ]]; then
            case "$OUTPUT_FORMAT" in
                "text")
                    output "${BLUE}📁 $category${NC}"
                    ;;
                "markdown")
                    output "- **$category**"
                    ;;
            esac
        else
            # Detailed category processing
            case "$OUTPUT_FORMAT" in
                "text")
                    output "${CYAN}📁 Category: $category${NC}"
                    ;;
                "markdown")
                    output "### $category"
                    output ""
                    ;;
            esac
            
            # Process files in category
            first_json_file=true
            for file in "$category_dir"/*; do
                if [[ -f "$file" ]]; then
                    filename=$(basename "$file")
                    
                    # Check if it's a test file
                    if matches_include "$filename"; then
                        category_test_files=$((category_test_files + 1))
                        total_test_files=$((total_test_files + 1))
                        
                        # Get file stats
                        stats=$(get_file_stats "$file")
                        size=$(echo "$stats" | cut -d: -f1)
                        lines=$(echo "$stats" | cut -d: -f2)
                        size=${size:-0}    # Default to 0 if empty
                        lines=${lines:-0}  # Default to 0 if empty
                        
                        category_file_size=$((category_file_size + size))
                        category_line_count=$((category_line_count + lines))
                        total_file_size=$((total_file_size + size))
                        total_line_count=$((total_line_count + lines))
                        
                        # Count test functions
                        test_funcs=$(count_test_functions "$file")
                        test_funcs=${test_funcs:-0}  # Default to 0 if empty
                        category_test_functions=$((category_test_functions + test_funcs))
                        total_test_functions=$((total_test_functions + test_funcs))
                        
                        # Check for build artifacts
                        build_artifact=""
                        if [[ -f "${file}.o" ]] || [[ -f "${file%.*}.o" ]]; then
                            build_artifact="yes"
                            category_build_artifacts=$((category_build_artifacts + 1))
                            total_build_artifacts=$((total_build_artifacts + 1))
                        else
                            build_artifact="no"
                        fi
                        
                        # Format size for display
                        size_display=$(format_size "$size")
                        
                        case "$OUTPUT_FORMAT" in
                            "text")
                                if [[ "$DETAILED" == true ]]; then
                                    output "  ${GREEN}🧪 $filename${NC}"
                                    output "     Functions: $test_funcs | Size: $size_display | Lines: $lines | Build artifact: $build_artifact"
                                else
                                    output "  ${GREEN}🧪 $filename${NC} (${test_funcs} functions)"
                                fi
                                ;;
                            "csv")
                                output "$category,$filename,$test_funcs,$size,$lines,$build_artifact"
                                ;;
                            "json")
                                if [[ "$first_json_file" == true ]]; then
                                    first_json_file=false
                                else
                                    output "        },"
                                fi
                                output "        {"
                                output "          \"name\": \"$filename\","
                                output "          \"test_functions\": $test_funcs,"
                                output "          \"file_size_bytes\": $size,"
                                output "          \"line_count\": $lines,"
                                output "          \"build_artifact\": \"$build_artifact\""
                                ;;
                            "markdown")
                                if [[ "$DETAILED" == true ]]; then
                                    output "- **$filename** - $test_funcs functions, $size_display, $lines lines, build artifact: $build_artifact"
                                else
                                    output "- **$filename** ($test_funcs functions)"
                                fi
                                ;;
                        esac
                    fi
                fi
            done
            
            # Close JSON file array if files were processed
            if [[ "$OUTPUT_FORMAT" == "json" && "$first_json_file" == false ]]; then
                output "        }"
            fi
            if [[ "$OUTPUT_FORMAT" == "json" ]]; then
                output "      ],"
                output "      \"stats\": {"
                output "        \"test_files\": $category_test_files,"
                output "        \"test_functions\": $category_test_functions,"
                output "        \"build_artifacts\": $category_build_artifacts,"
                output "        \"total_file_size_bytes\": $category_file_size,"
                output "        \"total_line_count\": $category_line_count"
                output "      }"
            fi
            
            # Category summary
            if [[ "$SHOW_STATS" == true && "$OUTPUT_FORMAT" == "text" ]]; then
                output "  ${YELLOW}📊 Category Stats: $category_test_files files, $category_test_functions functions${NC}"
                output ""
            elif [[ "$OUTPUT_FORMAT" == "markdown" ]]; then
                output ""
                output "**Stats:** $category_test_files test files, $category_test_functions test functions"
                output ""
            fi
        fi
    fi
else
    # Handle multiple categories (original logic)
    for category_dir in "$TEST_DIR"/*; do
        if [[ -d "$category_dir" ]]; then
            category=$(basename "$category_dir")
            
            # Skip if excluded
            if is_excluded "$category"; then
                continue
            fi
            
            total_categories=$((total_categories + 1))
            
            # Initialize category stats
            category_test_files=0
            category_test_functions=0
            category_build_artifacts=0
            category_file_size=0
            category_line_count=0
            
            # For JSON format, handle comma separation
            if [[ "$OUTPUT_FORMAT" == "json" ]]; then
                if [[ "$first_json_category" == true ]]; then
                    first_json_category=false
                else
                    output "    },"
                fi
                output "    {"
                output "      \"name\": \"$category\","
                output "      \"files\": ["
            fi
            
            if [[ "$CATEGORIES_ONLY" == true ]]; then
                case "$OUTPUT_FORMAT" in
                    "text")
                        output "${BLUE}📁 $category${NC}"
                        ;;
                    "markdown")
                        output "- **$category**"
                        ;;
                esac
            else
                # Detailed category processing
                case "$OUTPUT_FORMAT" in
                    "text")
                        output "${CYAN}📁 Category: $category${NC}"
                        ;;
                    "markdown")
                        output "### $category"
                        output ""
                        ;;
                esac
                
                # Process files in category
                first_json_file=true
                for file in "$category_dir"/*; do
                    if [[ -f "$file" ]]; then
                        filename=$(basename "$file")
                        
                        # Check if it's a test file
                        if matches_include "$filename"; then
                            category_test_files=$((category_test_files + 1))
                            total_test_files=$((total_test_files + 1))
                            
                            # Get file stats
                            stats=$(get_file_stats "$file")
                            size=$(echo "$stats" | cut -d: -f1)
                            lines=$(echo "$stats" | cut -d: -f2)
                            size=${size:-0}    # Default to 0 if empty
                            lines=${lines:-0}  # Default to 0 if empty
                            
                            category_file_size=$((category_file_size + size))
                            category_line_count=$((category_line_count + lines))
                            total_file_size=$((total_file_size + size))
                            total_line_count=$((total_line_count + lines))
                            
                            # Count test functions
                            test_funcs=$(count_test_functions "$file")
                            test_funcs=${test_funcs:-0}  # Default to 0 if empty
                            category_test_functions=$((category_test_functions + test_funcs))
                            total_test_functions=$((total_test_functions + test_funcs))
                            
                            # Check for build artifacts
                            build_artifact=""
                            if [[ -f "${file}.o" ]] || [[ -f "${file%.*}.o" ]]; then
                                build_artifact="yes"
                                category_build_artifacts=$((category_build_artifacts + 1))
                                total_build_artifacts=$((total_build_artifacts + 1))
                            else
                                build_artifact="no"
                            fi
                            
                            # Format size for display
                            size_display=$(format_size "$size")
                            
                            case "$OUTPUT_FORMAT" in
                                "text")
                                    if [[ "$DETAILED" == true ]]; then
                                        output "  ${GREEN}🧪 $filename${NC}"
                                        output "     Functions: $test_funcs | Size: $size_display | Lines: $lines | Build artifact: $build_artifact"
                                    else
                                        output "  ${GREEN}🧪 $filename${NC} (${test_funcs} functions)"
                                    fi
                                    ;;
                                "csv")
                                    output "$category,$filename,$test_funcs,$size,$lines,$build_artifact"
                                    ;;
                                "json")
                                    if [[ "$first_json_file" == true ]]; then
                                        first_json_file=false
                                    else
                                        output "        },"
                                    fi
                                    output "        {"
                                    output "          \"name\": \"$filename\","
                                    output "          \"test_functions\": $test_funcs,"
                                    output "          \"file_size_bytes\": $size,"
                                    output "          \"line_count\": $lines,"
                                    output "          \"build_artifact\": \"$build_artifact\""
                                    ;;
                                "markdown")
                                    if [[ "$DETAILED" == true ]]; then
                                        output "- **$filename** - $test_funcs functions, $size_display, $lines lines, build artifact: $build_artifact"
                                    else
                                        output "- **$filename** ($test_funcs functions)"
                                    fi
                                    ;;
                            esac
                        fi
                    fi
                done
                
                # Close JSON file array if files were processed
                if [[ "$OUTPUT_FORMAT" == "json" && "$first_json_file" == false ]]; then
                    output "        }"
                fi
                if [[ "$OUTPUT_FORMAT" == "json" ]]; then
                    output "      ],"
                    output "      \"stats\": {"
                    output "        \"test_files\": $category_test_files,"
                    output "        \"test_functions\": $category_test_functions,"
                    output "        \"build_artifacts\": $category_build_artifacts,"
                    output "        \"total_file_size_bytes\": $category_file_size,"
                    output "        \"total_line_count\": $category_line_count"
                    output "      }"
                fi
                
                # Category summary
                if [[ "$SHOW_STATS" == true && "$OUTPUT_FORMAT" == "text" ]]; then
                    output "  ${YELLOW}📊 Category Stats: $category_test_files files, $category_test_functions functions${NC}"
                    output ""
                elif [[ "$OUTPUT_FORMAT" == "markdown" ]]; then
                    output ""
                    output "**Stats:** $category_test_files test files, $category_test_functions test functions"
                    output ""
                fi
            fi
            
            # Store category stats (removed associative array usage for compatibility)
        fi
    done
fi

# Close JSON structure
if [[ "$OUTPUT_FORMAT" == "json" ]]; then
    if [[ $total_categories -gt 0 ]]; then
        output "    }"
    fi
    output "  ],"
    output "  \"summary\": {"
    output "    \"total_categories\": $total_categories,"
    output "    \"total_test_files\": $total_test_files,"
    output "    \"total_test_functions\": $total_test_functions,"
    output "    \"total_build_artifacts\": $total_build_artifacts,"
    output "    \"total_file_size_bytes\": $total_file_size,"
    output "    \"total_line_count\": $total_line_count"
    output "  }"
    output "}"
fi

# Overall statistics
if [[ "$SHOW_STATS" == true || "$OUTPUT_FORMAT" == "text" ]]; then
    case "$OUTPUT_FORMAT" in
        "text")
            output "${WHITE}==================================================================${NC}"
            output "${WHITE}                        SUMMARY STATISTICS${NC}"
            output "${WHITE}==================================================================${NC}"
            output "${PURPLE}📊 Test Categories:${NC} $total_categories"
            output "${PURPLE}📄 Test Files:${NC} $total_test_files"
            output "${PURPLE}🧪 Test Functions:${NC} $total_test_functions"
            output "${PURPLE}🔧 Build Artifacts:${NC} $total_build_artifacts"
            
            # Format total file size
            size_display=$(format_size "$total_file_size")
            output "${PURPLE}📏 Total Size:${NC} $size_display"
            output "${PURPLE}📝 Total Lines:${NC} $total_line_count"
            
            if [[ $total_test_files -gt 0 ]]; then
                avg_functions=$(echo "$total_test_functions $total_test_files" | awk '{printf "%.1f", $1/$2}')
                avg_lines=$(echo "$total_line_count $total_test_files" | awk '{printf "%.0f", $1/$2}')
                output "${PURPLE}📈 Avg Functions/File:${NC} $avg_functions"
                output "${PURPLE}📈 Avg Lines/File:${NC} $avg_lines"
            fi
            ;;
        "markdown")
            output "## Summary Statistics"
            output ""
            output "| Metric | Value |"
            output "|--------|-------|"
            output "| Test Categories | $total_categories |"
            output "| Test Files | $total_test_files |"
            output "| Test Functions | $total_test_functions |"
            output "| Build Artifacts | $total_build_artifacts |"
            output "| Total File Size | $(format_size "$total_file_size") |"
            output "| Total Lines | $total_line_count |"
            if [[ $total_test_files -gt 0 ]]; then
                output "| Avg Functions/File | $(echo "$total_test_functions $total_test_files" | awk '{printf "%.1f", $1/$2}') |"
                output "| Avg Lines/File | $(echo "$total_line_count $total_test_files" | awk '{printf "%.0f", $1/$2}') |"
            fi
            ;;
    esac
fi

# Final message
if [[ "$OUTPUT_FORMAT" == "text" ]]; then
    if [[ -n "$OUTPUT_FILE" ]]; then
        echo -e "${GREEN}✅ Test analysis complete! Results saved to: $OUTPUT_FILE${NC}"
    else
        output ""
        output "${GREEN}✅ Test analysis complete!${NC}"
    fi
fi

exit 0 
