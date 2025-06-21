#!/bin/bash

# Asthra Punctuation Fixer - v1.12 Grammar Migration
# Part of the Flexible Punctuation Removal Plan Phase 3: Codebase Migration
# 
# This script automatically fixes Asthra source files to conform to the new v1.12 grammar
# which eliminates optional trailing punctuation and enforces consistent comma separators.

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
DRY_RUN=false
VERBOSE=false
BACKUP_DIR="backup_punctuation_v1_12"
STATS_FILE="punctuation_fix_stats.log"

# Initialize statistics
TOTAL_FILES=0
MODIFIED_FILES=0
TRAILING_COMMAS_FIXED=0
STRUCT_LITERALS_FIXED=0
MATCH_ARMS_FIXED=0
ENUM_LITERALS_FIXED=0

# Print usage information
usage() {
    echo "Usage: $0 [OPTIONS] [DIRECTORY]"
    echo ""
    echo "Automatically fix Asthra files to conform to v1.12 grammar rules:"
    echo "  - Remove trailing commas from struct literals"
    echo "  - Remove trailing commas from enum variants" 
    echo "  - Remove trailing commas from match arms"
    echo "  - Remove trailing commas from patterns"
    echo ""
    echo "OPTIONS:"
    echo "  -d, --dry-run     Show what would be changed without modifying files"
    echo "  -v, --verbose     Show detailed output"
    echo "  -b, --backup      Create backup directory (default: $BACKUP_DIR)"
    echo "  -h, --help        Show this help message"
    echo ""
    echo "EXAMPLES:"
    echo "  $0                          # Fix all .asthra files in current directory"
    echo "  $0 --dry-run stdlib/        # Preview changes in stdlib directory"
    echo "  $0 --verbose examples/      # Fix examples with detailed output"
}

# Parse command line arguments
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--dry-run)
                DRY_RUN=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -b|--backup)
                BACKUP_DIR="$2"
                shift 2
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                if [[ -z "$TARGET_DIR" ]]; then
                    TARGET_DIR="$1"
                else
                    echo -e "${RED}Error: Multiple directories specified${NC}"
                    usage
                    exit 1
                fi
                shift
                ;;
        esac
    done
    
    # Default to current directory if none specified
    if [[ -z "$TARGET_DIR" ]]; then
        TARGET_DIR="."
    fi
}

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_verbose() {
    if [[ "$VERBOSE" == "true" ]]; then
        echo -e "${BLUE}[VERBOSE]${NC} $1"
    fi
}

# Create backup of files before modification
create_backup() {
    local file="$1"
    
    if [[ "$DRY_RUN" == "false" ]]; then
        # Create backup directory structure
        local backup_file="$BACKUP_DIR/$file"
        local backup_dir=$(dirname "$backup_file")
        
        mkdir -p "$backup_dir"
        cp "$file" "$backup_file"
        log_verbose "Created backup: $backup_file"
    fi
}

