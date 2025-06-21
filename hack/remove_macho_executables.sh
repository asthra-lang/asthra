#!/bin/bash

# Script to find and remove Mach-O 64-bit executable arm64 files
# Usage: ./remove_macho_executables.sh [options] [directory1] [directory2] [directory3] ...
#        ./remove_macho_executables.sh (uses current directory if no arguments)

# Function to display help
show_help() {
    echo "Usage: $0 [options] [directory1] [directory2] [directory3] ..."
    echo ""
    echo "Find and remove Mach-O 64-bit executable arm64 files from specified directories."
    echo "If no directories are specified, searches in the current directory."
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -n, --dry-run           Show files that would be removed without deleting them"
    echo "  -v, --verbose           Show detailed information about each file"
    echo "  -e, --exclude DIR       Exclude directory from search (can be used multiple times)"
    echo "  -p, --exclude-pattern PATTERN  Exclude files matching pattern (can be used multiple times)"
    echo "  --force                 Skip confirmation prompts (use with caution)"
    echo "  --list-only             Only list files, don't prompt for removal"
    echo ""
    echo "Examples:"
    echo "  $0                              # Find and remove Mach-O executables from current directory"
    echo "  $0 --dry-run .                  # Show what would be removed without deleting"
    echo "  $0 -e .git -e node_modules .    # Exclude .git and node_modules directories"
    echo "  $0 -p 'test_*' src              # Exclude files starting with 'test_'"
    echo "  $0 --verbose --list-only build/ # List all Mach-O executables with details"
    echo ""
    echo "What this script detects:"
    echo "  - Files identified by 'file' command as 'Mach-O 64-bit executable arm64'"
    echo "  - Compiled executables from C/C++, Rust, Go, and other native languages"
    echo "  - Build artifacts that shouldn't be committed to version control"
    echo ""
    echo "Common directories to exclude:"
    echo "  - Version control: .git, .svn, .hg"
    echo "  - Dependencies: node_modules, vendor, target"
    echo "  - IDE files: .vscode, .idea"
    echo ""
    echo "Safety features:"
    echo "  - Confirmation prompt before deletion (unless --force is used)"
    echo "  - Dry-run mode to preview changes"
    echo "  - Exclude patterns to protect important files"
    echo "  - Verbose mode for detailed analysis"
    exit 0
}

# Function to format bytes for human reading
format_bytes() {
    local bytes="$1"
    if [ "$bytes" -ge 1073741824 ]; then
        printf "%.1fGB" "$(echo "scale=1; $bytes / 1073741824" | bc -l)"
    elif [ "$bytes" -ge 1048576 ]; then
        printf "%.1fMB" "$(echo "scale=1; $bytes / 1048576" | bc -l)"
    elif [ "$bytes" -ge 1024 ]; then
        printf "%.1fKB" "$(echo "scale=1; $bytes / 1024" | bc -l)"
    else
        echo "${bytes}B"
    fi
}

# Function to check if file is a Mach-O 64-bit executable arm64
is_macho_executable() {
    local file="$1"
    
    # Check if file exists and is readable
    if [ ! -f "$file" ] || [ ! -r "$file" ]; then
        return 1
    fi
    
    # Use file command to check file type
    local file_output
    file_output=$(file -b "$file" 2>/dev/null)
    
    # Check for exact match: "Mach-O 64-bit executable arm64"
    if echo "$file_output" | grep -q "Mach-O 64-bit executable arm64"; then
        return 0
    fi
    
    return 1
}

# Function to check if file should be excluded
should_exclude_file() {
    local file="$1"
    local basename_file
    basename_file=$(basename "$file")
    
    # Check exclude patterns
    for pattern in "${EXCLUDE_PATTERNS[@]}"; do
        if [[ "$basename_file" == $pattern ]] || [[ "$file" == *"$pattern"* ]]; then
            return 0
        fi
    done
    
    return 1
}

# Function to check if directory should be excluded
should_exclude_dir() {
    local dir="$1"
    local basename_dir
    basename_dir=$(basename "$dir")
    
    # Check exclude directories
    for exclude_dir in "${EXCLUDE_DIRS[@]}"; do
        if [[ "$basename_dir" == "$exclude_dir" ]] || [[ "$dir" == *"/$exclude_dir" ]] || [[ "$dir" == "$exclude_dir" ]]; then
            return 0
        fi
    done
    
    return 1
}

# Initialize variables
SEARCH_DIRS=()
EXCLUDE_DIRS=()
EXCLUDE_PATTERNS=()
DRY_RUN=false
VERBOSE=false
FORCE=false
LIST_ONLY=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            ;;
        -n|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --force)
            FORCE=true
            shift
            ;;
        --list-only)
            LIST_ONLY=true
            shift
            ;;
        -e|--exclude)
            if [ -n "$2" ]; then
                EXCLUDE_DIRS+=("$2")
                shift 2
            else
                echo "Error: --exclude requires a directory name"
                exit 1
            fi
            ;;
        -p|--exclude-pattern)
            if [ -n "$2" ]; then
                EXCLUDE_PATTERNS+=("$2")
                shift 2
            else
                echo "Error: --exclude-pattern requires a pattern"
                exit 1
            fi
            ;;
        -*)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
        *)
            SEARCH_DIRS+=("$1")
            shift
            ;;
    esac
