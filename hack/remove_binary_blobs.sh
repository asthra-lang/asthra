#!/bin/bash

# Script to remove binary blobs from specified directories
# Usage: ./remove_binary_blobs.sh [options] [directory1] [directory2] [directory3] ...
#        ./remove_binary_blobs.sh (uses current directory if no arguments)

# Function to display help
show_help() {
    echo "Usage: $0 [options] [directory1] [directory2] [directory3] ..."
    echo ""
    echo "Remove binary blobs from specified directories."
    echo "If no directories are specified, searches in the current directory."
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -n, --dry-run           Show files that would be removed without deleting them"
    echo "  -v, --verbose           Show detailed information about each file"
    echo "  -s, --size-threshold SIZE  Only remove files larger than SIZE (default: 1MB)"
    echo "                             SIZE can be specified with suffixes: K, M, G"
    echo "                             Examples: 500K, 10M, 1G"
    echo "  -e, --exclude DIR       Exclude directory from search (can be used multiple times)"
    echo "  -p, --exclude-pattern PATTERN  Exclude files matching pattern (can be used multiple times)"
    echo "  -i, --include-pattern PATTERN  Only consider files matching pattern (can be used multiple times)"
    echo "  --force                 Skip confirmation prompts (use with caution)"
    echo ""
    echo "Examples:"
    echo "  $0                              # Remove binary blobs from current directory"
    echo "  $0 --dry-run .                  # Show what would be removed without deleting"
    echo "  $0 -s 5M /path/to/project       # Remove binary files larger than 5MB"
    echo "  $0 -e .git -e node_modules .    # Exclude .git and node_modules directories"
    echo "  $0 -p '*.log' -p '*.tmp' src    # Exclude log and temp files from removal"
    echo "  $0 -i '*.exe' -i '*.dll' .      # Only consider .exe and .dll files"
    echo "  $0 --verbose --size-threshold 100K build/  # Verbose mode, 100KB threshold"
    echo ""
    echo "Common binary file types that will be detected:"
    echo "  - Executables: .exe, .bin, .out, .app"
    echo "  - Libraries: .so, .dll, .dylib, .a, .lib"
    echo "  - Archives: .zip, .tar, .gz, .bz2, .xz, .7z, .rar"
    echo "  - Images: .jpg, .jpeg, .png, .gif, .bmp, .tiff, .ico"
    echo "  - Audio/Video: .mp3, .mp4, .avi, .mov, .wav, .flac"
    echo "  - Documents: .pdf, .doc, .docx, .xls, .xlsx, .ppt, .pptx"
    echo "  - Object files: .o, .obj, .pyc, .class"
    echo "  - Database files: .db, .sqlite, .mdb"
    echo ""
    echo "Common directories to exclude:"
    echo "  - Version control: .git, .svn, .hg"
    echo "  - Dependencies: node_modules, vendor, target"
    echo "  - Build outputs: build, dist, out, bin"
    echo "  - IDE files: .vscode, .idea"
    echo ""
    echo "Safety features:"
    echo "  - Confirmation prompt before deletion (unless --force is used)"
    echo "  - Dry-run mode to preview changes"
    echo "  - Size threshold to avoid removing small files"
    echo "  - Exclude patterns to protect important files"
    echo "  - Verbose mode for detailed analysis"
    exit 0
}

# Function to convert size string to bytes
size_to_bytes() {
    local size_str="$1"
    local number="${size_str%[KMG]}"
    local suffix="${size_str: -1}"
    
    # If no suffix, assume bytes
    if [[ "$suffix" =~ [0-9] ]]; then
        echo "$size_str"
        return
    fi
    
    case "$suffix" in
        K|k) echo $((number * 1024)) ;;
        M|m) echo $((number * 1024 * 1024)) ;;
        G|g) echo $((number * 1024 * 1024 * 1024)) ;;
        *) echo "$number" ;;
    esac
}

# Function to format bytes for human reading
format_bytes() {
    local bytes="$1"
    if [ "$bytes" -ge 1073741824 ]; then
        echo "$(( bytes / 1073741824 ))GB"
    elif [ "$bytes" -ge 1048576 ]; then
        echo "$(( bytes / 1048576 ))MB"
    elif [ "$bytes" -ge 1024 ]; then
        echo "$(( bytes / 1024 ))KB"
    else
        echo "${bytes}B"
    fi
}

