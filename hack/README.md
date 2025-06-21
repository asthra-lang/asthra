# Hack Utilities

This directory contains utility scripts for maintaining and analyzing the Asthra codebase. These tools help with code quality, cleanup, and analysis tasks.

## 📁 Available Scripts

### 🗑️ `remove_binary_blobs.sh`
**Purpose**: Remove binary blobs from specified directories to clean up the codebase and reduce repository size.

**Key Features**:
- Intelligent binary file detection using `file` command and extension analysis
- Size threshold filtering (default: 1MB minimum)
- Dry-run mode for safe preview
- Directory and pattern exclusions
- Confirmation prompts for safety
- Detailed reporting and statistics

**Basic Usage**:
```bash
# Preview what would be removed (safe)
./remove_binary_blobs.sh --dry-run .

# Remove binary files larger than 1MB (default)
./remove_binary_blobs.sh .

# Remove files larger than 5MB with verbose output
./remove_binary_blobs.sh -s 5M --verbose .

# Exclude specific directories
./remove_binary_blobs.sh -e .git -e node_modules -e build .

# Only target specific file types
./remove_binary_blobs.sh -i '*.exe' -i '*.dll' .
```

**Common Options**:
- `-n, --dry-run`: Preview without deleting
- `-v, --verbose`: Show detailed file information
- `-s, --size-threshold SIZE`: Set minimum file size (e.g., 5M, 100K)
- `-e, --exclude DIR`: Exclude directories
- `-p, --exclude-pattern PATTERN`: Exclude file patterns
- `--force`: Skip confirmation (use with caution)

---

### 📊 `find_largest_source_files.sh`
**Purpose**: Find the largest source files by line count to identify files that might need refactoring or modularization.

**Key Features**:
- Searches multiple programming languages and file types
- Configurable directory exclusions
- Pattern-based file exclusions
- Detailed file type support (C/C++, Python, JavaScript, Go, Rust, etc.)
- Line count analysis with file size information

**Basic Usage**:
```bash
# Find largest files in current directory
./find_largest_source_files.sh

# Search specific directories
./find_largest_source_files.sh src/ tests/ docs/

# Exclude build directories and test files
./find_largest_source_files.sh -e build -e node_modules -p '*.test.*' .

# Exclude documentation files
./find_largest_source_files.sh -p '*.md' -p '*.txt' src/
```

**Common Options**:
- `-e, --exclude DIR`: Exclude directories from search
- `-p, --exclude-pattern PATTERN`: Exclude files matching pattern
- `-h, --help`: Show detailed help and examples

**Supported File Types**:
- Programming: C, C++, Python, Java, JavaScript, TypeScript, Go, Rust, Ruby, PHP, etc.
- Configuration: JSON, YAML, TOML, XML, INI
- Build files: Makefile, CMake, Gradle
- Documentation: Markdown, LaTeX

---

### 🔧 `fix_missing_newlines.sh`
**Purpose**: Check and fix missing trailing newlines in source files to ensure consistent file formatting.

**Key Features**:
- Detects files missing trailing newlines
- Automatically adds missing newlines
- Dry-run mode for preview
- Supports all major source file types
- Safe text file detection to avoid binary files

**Basic Usage**:
```bash
# Preview files that need fixing
./fix_missing_newlines.sh --dry-run .

# Fix missing newlines in current directory
./fix_missing_newlines.sh .

# Fix files in specific directories
./fix_missing_newlines.sh src/ tests/ docs/
```

**Common Options**:
- `-n, --dry-run`: Show files that would be modified without making changes
- `-h, --help`: Show help message

**Why This Matters**:
- Many tools expect files to end with newlines
- POSIX standard requires text files to end with newlines
- Prevents "No newline at end of file" warnings in Git
- Ensures consistent formatting across the codebase

---

### 🗑️ `remove_macho_executables.sh`
**Purpose**: Find and remove Mach-O 64-bit executable arm64 files to clean up build artifacts before committing to version control.

**Key Features**:
- Specifically targets Mach-O 64-bit executable arm64 files using `file` command detection
- Dry-run mode for safe preview
- Directory and pattern exclusions
- Confirmation prompts for safety
- Detailed file size reporting
- List-only mode for analysis without removal

