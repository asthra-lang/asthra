#!/bin/bash
# fix_single_file.sh - Fix question mark operators in a single file
# Part of Question Mark Operator Removal - Phase 4 Automation Plan

set -e

if [ $# -ne 1 ]; then
    echo "Usage: $0 <file_path>"
    exit 1
fi

file="$1"
temp_file="${file}.tmp"
backup_file="${file}.backup_phase4"

echo "Processing: $file"

# Create backup
cp "$file" "$backup_file"

# Apply systematic replacements
python3 - "$file" << 'EOF'
import sys
import re

def fix_question_mark_operators(content):
    lines = content.split('\n')
    result_lines = []
    
    for line in lines:
        original_line = line
        
        # Pattern 1: Assignment with question mark
        # Before: let var = expression()?;
        assignment_match = re.match(r'^(\s*)let\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*(.*)\?\s*;\s*$', line)
        if assignment_match:
            indent, var_name, expression = assignment_match.groups()
            result_lines.append(f"{indent}let {var_name}_result = {expression};")
            result_lines.append(f"{indent}let {var_name} = match {var_name}_result {{")
            result_lines.append(f"{indent}    Result.Ok(value) => value,")
            result_lines.append(f"{indent}    Result.Err(e) => return Result.Err(e)")
            result_lines.append(f"{indent}}};")
            continue
        
        # Pattern 2: Simple operations ending with semicolon
        # Before: expression()?;
        operation_match = re.match(r'^(\s*)(.*)\?\s*;\s*$', line)
        if operation_match and 'let' not in line:
            indent, expression = operation_match.groups()
            result_lines.append(f"{indent}match {expression} {{")
            result_lines.append(f"{indent}    Result.Ok(_) => {{}},")
            result_lines.append(f"{indent}    Result.Err(e) => return Result.Err(e)")
            result_lines.append(f"{indent}}}")
            continue
        
        # Keep line unchanged
        result_lines.append(line)
    
    return '\n'.join(result_lines)

# Read file
file_path = sys.argv[1]
with open(file_path, 'r') as f:
    content = f.read()

# Fix content
fixed_content = fix_question_mark_operators(content)

# Write to temp file
with open(file_path + '.tmp', 'w') as f:
    f.write(fixed_content)

EOF

# Check if changes were made
if ! cmp -s "$file" "$temp_file"; then
    mv "$temp_file" "$file"
    echo "✅ $file - Changes applied"
    
    # Basic validation
    if grep -q "Result\.Ok\|Result\.Err" "$file"; then
        echo "✅ $file - Basic syntax validation passed"
        rm "$backup_file"
        exit 0
    else
        echo "❌ $file - Syntax validation failed, restoring backup"
        mv "$backup_file" "$file"
        exit 1
    fi
else
    rm "$temp_file"
    rm "$backup_file"
    echo "ℹ️  $file - No changes needed"
    exit 0
fi 
