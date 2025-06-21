#!/usr/bin/env python3
"""
Fix MDX compilation issues by replacing problematic patterns with HTML entities.
"""

import os
import re
import glob

def fix_mdx_issues(file_path):
    """Fix MDX compilation issues in a single file."""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    
    # Fix patterns that cause MDX compilation errors
    
    # Fix " + ( pattern - replace with &#123;
    content = re.sub(r'" \+ \(', '&#123;', content)
    
    # Fix ) + " pattern - replace with &#125;
    content = re.sub(r'\) \+ "', '&#125;', content)
    
    # Fix < followed by numbers (like <100ms)
    content = re.sub(r'<(\d+)', r'&lt;\1', content)
    
    # Fix commas in JSX-like contexts (like <T,E>)
    content = re.sub(r'<([^>]*),([^>]*?)>', r'&lt;\1,\2&gt;', content)
    
    # Fix standalone < and > that might be interpreted as JSX
    content = re.sub(r'(?<!\w)<(?!\w)', '&lt;', content)
    content = re.sub(r'(?<!\w)>(?!\w)', '&gt;', content)
    
    # Fix numbered list items that start with numbers followed by periods
    content = re.sub(r'^(\d+)\.', r'\1\.', content, flags=re.MULTILINE)
    
    # Fix ampersands that aren't already HTML entities
    content = re.sub(r'&(?![a-zA-Z0-9#]+;)', '&amp;', content)
    
    # Fix square brackets in inline text that might be interpreted as JSX
    content = re.sub(r'\[([^\]]*)\]', r'&#91;\1&#93;', content)
    
    if content != original_content:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Fixed MDX issues in: {file_path}")
        return True
    
    return False

def main():
    """Fix MDX issues in all documentation files."""
    docs_dir = "docs"
    
    # Find all markdown files
    md_files = []
    for root, dirs, files in os.walk(docs_dir):
        for file in files:
            if file.endswith('.md'):
                md_files.append(os.path.join(root, file))
    
    fixed_count = 0
    for file_path in md_files:
        if fix_mdx_issues(file_path):
            fixed_count += 1
    
    print(f"\nFixed MDX issues in {fixed_count} files.")

if __name__ == "__main__":
    main() 