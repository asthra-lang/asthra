#!/bin/bash

# Script to find the largest 5 source files based on line count
# Usage: ./find_largest_source_files.sh [options] [directory1] [directory2] [directory3] ...
#        ./find_largest_source_files.sh (uses current directory if no arguments)

# Function to display help
show_help() {
    echo "Usage: $0 [options] [directory1] [directory2] [directory3] ..."
    echo ""
    echo "Find the largest 5 source files by line count in specified directories."
    echo "If no directories are specified, searches in the current directory."
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -e, --exclude DIR       Exclude directory from search (can be used multiple times)"
    echo "  -p, --exclude-pattern PATTERN  Exclude files matching pattern (can be used multiple times)"
    echo ""
    echo "Examples:"
    echo "  $0                              # Search in current directory"
    echo "  $0 /path/to/src                 # Search in one directory"
    echo "  $0 dir1 dir2 dir3               # Search in multiple directories"
    echo "  $0 -e node_modules -e .git .    # Exclude node_modules and .git from current dir"
    echo "  $0 --exclude build --exclude dist src  # Exclude build and dist directories"
    echo "  $0 -p '*.md' -p '*.txt' .       # Exclude Markdown and text files"
    echo "  $0 --exclude-pattern '*.test.*' src     # Exclude test files"
    echo "  $0 -e build -p '*.min.js' -p '*.d.ts' src  # Combine directory and pattern exclusions"
    echo ""
    echo "Common directories to exclude:"
    echo "  - node_modules, .git, .svn, .hg"
    echo "  - build, dist, target, out, bin"
    echo "  - .vscode, .idea, .DS_Store"
    echo "  - __pycache__, .pytest_cache"
    echo ""
    echo "Common file patterns to exclude:"
    echo "  - Documentation: '*.md', '*.txt', '*.rst'"
    echo "  - Generated files: '*.min.js', '*.d.ts', '*.generated.*'"
    echo "  - Test files: '*.test.*', '*.spec.*', '*_test.*'"
    echo "  - Temporary files: '*.tmp', '*.bak', '*.swp'"
    echo ""
    echo "Supported file types include:"
    echo "  - Programming languages: C, C++, Python, Java, JavaScript, Go, Rust, etc."
    echo "  - Configuration files: JSON, YAML, TOML, XML, INI"
    echo "  - Build files: Makefile, CMake, Gradle"
    echo "  - Scripts: Shell, PowerShell, Batch"
    echo "  - Documentation: Markdown, LaTeX"
    exit 0
}

# Initialize arrays
SEARCH_DIRS=()
EXCLUDE_DIRS=()
EXCLUDE_PATTERNS=()

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            ;;
        -e|--exclude)
            if [[ -z "$2" ]]; then
                echo "Error: --exclude requires a directory name"
                exit 1
            fi
            EXCLUDE_DIRS+=("$2")
            shift 2
            ;;
        -p|--exclude-pattern)
            if [[ -z "$2" ]]; then
                echo "Error: --exclude-pattern requires a pattern"
                exit 1
            fi
            EXCLUDE_PATTERNS+=("$2")
            shift 2
            ;;
        -*)
            echo "Error: Unknown option '$1'"
            echo "Use -h or --help for usage information"
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

echo "Searching for largest source files in:"
for dir in "${SEARCH_DIRS[@]}"; do
    echo "  - $dir"
done

