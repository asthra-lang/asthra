#!/bin/bash
# fix_qmark_systematic.sh - Systematic replacement with validation
# Part of Question Mark Operator Removal - Phase 4 Automation Plan

set -e

echo "=== Systematic Question Mark Operator Replacement ==="

# Configuration
TEMP_DIR="/tmp/qmark_phase4"
FILES_LIST="$TEMP_DIR/qmark_files.txt"
BACKUP_DIR="$TEMP_DIR/backups"
LOG_FILE="$TEMP_DIR/replacement_log.txt"

mkdir -p "$BACKUP_DIR"

if [ ! -f "$FILES_LIST" ]; then
    echo "❌ Error: Files list not found. Run detection script first."
    exit 1
fi

echo "Replacement Log - $(date)" > "$LOG_FILE"
echo "==========================================" >> "$LOG_FILE"

# Function to fix individual file
fix_file() {
    local file="$1"
    local backup="${BACKUP_DIR}/$(basename "$file").backup"
    local temp_file="${file}.tmp"
    
    echo "Processing: $file"
    echo "Processing: $file" >> "$LOG_FILE"
    
    # Create backup
    cp "$file" "$backup"
    
    # Apply systematic replacements using established patterns from Phases 1-3
    # Process line by line for better control
    while IFS= read -r line; do
        # Pattern 1: Assignment with question mark
        # Before: let var = expression()?;
        if echo "$line" | grep -q 'let .* = .*)?;'; then
            var_name=$(echo "$line" | sed 's/^[[:space:]]*let \([a-zA-Z_][a-zA-Z0-9_]*\) = .*/\1/')
            expression=$(echo "$line" | sed 's/^[[:space:]]*let [a-zA-Z_][a-zA-Z0-9_]* = \(.*\)?;.*/\1/')
            echo "    let ${var_name}_result = ${expression};"
            echo "    let $var_name = match ${var_name}_result {"
            echo "        Result.Ok(value) => value,"
            echo "        Result.Err(e) => return Result.Err(e)"
            echo "    };"
        # Pattern 2: Simple operations ending with semicolon
        # Before: expression()?;
        elif echo "$line" | grep -q '.*)?;$'; then
            expression=$(echo "$line" | sed 's/^[[:space:]]*\(.*\)?;$/\1/')
            echo "    match $expression {"
            echo "        Result.Ok(_) => {},"
            echo "        Result.Err(e) => return Result.Err(e)"
            echo "    }"
        else
            # Keep line unchanged
            echo "$line"
        fi
    done < "$file" > "$temp_file"
    
    # Check if changes were made
    if ! cmp -s "$file" "$temp_file"; then
        mv "$temp_file" "$file"
        echo "✅ $file - Changes applied" | tee -a "$LOG_FILE"
        return 0
    else
        rm "$temp_file"
        echo "ℹ️  $file - No changes needed" | tee -a "$LOG_FILE"
        return 1
    fi
}

# Function to validate syntax (basic check)
validate_syntax() {
    local file="$1"
    
    # Basic syntax validation - check for balanced braces and common issues
    if ! grep -q "Result\.Ok\|Result\.Err" "$file"; then
        echo "⚠️  $file - No Result patterns found after processing"
        return 1
    fi
    
    # Check for unbalanced braces (simple heuristic)
    local open_braces=$(grep -o '{' "$file" | wc -l)
    local close_braces=$(grep -o '}' "$file" | wc -l)
    
    if [ "$open_braces" -ne "$close_braces" ]; then
        echo "❌ $file - Unbalanced braces detected"
        return 1
    fi
    
    echo "✅ $file - Basic syntax validation passed"
    return 0
}

# Process files in batches
batch_size=5
processed=0
successful=0
failed=0

echo ""
echo "Processing files in batches of $batch_size..."

while read -r file; do
    if [ ! -f "$file" ]; then
        echo "⚠️  Skipping non-existent file: $file"
        continue
    fi
    
    # Process file
    if fix_file "$file"; then
        # Validate the changes
        if validate_syntax "$file"; then
            ((successful++))
            echo "Success: $file" >> "$LOG_FILE"
        else
            # Restore from backup if validation fails
            backup="${BACKUP_DIR}/$(basename "$file").backup"
            if [ -f "$backup" ]; then
                cp "$backup" "$file"
                echo "❌ $file - Syntax validation failed, restored from backup" | tee -a "$LOG_FILE"
            fi
            ((failed++))
        fi
    fi
    
    ((processed++))
    
    # Progress update
    if [ $((processed % batch_size)) -eq 0 ]; then
        echo ""
        echo "Progress: $processed files processed ($successful successful, $failed failed)"
        echo ""
    fi
    
done < "$FILES_LIST"

# Final summary
echo ""
echo "=== Processing Complete ==="
echo "Total files processed: $processed"
echo "Successful fixes: $successful"
echo "Failed fixes: $failed"
echo "Success rate: $(( successful * 100 / processed ))%"

echo "" >> "$LOG_FILE"
echo "=== Final Summary ===" >> "$LOG_FILE"
echo "Total files processed: $processed" >> "$LOG_FILE"
echo "Successful fixes: $successful" >> "$LOG_FILE"
echo "Failed fixes: $failed" >> "$LOG_FILE"
echo "Success rate: $(( successful * 100 / processed ))%" >> "$LOG_FILE"

echo ""
echo "Log file: $LOG_FILE"
echo "Backups stored in: $BACKUP_DIR"

echo ""
echo "Next steps:"
echo "1. Review the processing log"
echo "2. Run QA validation script"
echo "3. Test compilation of modified files"

if [ "$failed" -gt 0 ]; then
    echo ""
    echo "⚠️  Warning: $failed files failed processing and were restored from backup."
    echo "These may require manual intervention."
fi 