# Function to check if file is binary
is_binary_file() {
    local file="$1"
    
    # Check file command output
    local file_output
    file_output=$(file -b "$file" 2>/dev/null)
    
    # Common binary file indicators
    if echo "$file_output" | grep -qi -E "(executable|binary|archive|compressed|image|audio|video|database|object|library)"; then
        return 0
    fi
    
    # Check for specific binary file extensions
    local extension="${file##*.}"
    extension=$(echo "$extension" | tr '[:upper:]' '[:lower:]')
    
    case "$extension" in
        # Executables
        exe|bin|out|app|msi|deb|rpm|dmg|pkg)
            return 0 ;;
        # Libraries
        so|dll|dylib|a|lib|framework)
            return 0 ;;
        # Archives
        zip|tar|gz|bz2|xz|7z|rar|lzma|z|tgz|tbz2|txz)
            return 0 ;;
        # Images
        jpg|jpeg|png|gif|bmp|tiff|tif|ico|svg|webp|raw|cr2|nef|arw)
            return 0 ;;
        # Audio/Video
        mp3|mp4|avi|mov|mkv|flv|wmv|wav|flac|aac|ogg|m4a|m4v|webm)
            return 0 ;;
        # Documents
        pdf|doc|docx|xls|xlsx|ppt|pptx|odt|ods|odp)
            return 0 ;;
        # Object files
        o|obj|pyc|pyo|class|jar|war|ear)
            return 0 ;;
        # Database files
        db|sqlite|sqlite3|mdb|accdb)
            return 0 ;;
        # Font files
        ttf|otf|woff|woff2|eot)
            return 0 ;;
        # Other binary formats
        iso|img|vdi|vmdk|qcow2|swp|tmp|cache)
            return 0 ;;
    esac
    
    # Check if file contains null bytes (strong indicator of binary)
    if [ -f "$file" ] && [ -r "$file" ]; then
        if head -c 8192 "$file" 2>/dev/null | grep -q $'\0'; then
            return 0
        fi
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
    
    # Check include patterns (if any specified)
    if [ ${#INCLUDE_PATTERNS[@]} -gt 0 ]; then
        local matches_include=false
        for pattern in "${INCLUDE_PATTERNS[@]}"; do
            if [[ "$basename_file" == $pattern ]] || [[ "$file" == *"$pattern"* ]]; then
                matches_include=true
                break
            fi
        done
        if [ "$matches_include" = false ]; then
            return 0
        fi
    fi
    
    return 1
}

# Initialize variables
SEARCH_DIRS=()
EXCLUDE_DIRS=()
EXCLUDE_PATTERNS=()
INCLUDE_PATTERNS=()
DRY_RUN=false
VERBOSE=false
FORCE=false
SIZE_THRESHOLD_BYTES=1048576  # 1MB default

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
        -s|--size-threshold)
            if [[ -z "$2" ]]; then
                echo "Error: --size-threshold requires a size value"
                exit 1
            fi
            SIZE_THRESHOLD_BYTES=$(size_to_bytes "$2")
            if ! [[ "$SIZE_THRESHOLD_BYTES" =~ ^[0-9]+$ ]]; then
                echo "Error: Invalid size format '$2'"
                echo "Use formats like: 100K, 5M, 1G"
                exit 1
            fi
            shift 2
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
        -i|--include-pattern)
            if [[ -z "$2" ]]; then
                echo "Error: --include-pattern requires a pattern"
                exit 1
            fi
            INCLUDE_PATTERNS+=("$2")
            shift 2
            ;;
        --force)
            FORCE=true
            shift
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

# Display configuration
if [ "$DRY_RUN" = true ]; then
    echo "DRY RUN MODE: No files will be deleted"
    echo "============================================"
fi

echo "Searching for binary blobs in:"
for dir in "${SEARCH_DIRS[@]}"; do
    echo "  - $dir"
done

echo ""
echo "Configuration:"
echo "  Size threshold: $(format_bytes $SIZE_THRESHOLD_BYTES)"
echo "  Verbose mode: $VERBOSE"
echo "  Force mode: $FORCE"

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

