#!/bin/bash
# Asthra Grammar Conformance Batch Fixer
# 
# Batch processing system with safety checks for grammar conformance fixes.
# Part of the Grammar Conformance Fix Plan v1.0.
#
# Copyright (c) 2024 Asthra Project
# Licensed under the terms specified in LICENSE

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BACKUP_DIR="grammar_fix_backups_$(date +%Y%m%d_%H%M%S)"
FIXER_SCRIPT="$SCRIPT_DIR/grammar_conformance_fixer.py"
CHECKER_SCRIPT="$SCRIPT_DIR/grammar_conformance_checker.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging
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

# Help function
show_help() {
    cat << EOF
Asthra Grammar Conformance Batch Fixer

USAGE:
    $0 [OPTIONS] COMMAND

COMMANDS:
    scan                    Scan for violations without fixing
    fix-critical           Fix critical violations (string interpolation)
    fix-high               Fix high priority violations (empty params/structs)
    fix-medium             Fix medium priority violations (auto types)
    fix-low                Fix low priority violations (return statements)
    fix-all                Fix all violations
    validate               Validate fixes by running tests
    rollback               Rollback to backup

OPTIONS:
    -d, --directory DIR    Target directory (default: current directory)
    -b, --backup-dir DIR   Backup directory (default: auto-generated)
    -n, --dry-run          Show what would be fixed without making changes
    -y, --yes              Skip confirmation prompts
    -v, --verbose          Show detailed output
    -h, --help             Show this help message

EXAMPLES:
    $0 scan                                    # Scan for violations
    $0 fix-critical --backup-dir backups/     # Fix critical issues with backup
    $0 fix-all --dry-run                      # Preview all fixes
    $0 validate                               # Validate current state

SAFETY FEATURES:
    - Automatic backup creation before any changes
    - Dry-run mode for preview
    - Confirmation prompts for destructive operations
    - Rollback capability
    - Comprehensive logging

EOF
}

# Check dependencies
check_dependencies() {
    log_info "Checking dependencies..."
    
    if ! command -v python3 &> /dev/null; then
        log_error "python3 is required but not installed"
        exit 1
    fi
    
    if [[ ! -f "$FIXER_SCRIPT" ]]; then
        log_error "Fixer script not found: $FIXER_SCRIPT"
        exit 1
    fi
    
    if [[ ! -f "$CHECKER_SCRIPT" ]]; then
        log_error "Checker script not found: $CHECKER_SCRIPT"
        exit 1
    fi
    
    log_success "Dependencies check passed"
}

# Create backup
create_backup() {
    local target_dir="$1"
    local backup_dir="$2"
    
    log_info "Creating backup in $backup_dir..."
    
    mkdir -p "$backup_dir"
    
    # Copy all .asthra files maintaining directory structure
    find "$target_dir" -name "*.asthra" -type f | while read -r file; do
        # Use portable relative path calculation
        rel_path="${file#$target_dir/}"
        backup_file="$backup_dir/$rel_path"
        backup_dir_path=$(dirname "$backup_file")
        
        mkdir -p "$backup_dir_path"
        cp "$file" "$backup_file"
    done
    
    # Create backup manifest
    cat > "$backup_dir/BACKUP_MANIFEST.txt" << EOF
Asthra Grammar Conformance Fix Backup
Created: $(date)
Source Directory: $target_dir
Backup Directory: $backup_dir
Script Version: 1.0

Files backed up:
$(find "$backup_dir" -name "*.asthra" -type f | wc -l) Asthra files

To restore:
    $0 rollback --backup-dir "$backup_dir"
EOF
    
    log_success "Backup created successfully"
}

# Scan for violations
scan_violations() {
    local target_dir="$1"
    local verbose="$2"
    
    log_info "Scanning for grammar violations in $target_dir..."
    
    local scan_args="--scan $target_dir"
    if [[ "$verbose" == "true" ]]; then
        scan_args="$scan_args --json violations_scan.json --csv violations_scan.csv"
    fi
    
    python3 "$CHECKER_SCRIPT" $scan_args
}

# Process category with confirmation
process_category() {
    local category="$1"
    local target_dir="$2"
    local backup_dir="$3"
    local dry_run="$4"
    local skip_confirm="$5"
    local verbose="$6"
    
    log_info "Processing $category violations..."
    
    # Check if there are violations by running a dry-run fix first
    local temp_output
    temp_output=$(python3 "$FIXER_SCRIPT" --scan "$target_dir" --fix "$category" --dry-run 2>/dev/null)
    local fix_count
    fix_count=$(echo "$temp_output" | grep "Total Fixes Applied:" | grep -o '[0-9]\+' || echo "0")
    
    if [[ "$fix_count" == "0" ]]; then
        log_info "No $category violations found"
        return 0
    fi
    
    echo
    log_warning "Found violations for category: $category"
    echo "Target directory: $target_dir"
    echo "Backup directory: $backup_dir"
    echo "Dry run mode: $dry_run"
    echo
    
    if [[ "$skip_confirm" != "true" && "$dry_run" != "true" ]]; then
        echo -n "Proceed with fixing $category violations? (y/N): "
        read -r confirm
        if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
            log_info "Skipping $category fixes"
            return 0
        fi
    fi
    
    # Build fixer arguments
    local fixer_args="--scan $target_dir --fix $category"
    
    if [[ "$dry_run" == "true" ]]; then
        fixer_args="$fixer_args --dry-run"
    else
        fixer_args="$fixer_args --backup-dir $backup_dir"
    fi
    
    if [[ "$verbose" == "true" ]]; then
        fixer_args="$fixer_args --verbose --report ${category}_fix_report.json"
    fi
    
    # Apply fixes
    log_info "Applying $category fixes..."
    python3 "$FIXER_SCRIPT" $fixer_args
    
    if [[ "$?" == "0" ]]; then
        log_success "$category fixes completed successfully"
    else
        log_error "$category fixes failed"
        return 1
    fi
}