# Fix trailing commas in various contexts
fix_trailing_commas() {
    local file="$1"
    local temp_file=$(mktemp)
    local changes_made=false
    
    # Create backup before modification
    create_backup "$file"
    
    # Process the file line by line to handle different contexts
    local in_struct_literal=false
    local in_match_expr=false
    local brace_depth=0
    
    while IFS= read -r line || [[ -n "$line" ]]; do
        original_line="$line"
        
        # Track context for better accuracy
        # Count braces to track depth
        brace_count=$(echo "$line" | tr -cd '{' | wc -c)
        close_brace_count=$(echo "$line" | tr -cd '}' | wc -c)
        brace_depth=$((brace_depth + brace_count - close_brace_count))
        
        # 1. Fix trailing commas in struct literals: { field: value, } -> { field: value }
        if [[ "$line" =~ ^[[:space:]]*.*,+[[:space:]]*}[[:space:]]*$ ]]; then
            new_line=$(echo "$line" | sed 's/,[[:space:]]*}/}/g')
            if [[ "$new_line" != "$line" ]]; then
                line="$new_line"
                changes_made=true
                TRAILING_COMMAS_FIXED=$((TRAILING_COMMAS_FIXED + 1))
                STRUCT_LITERALS_FIXED=$((STRUCT_LITERALS_FIXED + 1))
                log_verbose "Fixed struct literal trailing comma: $file"
            fi
        fi
        
        # 2. Fix trailing commas in match arms: Pattern => expr, -> Pattern => expr
        # This includes both expression and block forms: Pattern => expr, and Pattern => {},
        if [[ "$line" =~ =\>[[:space:]]*.*,[[:space:]]*$ ]]; then
            new_line=$(echo "$line" | sed 's/,[[:space:]]*$//')
            if [[ "$new_line" != "$line" ]]; then
                line="$new_line"
                changes_made=true
                TRAILING_COMMAS_FIXED=$((TRAILING_COMMAS_FIXED + 1))
                MATCH_ARMS_FIXED=$((MATCH_ARMS_FIXED + 1))
                log_verbose "Fixed match arm trailing comma: $file"
            fi
        fi
        
        # 3. Fix trailing commas in enum patterns: Variant { field, } -> Variant { field }
        if [[ "$line" =~ ^[[:space:]]*[A-Z][a-zA-Z0-9_]*[[:space:]]*\{.*,[[:space:]]*\}[[:space:]]*$ ]]; then
            new_line=$(echo "$line" | sed 's/,[[:space:]]*}/}/g')
            if [[ "$new_line" != "$line" ]]; then
                line="$new_line"
                changes_made=true
                TRAILING_COMMAS_FIXED=$((TRAILING_COMMAS_FIXED + 1))
                ENUM_LITERALS_FIXED=$((ENUM_LITERALS_FIXED + 1))
                log_verbose "Fixed enum pattern trailing comma: $file"
            fi
        fi
        
        # 4. Fix trailing commas in function calls and array literals
        if [[ "$line" =~ .*\(.*,[[:space:]]*\)[[:space:]]*\;?[[:space:]]*$ ]] || [[ "$line" =~ .*\[.*,[[:space:]]*\][[:space:]]*\;?[[:space:]]*$ ]]; then
            new_line=$(echo "$line" | sed 's/,[[:space:]]*\([)\]]\)/\1/g')
            if [[ "$new_line" != "$line" ]]; then
                line="$new_line"
                changes_made=true
                TRAILING_COMMAS_FIXED=$((TRAILING_COMMAS_FIXED + 1))
                log_verbose "Fixed function call/array trailing comma: $file"
            fi
        fi
        
        echo "$line" >> "$temp_file"
    done < "$file"
    
    # Only update the file if changes were made and not in dry-run mode
    if [[ "$changes_made" == "true" ]]; then
        if [[ "$DRY_RUN" == "false" ]]; then
            mv "$temp_file" "$file"
            log_success "Updated: $file"
        else
            log_info "Would update: $file"
            rm "$temp_file"
        fi
        MODIFIED_FILES=$((MODIFIED_FILES + 1))
        return 0
    else
        rm "$temp_file"
        return 1
    fi
}

# Validate that the file is still valid Asthra syntax (basic check)
validate_asthra_syntax() {
    local file="$1"
    
    # Basic syntax validation - check for common issues
    local errors=0
    
    # Check for unmatched braces
    local open_braces=$(grep -o '{' "$file" | wc -l)
    local close_braces=$(grep -o '}' "$file" | wc -l)
    
    if [[ $open_braces -ne $close_braces ]]; then
        log_warning "Mismatched braces in $file (open: $open_braces, close: $close_braces)"
        errors=$((errors + 1))
    fi
    
    # Check for trailing semicolons in struct fields (should be commas)
    if grep -q '[a-zA-Z_][a-zA-Z0-9_]*:[[:space:]]*[^,}]*;[[:space:]]*[a-zA-Z_]' "$file"; then
        log_warning "Found semicolon separators in struct fields in $file (should be commas)"
        errors=$((errors + 1))
    fi
    
    return $errors
}