done

# If no directories specified, use current directory
if [ ${#SEARCH_DIRS[@]} -eq 0 ]; then
    SEARCH_DIRS=(".")
fi

# Check if required commands are available
if ! command -v file >/dev/null 2>&1; then
    echo "Error: 'file' command is required but not found"
    exit 1
fi

# Arrays to store found files
MACHO_FILES=()
TOTAL_SIZE=0

echo "Searching for Mach-O 64-bit executable arm64 files..."
echo "Search directories: ${SEARCH_DIRS[*]}"

if [ ${#EXCLUDE_DIRS[@]} -gt 0 ]; then
    echo "Excluding directories: ${EXCLUDE_DIRS[*]}"
fi

if [ ${#EXCLUDE_PATTERNS[@]} -gt 0 ]; then
    echo "Excluding patterns: ${EXCLUDE_PATTERNS[*]}"
fi

echo "=============================================="

# Find files
for search_dir in "${SEARCH_DIRS[@]}"; do
    if [ ! -d "$search_dir" ]; then
        echo "Warning: Directory '$search_dir' does not exist, skipping..."
        continue
    fi
    
    # Use find to get all files, then filter
    while IFS= read -r -d '' file; do
        # Check if file is in excluded directory
        skip_file=false
        for exclude_dir in "${EXCLUDE_DIRS[@]}"; do
            if [[ "$file" == *"/$exclude_dir/"* ]] || [[ "$file" == "./$exclude_dir/"* ]]; then
                skip_file=true
                break
            fi
        done
        
        if [ "$skip_file" = true ]; then
            if [ "$VERBOSE" = true ]; then
                echo "Skipping (excluded directory): $file"
            fi
            continue
        fi
        
        # Skip if file should be excluded by pattern
        if should_exclude_file "$file"; then
            if [ "$VERBOSE" = true ]; then
                echo "Skipping (excluded pattern): $file"
            fi
            continue
        fi
        
        # Check if it's a Mach-O executable
        if is_macho_executable "$file"; then
            MACHO_FILES+=("$file")
            
            # Get file size
            if [ -f "$file" ]; then
                file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "0")
                TOTAL_SIZE=$((TOTAL_SIZE + file_size))
                
                if [ "$VERBOSE" = true ]; then
                    formatted_size=$(format_bytes "$file_size")
                    echo "Found: $file (${formatted_size})"
                else
                    echo "Found: $file"
                fi
            fi
        fi
    done < <(find "$search_dir" -type f -print0 2>/dev/null)
done

echo "=============================================="

# Summary
if [ ${#MACHO_FILES[@]} -eq 0 ]; then
    echo "No Mach-O 64-bit executable arm64 files found."
    exit 0
fi

echo "Found ${#MACHO_FILES[@]} Mach-O 64-bit executable arm64 file(s)"
if [ "$TOTAL_SIZE" -gt 0 ]; then
    formatted_total=$(format_bytes "$TOTAL_SIZE")
    echo "Total size: $formatted_total"
fi

# If list-only mode, exit here
if [ "$LIST_ONLY" = true ]; then
    echo ""
    echo "List-only mode: No files will be removed."
    exit 0
fi

# If dry-run mode, show what would be removed
if [ "$DRY_RUN" = true ]; then
    echo ""
    echo "DRY RUN: The following files would be removed:"
    for file in "${MACHO_FILES[@]}"; do
        echo "  $file"
    done
    echo ""
    echo "Run without --dry-run to actually remove these files."
    exit 0
fi

# Confirmation prompt (unless --force is used)
if [ "$FORCE" != true ]; then
    echo ""
    echo "The following files will be PERMANENTLY DELETED:"
    for file in "${MACHO_FILES[@]}"; do
        echo "  $file"
    done
    echo ""
    read -p "Are you sure you want to delete these ${#MACHO_FILES[@]} file(s)? [y/N]: " -r
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Operation cancelled."
        exit 0
    fi
fi

# Remove files
echo ""
echo "Removing files..."
REMOVED_COUNT=0
FAILED_COUNT=0

for file in "${MACHO_FILES[@]}"; do
    if rm "$file" 2>/dev/null; then
        echo "Removed: $file"
        REMOVED_COUNT=$((REMOVED_COUNT + 1))
    else
        echo "Failed to remove: $file"
        FAILED_COUNT=$((FAILED_COUNT + 1))
    fi
done

echo "=============================================="
echo "Removal complete!"
echo "Successfully removed: $REMOVED_COUNT file(s)"
if [ "$FAILED_COUNT" -gt 0 ]; then
    echo "Failed to remove: $FAILED_COUNT file(s)"
fi

if [ "$TOTAL_SIZE" -gt 0 ]; then
    formatted_total=$(format_bytes "$TOTAL_SIZE")
    echo "Disk space freed: ~$formatted_total"
fi 