**Basic Usage**:
```bash
# Preview what would be removed (safe)
./remove_macho_executables.sh --dry-run .

# Remove Mach-O executables with confirmation
./remove_macho_executables.sh .

# List all Mach-O executables with details
./remove_macho_executables.sh --list-only --verbose .

# Exclude specific directories
./remove_macho_executables.sh -e .git -e ampu/target .

# Exclude files matching patterns
./remove_macho_executables.sh -p 'test_*' -p '*_demo' .
```

**Common Options**:
- `-n, --dry-run`: Preview without deleting
- `-v, --verbose`: Show detailed file information with sizes
- `-e, --exclude DIR`: Exclude directories
- `-p, --exclude-pattern PATTERN`: Exclude file patterns
- `--list-only`: Only list files, don't prompt for removal
- `--force`: Skip confirmation (use with caution)

**What This Detects**:
- Compiled executables from C/C++, Rust, Go, and other native languages
- Build artifacts that shouldn't be committed to version control
- Test executables and demo programs
- Build script executables from Rust/Cargo builds

---

### 📏 `find_largest_files.sh`
**Purpose**: Find the largest files by disk size to identify potential cleanup targets or storage issues.

**Key Features**:
- Finds files by actual disk size (not line count)
- Human-readable size formatting
- Simple and fast analysis
- Helps identify large binary files or data files

**Basic Usage**:
```bash
# Find largest files in current directory
./find_largest_files.sh

# Search specific directory
./find_largest_files.sh /path/to/directory
```

**Output Example**:
```
Finding the 10 largest files in: .
=============================================
   15M  ./build/asthra_compiler
   8.2M ./tests/data/large_dataset.bin
   3.1M ./docs/manual.pdf
   2.8M ./src/parser/generated_parser.c
   1.9M ./lib/external_library.a
```

### 🧪 `scan_tests.sh`
**Purpose**: Scan and analyze test files in the Asthra programming language test suite to generate comprehensive test reports.

**Key Features**:
- Test category organization analysis (31 test directories)
- Test file discovery with pattern matching
- Test function counting and naming pattern analysis
- Build artifact detection (.o files)
- File size and line count statistics
- Multiple output formats (text, CSV, JSON, markdown)
- Detailed test coverage analysis

**Basic Usage**:
```bash
# Scan default tests directory with summary
./scan_tests.sh

# Show only test categories
./scan_tests.sh -c

# Detailed analysis with statistics
./scan_tests.sh -s --detailed

# Generate CSV report for spreadsheet analysis
./scan_tests.sh -f csv -o test_report.csv

# Generate markdown documentation
./scan_tests.sh -f markdown -o TEST_DOCUMENTATION.md

# Exclude specific test categories
./scan_tests.sh -e integration -e performance

# Include only specific file patterns
./scan_tests.sh -i 'test_*.c' --detailed
```

**Output Formats**:
- `text` - Human-readable colored terminal output (default)
- `csv` - Comma-separated values for spreadsheet analysis
- `json` - JSON format for programmatic processing
- `markdown` - Markdown format for documentation

**Analysis Features**:
- **Test Organization**: Scans 31+ test categories (lexer, parser, semantic, codegen, etc.)
- **Function Detection**: Counts test functions using `test_*` and `*_test` patterns
- **Build Artifacts**: Detects .o files indicating compiled tests
- **Statistics**: File sizes, line counts, test function density
- **Coverage Analysis**: Test distribution across categories

**Common Options**:
- `-s, --stats`: Include detailed statistics
- `-d, --detailed`: Show detailed test function analysis
- `-c, --categories-only`: Show only test categories
- `-e, --exclude DIR`: Exclude specific test categories
- `-i, --include PATTERN`: Include files matching pattern
- `-f, --format FORMAT`: Output format (text, csv, json, markdown)
- `-o, --output FILE`: Save results to file
- `--no-color`: Disable colored output

**Sample Output**:
```
==================================================================
                  ASTHRA TEST SUITE ANALYSIS
==================================================================
📊 Test Categories: 31
📄 Test Files: 691
🧪 Test Functions: 490
🔧 Build Artifacts: 57
📏 Total Size: 5.1MB
📝 Total Lines: 146,358
📈 Avg Functions/File: 0.7
📈 Avg Lines/File: 212
```

## 🚀 Quick Start Guide