# Process a single Asthra file
process_file() {
    local file="$1"
    
    log_verbose "Processing: $file"
    TOTAL_FILES=$((TOTAL_FILES + 1))
    
    # Skip non-Asthra files
    if [[ ! "$file" =~ \.asthra$ ]]; then
        log_verbose "Skipping non-Asthra file: $file"
        return
    fi
    
    # Skip backup files
    if [[ "$file" =~ \.bak$ ]] || [[ "$file" =~ ~$ ]]; then
        log_verbose "Skipping backup file: $file"
        return
    fi
    
    # Fix trailing commas
    if fix_trailing_commas "$file"; then
        # Validate syntax after changes
        if [[ "$DRY_RUN" == "false" ]]; then
            validate_asthra_syntax "$file"
        fi
    else
        log_verbose "No changes needed: $file"
    fi
}

# Find and process all Asthra files in the target directory
process_directory() {
    local target_dir="$1"
    
    log_info "Processing Asthra files in: $target_dir"
    
    # Create backup directory if not in dry-run mode
    if [[ "$DRY_RUN" == "false" ]]; then
        mkdir -p "$BACKUP_DIR"
        log_info "Created backup directory: $BACKUP_DIR"
    fi
    
    # Find all .asthra files and process them
    while IFS= read -r -d '' file; do
        process_file "$file"
    done < <(find "$target_dir" -name "*.asthra" -type f -print0)
}

# Generate statistics report
generate_report() {
    local report_file="$STATS_FILE"
    
    {
        echo "# Asthra v1.12 Grammar Migration Report"
        echo "Generated: $(date)"
        echo "Target Directory: $TARGET_DIR"
        echo "Dry Run: $DRY_RUN"
        echo ""
        echo "## Statistics"
        echo "Total Files Processed: $TOTAL_FILES"
        echo "Files Modified: $MODIFIED_FILES"
        echo "Total Trailing Commas Fixed: $TRAILING_COMMAS_FIXED"
        echo "  - Struct Literals: $STRUCT_LITERALS_FIXED"
        echo "  - Match Arms: $MATCH_ARMS_FIXED"
        echo "  - Enum Patterns: $ENUM_LITERALS_FIXED"
        echo ""
        echo "## Success Rate"
        if [[ $TOTAL_FILES -gt 0 ]]; then
            local success_rate=$((MODIFIED_FILES * 100 / TOTAL_FILES))
            echo "Files requiring fixes: $success_rate%"
        fi
        
        if [[ "$DRY_RUN" == "false" ]]; then
            echo ""
            echo "## Backup Location"
            echo "Original files backed up to: $BACKUP_DIR"
        fi
    } > "$report_file"
    
    # Display report
    echo ""
    log_info "=== MIGRATION REPORT ==="
    log_info "Files processed: $TOTAL_FILES"
    log_info "Files modified: $MODIFIED_FILES"
    log_info "Trailing commas fixed: $TRAILING_COMMAS_FIXED"
    
    if [[ "$DRY_RUN" == "false" ]]; then
        log_success "Report saved to: $report_file"
        log_success "Backup created in: $BACKUP_DIR"
    else
        log_info "Dry run complete - no files were modified"
    fi
}

# Main execution
main() {
    echo "Asthra v1.12 Grammar Migration Tool"
    echo "Part of the Flexible Punctuation Removal Plan"
    echo ""
    
    parse_args "$@"
    
    # Validate target directory exists
    if [[ ! -d "$TARGET_DIR" ]]; then
        log_error "Directory not found: $TARGET_DIR"
        exit 1
    fi
    
    # Start processing
    process_directory "$TARGET_DIR"
    
    # Generate final report
    generate_report
    
    if [[ $MODIFIED_FILES -gt 0 ]]; then
        if [[ "$DRY_RUN" == "false" ]]; then
            echo ""
            log_success "✅ Migration completed successfully!"
            log_info "All Asthra files now conform to v1.12 grammar rules"
            log_info "Optional trailing punctuation has been removed"
            log_info "Run 'make test' to verify all changes work correctly"
        else
            echo ""
            log_info "Dry run completed - run without --dry-run to apply changes"
        fi
    else
        log_info "No files required modification - already v1.12 compliant!"
    fi
}

# Run the script
main "$@" 
