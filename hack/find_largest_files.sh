#!/bin/bash

# Script to find the largest 5 files in a directory including subdirectories
# Usage: ./find_largest_files.sh [directory_path] [number_of_files]

# Default values
DEFAULT_DIR="."
DEFAULT_COUNT=5

# Function to display usage
usage() {
    echo "Usage: $0 [directory_path] [number_of_files]"
    echo "  directory_path: Path to search (default: current directory)"
    echo "  number_of_files: Number of largest files to display (default: 5)"
    echo ""
    echo "Examples:"
    echo "  $0                    # Find 5 largest files in current directory"
    echo "  $0 /home/user         # Find 5 largest files in /home/user"
    echo "  $0 /var/log 10        # Find 10 largest files in /var/log"
    exit 1
}

# Parse command line arguments
if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    usage
fi

# Set directory path (first argument or default)
SEARCH_DIR="${1:-$DEFAULT_DIR}"

# Set number of files to display (second argument or default)
NUM_FILES="${2:-$DEFAULT_COUNT}"

# Validate that the directory exists
if [ ! -d "$SEARCH_DIR" ]; then
    echo "Error: Directory '$SEARCH_DIR' does not exist or is not accessible."
    exit 1
fi

# Validate that num_files is a positive integer
if ! [[ "$NUM_FILES" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Number of files must be a positive integer."
    exit 1
fi

echo "Finding the largest $NUM_FILES files in '$SEARCH_DIR' and its subdirectories..."
echo "========================================================================"

# Find files, get their sizes, sort by size (largest first), and display top N
# Using find to get all files recursively
# -type f: only files (not directories)
# -exec ls -la {} \; : execute ls -la on each file to get detailed info
# awk: extract size and filename
# sort -nr: sort numerically in reverse order (largest first)
# head: get top N files
# awk again: format the output nicely

find "$SEARCH_DIR" -type f -exec ls -la {} \; 2>/dev/null | \
    awk '{
        # Extract size (5th field) and filename (last field)
        size = $5
        filename = $0
        # Remove everything up to and including the last space to get just the filename
        gsub(/^.* /, "", filename)
        print size " " filename
    }' | \
    sort -nr | \
    head -n "$NUM_FILES" | \
    awk '{
        size = $1
        filename = $2
        for (i = 3; i <= NF; i++) filename = filename " " $i
        
        # Convert size to human readable format
        if (size >= 1073741824) {
            printf "%8.2f GB  %s\n", size/1073741824, filename
        } else if (size >= 1048576) {
            printf "%8.2f MB  %s\n", size/1048576, filename
        } else if (size >= 1024) {
            printf "%8.2f KB  %s\n", size/1024, filename
        } else {
            printf "%8d B   %s\n", size, filename
        }
    }'

echo "========================================================================"
echo "Search completed." 