### 1. **Repository Cleanup Workflow**
```bash
# Step 1: Analyze what's taking up space
./find_largest_files.sh

# Step 2: Preview Mach-O executable removal (common build artifacts)
./remove_macho_executables.sh --dry-run .

# Step 3: Remove Mach-O executables (with confirmation)
./remove_macho_executables.sh -e .git -e ampu/target .

# Step 4: Preview binary blob removal
./remove_binary_blobs.sh --dry-run .

# Step 5: Remove binary blobs (with confirmation)
./remove_binary_blobs.sh -e .git -e node_modules .

# Step 6: Fix file formatting issues
./fix_missing_newlines.sh .
```

### 2. **Code Quality Analysis**
```bash
# Find large source files that might need refactoring
./find_largest_source_files.sh -e build -e vendor .

# Focus on specific languages
./find_largest_source_files.sh -p '*.c' -p '*.h' src/
```

### 3. **Pre-commit Cleanup**
```bash
# Quick formatting fix
./fix_missing_newlines.sh src/ tests/

# Remove any accidentally committed Mach-O executables
./remove_macho_executables.sh --dry-run .

# Remove any accidentally committed binaries
./remove_binary_blobs.sh --dry-run .
```

## 🛡️ Safety Features

All scripts include multiple safety features:

- **Dry-run modes**: Preview changes before applying them
- **Confirmation prompts**: Explicit confirmation for destructive operations
- **Exclude patterns**: Protect important files and directories
- **Error handling**: Graceful handling of missing files or permissions
- **Detailed logging**: Clear output showing what's being done

## 📋 Common Use Cases

### Repository Maintenance
- **Before releases**: Clean up build artifacts and temporary files
- **Storage optimization**: Remove large binary files that shouldn't be in version control
- **Code quality**: Identify large files that might need refactoring

### Development Workflow
- **Pre-commit hooks**: Ensure consistent file formatting
- **Build cleanup**: Remove generated files and build artifacts
- **Dependency management**: Clean up downloaded dependencies and caches

### Analysis and Monitoring
- **Technical debt**: Identify large, complex files
- **Storage usage**: Track what's consuming disk space
- **File organization**: Understand codebase structure and size distribution

## 🔧 Customization

### Environment Variables
Some scripts respect common environment variables:
- `EDITOR`: Used for any interactive editing features
- `PAGER`: Used for displaying long output

### Configuration Files
Scripts use command-line arguments rather than configuration files for simplicity and transparency.

### Integration with Build Systems
These scripts can be integrated into:
- **Makefiles**: Add targets for cleanup and analysis
- **CI/CD pipelines**: Automated repository maintenance
- **Git hooks**: Pre-commit and pre-push validation
- **Development scripts**: Part of larger development workflows

## 📚 Examples

### Complete Repository Cleanup
```bash
#!/bin/bash
# Complete cleanup script

echo "=== Repository Cleanup ==="

echo "1. Finding largest files..."
./hack/find_largest_files.sh

echo -e "\n2. Analyzing source file sizes..."
./hack/find_largest_source_files.sh -e build -e .git .

echo -e "\n3. Checking for binary blobs..."
./hack/remove_binary_blobs.sh --dry-run -e .git .

echo -e "\n4. Fixing file formatting..."
./hack/fix_missing_newlines.sh src/ tests/

echo -e "\n=== Cleanup Complete ==="
```

### Selective Binary Cleanup
```bash
# Remove only large executables and archives, preserve small files
./hack/remove_binary_blobs.sh \
  --size-threshold 10M \
  --include-pattern '*.exe' \
  --include-pattern '*.zip' \
  --include-pattern '*.tar.gz' \
  --verbose \
  build/
```

### Development Analysis
```bash
# Analyze code complexity by file size
./hack/find_largest_source_files.sh \
  --exclude build \
  --exclude vendor \
  --exclude-pattern '*.generated.*' \
  --exclude-pattern '*.min.*' \
  src/
```

## 🤝 Contributing

When adding new scripts to this directory:

1. **Follow the established patterns**: Use similar command-line argument parsing and help systems
2. **Include comprehensive help**: Use `--help` flag with detailed examples
3. **Add safety features**: Include dry-run modes and confirmation prompts for destructive operations
4. **Update this README**: Document new scripts with usage examples
5. **Make scripts executable**: `chmod +x script_name.sh`
6. **Use consistent style**: Follow the coding patterns established in existing scripts

## 📄 License

These utility scripts are part of the Asthra programming language project and follow the same license terms as the main project. 