if [ ${#INCLUDE_PATTERNS[@]} -gt 0 ]; then
    echo ""
    echo "Including only file patterns:"
    for pattern in "${INCLUDE_PATTERNS[@]}"; do
        echo "  - $pattern"
    done
fi

echo "======================================================"

# Build find command with exclude conditions
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

# Add file type condition
FIND_ARGS+=("-type" "f" "-print")

# Counters
total_files_scanned=0
binary_files_found=0
files_to_remove=0
total_size_to_remove=0

# Arrays to store files for removal
declare -a FILES_TO_REMOVE
declare -a FILE_SIZES

echo "Scanning for binary files..."

# Find and analyze files
for dir in "${SEARCH_DIRS[@]}"; do
    while IFS= read -r -d '' file; do
        total_files_scanned=$((total_files_scanned + 1))
        
        # Skip if file should be excluded
        if should_exclude_file "$file"; then
            continue
        fi
        
        # Check if file is binary
        if is_binary_file "$file"; then
            binary_files_found=$((binary_files_found + 1))
            
            # Get file size
            file_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo 0)
            
            # Check size threshold
            if [ "$file_size" -ge "$SIZE_THRESHOLD_BYTES" ]; then
                FILES_TO_REMOVE+=("$file")
                FILE_SIZES+=("$file_size")
                files_to_remove=$((files_to_remove + 1))
                total_size_to_remove=$((total_size_to_remove + file_size))
                
                if [ "$VERBOSE" = true ]; then
                    echo "BINARY: $file ($(format_bytes $file_size))"
                fi
            elif [ "$VERBOSE" = true ]; then
                echo "SKIP (too small): $file ($(format_bytes $file_size))"
            fi
        fi
    done < <(find "$dir" "${FIND_ARGS[@]}" -print0 2>/dev/null)
done

echo ""
echo "Scan Results:"
echo "============="
echo "Total files scanned: $total_files_scanned"
echo "Binary files found: $binary_files_found"
echo "Files meeting removal criteria: $files_to_remove"
echo "Total size to be removed: $(format_bytes $total_size_to_remove)"

if [ $files_to_remove -eq 0 ]; then
    echo ""
    echo "No binary files found that meet the removal criteria."
    exit 0
fi

echo ""
echo "Files to be removed:"
echo "==================="
for i in "${!FILES_TO_REMOVE[@]}"; do
    file="${FILES_TO_REMOVE[$i]}"
    size="${FILE_SIZES[$i]}"
    echo "  $file ($(format_bytes $size))"
done

# Confirmation prompt (unless in dry-run or force mode)
if [ "$DRY_RUN" = false ] && [ "$FORCE" = false ]; then
    echo ""
    echo "WARNING: This will permanently delete $files_to_remove files totaling $(format_bytes $total_size_to_remove)."
    echo "This action cannot be undone!"
    echo ""
    read -p "Are you sure you want to proceed? (yes/no): " -r
    if [[ ! $REPLY =~ ^[Yy][Ee][Ss]$ ]]; then
        echo "Operation cancelled."
        exit 0
    fi
fi

# Remove files
if [ "$DRY_RUN" = false ]; then
    echo ""
    echo "Removing files..."
    echo "================="
    
    removed_count=0
    removed_size=0
    
    for i in "${!FILES_TO_REMOVE[@]}"; do
        file="${FILES_TO_REMOVE[$i]}"
        size="${FILE_SIZES[$i]}"
        
        if rm "$file" 2>/dev/null; then
            removed_count=$((removed_count + 1))
            removed_size=$((removed_size + size))
            echo "REMOVED: $file ($(format_bytes $size))"
        else
            echo "ERROR: Failed to remove $file"
        fi
    done
    
    echo ""
    echo "Removal Summary:"
    echo "================"
    echo "Files successfully removed: $removed_count"
    echo "Total size freed: $(format_bytes $removed_size)"
    
    if [ $removed_count -lt $files_to_remove ]; then
        failed_count=$((files_to_remove - removed_count))
        echo "Failed to remove: $failed_count files"
        echo "Check file permissions and try again if needed."
    fi
else
    echo ""
    echo "DRY RUN COMPLETE: No files were actually removed."
    echo "Run without --dry-run to perform the actual removal."
fi

echo ""
echo "Operation completed." 
