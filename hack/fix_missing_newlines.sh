#!/bin/bash

# Script to check and fix missing trailing newlines in source files
# Usage: ./fix_missing_newlines.sh [directory1] [directory2] [directory3] ...
#        ./fix_missing_newlines.sh (uses current directory if no arguments)

# Function to display help
show_help() {
    echo "Usage: $0 [directory1] [directory2] [directory3] ..."
    echo ""
    echo "Check source files for missing trailing newlines and add them if needed."
    echo "If no directories are specified, searches in the current directory."
    echo ""
    echo "Options:"
    echo "  -h, --help    Show this help message"
    echo "  -n, --dry-run Show files that would be modified without making changes"
    echo ""
    echo "Examples:"
    echo "  $0                    # Fix files in current directory"
    echo "  $0 /path/to/src       # Fix files in one directory"
    echo "  $0 dir1 dir2 dir3     # Fix files in multiple directories"
    echo "  $0 --dry-run src/     # Show what would be fixed without making changes"
    echo ""
    echo "Supported file types include:"
    echo "  - Programming languages: C, C++, Python, Java, JavaScript, Go, Rust, etc."
    echo "  - Configuration files: JSON, YAML, TOML, XML, INI"
    echo "  - Build files: Makefile, CMake, Gradle"
    echo "  - Scripts: Shell, PowerShell, Batch"
    echo "  - Documentation: Markdown, LaTeX"
    exit 0
}

# Variables
DRY_RUN=false
SEARCH_DIRS=()

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
        -*)
            echo "Error: Unknown option $1"
            echo "Use -h or --help for usage information."
            exit 1
            ;;
        *)
            SEARCH_DIRS+=("$1")
            shift
            ;;
    esac
done

# Set default to current directory if no directories provided
if [ ${#SEARCH_DIRS[@]} -eq 0 ]; then
    SEARCH_DIRS=(".")
fi

# Check if all directories exist
for dir in "${SEARCH_DIRS[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "Error: Directory '$dir' does not exist."
        exit 1
    fi
done

if [ "$DRY_RUN" = true ]; then
    echo "DRY RUN MODE: No files will be modified"
    echo "============================================"
fi

echo "Checking for missing trailing newlines in:"
for dir in "${SEARCH_DIRS[@]}"; do
    echo "  - $dir"
done
echo "======================================================"

# Define file extensions for source files (same as find_largest_source_files.sh)
EXTENSIONS=(
    "*.c" "*.h"                    # C
    "*.cpp" "*.cxx" "*.cc" "*.hpp" # C++
    "*.py"                         # Python
    "*.java"                       # Java
    "*.js" "*.jsx" "*.ts" "*.tsx"  # JavaScript/TypeScript
    "*.go"                         # Go
    "*.rs"                         # Rust
    "*.rb"                         # Ruby
    "*.php"                        # PHP
    "*.cs"                         # C#
    "*.swift"                      # Swift
    "*.kt" "*.kts"                 # Kotlin
    "*.scala"                      # Scala
    "*.clj" "*.cljs"               # Clojure
    "*.hs"                         # Haskell
    "*.ml" "*.mli"                 # OCaml
    "*.fs" "*.fsx"                 # F#
    "*.r" "*.R"                    # R
    "*.m"                          # Matlab/Objective-C
    "*.pl" "*.pm"                  # Perl
    "*.lua"                        # Lua
    "*.sh" "*.bash" "*.zsh"        # Shell scripts
    "*.ps1"                        # PowerShell
    "*.bat" "*.cmd"                # Batch files
    "*.vim"                        # Vim script
    "*.el"                         # Emacs Lisp
    "*.sql"                        # SQL
    "*.json"                       # JSON
    "*.yaml" "*.yml"               # YAML
    "*.toml"                       # TOML
    "*.xml"                        # XML
    "*.html" "*.htm"               # HTML
    "*.css" "*.scss" "*.sass"      # CSS/SASS
    "*.md" "*.markdown"            # Markdown
    "*.tex"                        # LaTeX
    "*.dockerfile" "Dockerfile*"    # Docker
    "Makefile*" "*.mk"             # Make
    "*.cmake" "CMakeLists.txt"     # CMake
    "*.gradle"                     # Gradle
    "*.ini" "*.cfg" "*.conf"       # Configuration files
    "*.proto"                      # Protocol Buffers
    "*.thrift"                     # Thrift
    "*.graphql" "*.gql"            # GraphQL
)

# Build find command with all extensions
FIND_ARGS=()
for i in "${!EXTENSIONS[@]}"; do
    if [ $i -eq 0 ]; then
        FIND_ARGS+=("(" "-name" "${EXTENSIONS[$i]}")
    else
        FIND_ARGS+=("-o" "-name" "${EXTENSIONS[$i]}")
    fi
done
FIND_ARGS+=(")")

# Function to check if file ends with newline
check_and_fix_newline() {
    local file="$1"
    
    # Skip empty files
    if [ ! -s "$file" ]; then
        return 0
    fi
    
    # Check if file ends with newline
    # Use tail to get the last byte and check if it's a newline
    if [ "$(tail -c1 "$file" 2>/dev/null)" != "" ]; then
        # File doesn't end with newline
        if [ "$DRY_RUN" = true ]; then
            echo "WOULD FIX: $file"
        else
            echo "FIXING: $file"
            # Add newline to the end of the file
            echo "" >> "$file"
        fi
        return 1
    fi
    return 0
}

# Counters
total_files=0
fixed_files=0

# Process files
echo "Scanning files..."
for dir in "${SEARCH_DIRS[@]}"; do
    find "$dir" -type f "${FIND_ARGS[@]}" 2>/dev/null | while read -r file; do
        # Skip binary files and files that might cause issues
        if file "$file" | grep -q "text\|empty"; then
            total_files=$((total_files + 1))
            
            if ! check_and_fix_newline "$file"; then
                fixed_files=$((fixed_files + 1))
            fi
        fi
    done
done

# Note: Due to subshell limitations, we need to recount for the summary
echo ""
echo "Summary:"
echo "========"

# Count total files again for accurate summary
total_count=0
for dir in "${SEARCH_DIRS[@]}"; do
    while IFS= read -r -d '' file; do
        if file "$file" | grep -q "text\|empty"; then
            total_count=$((total_count + 1))
        fi
    done < <(find "$dir" -type f "${FIND_ARGS[@]}" -print0 2>/dev/null)
done

# Count files that need fixing
needs_fixing=0
for dir in "${SEARCH_DIRS[@]}"; do
    while IFS= read -r -d '' file; do
        if file "$file" | grep -q "text\|empty"; then
            if [ -s "$file" ] && [ "$(tail -c1 "$file" 2>/dev/null)" != "" ]; then
                needs_fixing=$((needs_fixing + 1))
            fi
        fi
    done < <(find "$dir" -type f "${FIND_ARGS[@]}" -print0 2>/dev/null)
done

echo "Total source files scanned: $total_count"
if [ "$DRY_RUN" = true ]; then
    echo "Files that would be fixed: $needs_fixing"
    if [ $needs_fixing -gt 0 ]; then
        echo ""
        echo "Run without --dry-run to actually fix the files."
    fi
else
    echo "Files fixed: $needs_fixing"
fi

if [ $needs_fixing -eq 0 ]; then
    echo "All files already have proper trailing newlines! ✅"
fi 