# Validate fixes
validate_fixes() {
    local target_dir="$1"
    
    log_info "Validating fixes..."
    
    # Check for remaining violations
    log_info "Checking for remaining violations..."
    python3 "$CHECKER_SCRIPT" --scan "$target_dir" --strict
    
    if [[ "$?" == "0" ]]; then
        log_success "No violations found - validation passed"
    else
        log_warning "Some violations remain - check output above"
    fi
    
    # TODO: Add compilation check when compiler is available
    # log_info "Checking compilation..."
    # if command -v asthra-compiler &> /dev/null; then
    #     find "$target_dir" -name "*.asthra" -type f | while read -r file; do
    #         if ! asthra-compiler --syntax-check "$file" &> /dev/null; then
    #             log_error "Compilation failed for $file"
    #             return 1
    #         fi
    #     done
    #     log_success "All files compile successfully"
    # else
    #     log_warning "Asthra compiler not available - skipping compilation check"
    # fi
}

# Rollback changes
rollback_changes() {
    local backup_dir="$1"
    local target_dir="$2"
    
    if [[ ! -d "$backup_dir" ]]; then
        log_error "Backup directory not found: $backup_dir"
        exit 1
    fi
    
    if [[ ! -f "$backup_dir/BACKUP_MANIFEST.txt" ]]; then
        log_error "Invalid backup directory - manifest not found"
        exit 1
    fi
    
    log_info "Rolling back changes from $backup_dir..."
    
    # Restore files
    find "$backup_dir" -name "*.asthra" -type f | while read -r backup_file; do
        # Use portable relative path calculation
        rel_path="${backup_file#$backup_dir/}"
        target_file="$target_dir/$rel_path"
        
        if [[ -f "$target_file" ]]; then
            cp "$backup_file" "$target_file"
            log_info "Restored: $rel_path"
        fi
    done
    
    log_success "Rollback completed successfully"
}

# Main function
main() {
    local command=""
    local target_dir="$PROJECT_ROOT"
    local backup_dir=""
    local dry_run="false"
    local skip_confirm="false"
    local verbose="false"
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -d|--directory)
                target_dir="$2"
                shift 2
                ;;
            -b|--backup-dir)
                backup_dir="$2"
                shift 2
                ;;
            -n|--dry-run)
                dry_run="true"
                shift
                ;;
            -y|--yes)
                skip_confirm="true"
                shift
                ;;
            -v|--verbose)
                verbose="true"
                shift
                ;;
            -h|--help)
                show_help
                exit 0
                ;;
            scan|fix-critical|fix-high|fix-medium|fix-low|fix-all|validate|rollback)
                command="$1"
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    # Validate command
    if [[ -z "$command" ]]; then
        log_error "No command specified"
        show_help
        exit 1
    fi
    
    # Set default backup directory
    if [[ -z "$backup_dir" ]]; then
        backup_dir="$PROJECT_ROOT/$BACKUP_DIR"
    fi
    
    # Check dependencies
    check_dependencies
    
    # Execute command
    case "$command" in
        scan)
            scan_violations "$target_dir" "$verbose"
            ;;
        fix-critical)
            if [[ "$dry_run" != "true" ]]; then
                create_backup "$target_dir" "$backup_dir"
            fi
            process_category "string_interpolation" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            ;;
        fix-high)
            if [[ "$dry_run" != "true" ]]; then
                create_backup "$target_dir" "$backup_dir"
            fi
            process_category "empty_params" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            process_category "empty_struct_void" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            ;;
        fix-medium)
            if [[ "$dry_run" != "true" ]]; then
                create_backup "$target_dir" "$backup_dir"
            fi
            process_category "auto_type" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            ;;
        fix-low)
            if [[ "$dry_run" != "true" ]]; then
                create_backup "$target_dir" "$backup_dir"
            fi
            process_category "return_void" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            ;;
        fix-all)
            if [[ "$dry_run" != "true" ]]; then
                create_backup "$target_dir" "$backup_dir"
            fi
            process_category "string_interpolation" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            process_category "empty_params" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            process_category "empty_struct_void" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            process_category "auto_type" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            process_category "return_void" "$target_dir" "$backup_dir" "$dry_run" "$skip_confirm" "$verbose"
            ;;
        validate)
            validate_fixes "$target_dir"
            ;;
        rollback)
            rollback_changes "$backup_dir" "$target_dir"
            ;;
        *)
            log_error "Unknown command: $command"
            exit 1
            ;;
    esac
    
    log_success "Operation completed successfully"
}

# Run main function
main "$@" 