if [ ${#EXCLUDE_DIRS[@]} -gt 0 ]; then
    echo ""
    echo "Excluding directories:"
    for dir in "${EXCLUDE_DIRS[@]}"; do
        echo "  - $dir"
    done
fi

if [ ${#EXCLUDE_PATTERNS[@]} -gt 0 ]; then
    echo ""
    echo "Excluding file patterns:"
    for pattern in "${EXCLUDE_PATTERNS[@]}"; do
        echo "  - $pattern"
    done
fi

echo "======================================================"

# Define file extensions for source files
# Programming languages and configuration files
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

# Add exclude conditions for directories
if [ ${#EXCLUDE_DIRS[@]} -gt 0 ]; then
    FIND_ARGS+=("(")
    for i in "${!EXCLUDE_DIRS[@]}"; do
        exclude_dir="${EXCLUDE_DIRS[$i]}"
        if [ $i -eq 0 ]; then
            FIND_ARGS+=("-path" "*/${exclude_dir}" "-o" "-path" "*/${exclude_dir}/*")
        else
            FIND_ARGS+=("-o" "-path" "*/${exclude_dir}" "-o" "-path" "*/${exclude_dir}/*")
        fi
    done
    FIND_ARGS+=(")" "-prune" "-o")
fi

# Add file extension conditions
FIND_ARGS+=("(")
for i in "${!EXTENSIONS[@]}"; do
    if [ $i -eq 0 ]; then
        FIND_ARGS+=("-name" "${EXTENSIONS[$i]}")
    else
        FIND_ARGS+=("-o" "-name" "${EXTENSIONS[$i]}")
    fi
done
FIND_ARGS+=(")" "-type" "f")

# Add exclude pattern conditions
if [ ${#EXCLUDE_PATTERNS[@]} -gt 0 ]; then
    FIND_ARGS+=("!")
    FIND_ARGS+=("(")
    for i in "${!EXCLUDE_PATTERNS[@]}"; do
        pattern="${EXCLUDE_PATTERNS[$i]}"
        if [ $i -eq 0 ]; then
            FIND_ARGS+=("-name" "$pattern")
        else
            FIND_ARGS+=("-o" "-name" "$pattern")
        fi
    done
    FIND_ARGS+=(")")
fi

FIND_ARGS+=("-print")

# Create temporary file to store results
TEMP_FILE=$(mktemp)

# Find all source files and count lines
echo "Scanning files and counting lines..."
for dir in "${SEARCH_DIRS[@]}"; do
    find "$dir" "${FIND_ARGS[@]}" 2>/dev/null | while read -r file; do
        # Skip binary files and files that might cause issues
        if file "$file" | grep -q "text\|empty"; then
            line_count=$(wc -l < "$file" 2>/dev/null || echo 0)
            echo "$line_count $file"
        fi
    done
done | sort -nr > "$TEMP_FILE"

# Check if any files were found
if [ ! -s "$TEMP_FILE" ]; then
    echo "No source files found in the specified directories."
    rm "$TEMP_FILE"
    exit 0
fi

# Display the top 5 largest files
echo ""
echo "Top 5 largest source files by line count:"
echo "========================================="
printf "%-10s %s\n" "Lines" "File"
echo "----------------------------------------"

head -5 "$TEMP_FILE" | while read -r line_count file_path; do
    # Display path logic:
    # - For single directory (not current dir): show relative path from that directory
    # - For current directory only: remove leading ./
    # - For multiple directories: preserve directory context
    if [ ${#SEARCH_DIRS[@]} -eq 1 ] && [[ "${SEARCH_DIRS[0]}" != "." ]]; then
        # Single directory (not current dir) - show relative path
        dir="${SEARCH_DIRS[0]}"
        if [[ "$file_path" == "$dir"/* ]]; then
            rel_path="${file_path#$dir/}"
        else
            rel_path="$file_path"
        fi
    elif [ ${#SEARCH_DIRS[@]} -eq 1 ] && [[ "${SEARCH_DIRS[0]}" == "." ]]; then
        # Current directory only - remove leading ./
        if [[ "$file_path" == ./* ]]; then
            rel_path="${file_path#./}"
        else
            rel_path="$file_path"
        fi
    else
        # Multiple directories - keep directory context
        # Remove leading ./ if it exists, but preserve other directory prefixes
        if [[ "$file_path" == ./* ]]; then
            rel_path="${file_path#./}"
        else
            rel_path="$file_path"
        fi
    fi
    printf "%-10s %s\n" "$line_count" "$rel_path"
done

# Show total files scanned
total_files=$(wc -l < "$TEMP_FILE")
echo ""
echo "Total source files scanned: $total_files"

# Cleanup
rm "$TEMP_FILE" 
