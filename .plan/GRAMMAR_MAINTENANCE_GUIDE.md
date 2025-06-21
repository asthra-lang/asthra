# Grammar Maintenance Guide

**Version**: 1.0  
**Created**: 2025-01-08  
**Target Grammar**: v1.22  
**Status**: Production Ready  

## Overview

This guide provides comprehensive instructions for maintaining grammar conformance across the Asthra project. It covers automated tools, CI/CD integration, and best practices for preventing grammar violations.

## Quick Start

### For Developers

```bash
# Check a single file
python3 scripts/validate_single_file.py docs/asthrabyexample.md

# Fix a markdown file
python3 scripts/fix_markdown_examples.py docs/asthrabyexample.md

# Check all Asthra files
python3 scripts/grammar_conformance_checker.py --scan . --fix all --dry-run

# Validate user manual
./scripts/validate_manual_examples.sh docs/user-manual/
```

### For CI/CD

The project includes automated grammar conformance checking:

- **Pre-commit hooks**: Prevent violations from being committed
- **CI workflow**: Automated checking on pull requests
- **Automated reporting**: Regular conformance status reports

## Available Tools

### 1. Single File Validator (`validate_single_file.py`)

**Purpose**: Validate Asthra code examples in individual markdown files

**Usage**:
```bash
python3 scripts/validate_single_file.py <file_path>
```

**Features**:
- Extracts Asthra code blocks from markdown
- Validates against v1.22 grammar requirements
- Reports specific violations with line numbers
- Exit code 0 for success, 1 for violations found

**Example Output**:
```
Grammar Validation for docs/asthrabyexample.md
==================================================
Found 31 Asthra code examples

Example 1: VALID
Example 2: INVALID
  - String interpolation found (removed in v1.21)
  - Empty parameter list should use (none)

==================================================
Validation Summary:
Total examples: 31
Valid examples: 29
Invalid examples: 2
Total violations: 3
❌ Grammar violations found - fixes needed
```

### 2. Markdown Example Fixer (`fix_markdown_examples.py`)

**Purpose**: Automatically fix common grammar violations in markdown files

**Usage**:
```bash
# Dry run (show what would be fixed)
python3 scripts/fix_markdown_examples.py <file_path> --dry-run

# Apply fixes with backup
python3 scripts/fix_markdown_examples.py <file_path>

# Custom backup directory
python3 scripts/fix_markdown_examples.py <file_path> --backup-dir /path/to/backups
```

**Fixes Applied**:
- Empty parameter lists: `fn main()` → `fn main(none)`
- Missing visibility modifiers: `fn main` → `pub fn main`
- Return void statements: `return void;` → `return;`
- Basic auto type annotations: `let x: auto = 5` → `let x: i32 = 5`

### 3. Advanced String Interpolation Fixer (`fix_string_interpolation_advanced.py`)

**Purpose**: Convert string interpolation to concatenation

**Usage**:
```bash
python3 scripts/fix_string_interpolation_advanced.py <file_path>
```

**Conversions**:
- `"Hello {name}!"` → `"Hello " + name + "!"`
- `"Value: {x}"` → `"Value: " + x`
- `"Count: {items.len}"` → `"Count: " + items.len`

### 4. Grammar Conformance Checker (`grammar_conformance_checker.py`)

**Purpose**: Comprehensive checking and fixing of Asthra source files

**Usage**:
```bash
# Check specific directory
python3 scripts/grammar_conformance_checker.py --scan examples/ --fix all --dry-run

# Apply fixes with backup
python3 scripts/grammar_conformance_checker.py --scan . --fix all --backup-dir backups/

# Generate report
python3 scripts/grammar_conformance_checker.py --scan . --fix all --report violations.json
```

### 5. User Manual Validator (`validate_manual_examples.sh`)

**Purpose**: Validate all examples in the user manual

**Usage**:
```bash
./scripts/validate_manual_examples.sh docs/user-manual/
```

**Features**:
- Processes all markdown files in user manual
- Comprehensive validation reporting
- Integration with CI/CD pipeline

## Grammar Requirements (v1.22)

### Critical Requirements

1. **No String Interpolation** (removed in v1.21)
   - ❌ `"Hello {name}"`
   - ✅ `"Hello " + name`

2. **Explicit Parameter Lists** (required since v1.16)
   - ❌ `fn main() -> i32`
   - ✅ `fn main(none) -> i32`

3. **Visibility Modifiers** (required since v1.16)
   - ❌ `fn main(none) -> i32`
   - ✅ `pub fn main(none) -> i32`

4. **Explicit Type Annotations** (required since v1.15)
   - ❌ `let x: auto = 5`
   - ✅ `let x: i32 = 5`

5. **Import Aliases** (added in v1.22)
   - ✅ `import "stdlib/string" as str`

### Return Statements (v1.14)
- ❌ `return void;`
- ✅ `return;`

## CI/CD Integration

### GitHub Actions Workflow

The project includes a comprehensive CI workflow (`.github/workflows/grammar-conformance.yml`) that:

1. **Validates Asthra Files**: Checks all `.asthra` files for grammar conformance
2. **Validates Documentation**: Checks code examples in markdown files
3. **Generates Reports**: Creates detailed conformance reports
4. **Comments on PRs**: Automatically comments on pull requests with results

### Pre-commit Hooks

Pre-commit hooks (`.pre-commit-config.yaml`) prevent violations from being committed:

1. **Asthra Grammar Check**: Validates `.asthra` files
2. **Documentation Examples Check**: Validates markdown files
3. **User Manual Validation**: Comprehensive user manual checking

**Setup**:
```bash
# Install pre-commit
pip install pre-commit

# Install hooks
pre-commit install

# Run manually
pre-commit run --all-files
```

## Best Practices

### For Developers

1. **Run Validation Before Committing**:
   ```bash
   python3 scripts/validate_single_file.py docs/your-file.md
   ```

2. **Use Automated Fixes**:
   ```bash
   python3 scripts/fix_markdown_examples.py docs/your-file.md --dry-run
   ```

3. **Check User Manual Changes**:
   ```bash
   ./scripts/validate_manual_examples.sh docs/user-manual/
   ```

### For Documentation Writers

1. **Always Use v1.22 Syntax**:
   - Include visibility modifiers (`pub`/`priv`)
   - Use explicit parameter lists (`(none)` for empty)
   - Use string concatenation instead of interpolation
   - Include explicit type annotations

2. **Test Examples**:
   - Validate examples compile with current grammar
   - Use validation tools before submitting

3. **Follow Patterns**:
   - Look at existing compliant examples
   - Use the user manual as reference

### For Maintainers

1. **Regular Audits**:
   ```bash
   python3 scripts/grammar_conformance_checker.py --scan . --report audit.json
   ```

2. **Monitor CI Results**:
   - Review grammar conformance reports
   - Address violations promptly

3. **Update Tools**:
   - Keep validation scripts updated with grammar changes
   - Add new violation patterns as needed

## Troubleshooting

### Common Issues

1. **"String interpolation found"**
   - **Solution**: Use `fix_string_interpolation_advanced.py`
   - **Manual**: Replace `"text {var}"` with `"text " + var`

2. **"Empty parameter list should use (none)"**
   - **Solution**: Use `fix_markdown_examples.py`
   - **Manual**: Replace `fn name()` with `fn name(none)`

3. **"Missing visibility modifier"**
   - **Solution**: Use `fix_markdown_examples.py`
   - **Manual**: Add `pub` or `priv` before declarations

4. **"Auto type annotation found"**
   - **Solution**: Use `fix_markdown_examples.py` for basic cases
   - **Manual**: Replace `auto` with specific type

### Tool Issues

1. **Script Not Found**:
   ```bash
   chmod +x scripts/validate_manual_examples.sh
   ```

2. **Python Import Errors**:
   ```bash
   python3 -m pip install --upgrade pip
   ```

3. **Permission Denied**:
   ```bash
   chmod +x scripts/*.py
   chmod +x scripts/*.sh
   ```

## Maintenance Schedule

### Daily
- CI/CD automatically checks all changes
- Pre-commit hooks prevent violations

### Weekly
- Review CI reports for trends
- Address any new violation patterns

### Monthly
- Comprehensive audit of entire codebase
- Update tools for new grammar requirements
- Review and update this guide

### Quarterly
- Major tool improvements
- Performance optimization
- Documentation updates

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-01-08 | Initial version with v1.22 support |

## Support

For issues with grammar conformance tools:

1. **Check this guide** for common solutions
2. **Run tools with `--verbose`** for detailed output
3. **Check CI logs** for automated reports
4. **Review backup files** if fixes go wrong

## Related Documentation

- [Grammar Conformance Fix Plan](GRAMMAR_CONFORMANCE_FIX_PLAN.md) - Implementation history
- [User Manual Maintenance Plan](USER_MANUAL_MAINTENANCE_PLAN.md) - Documentation strategy
- [Grammar Specification](spec/grammar.md) - Authoritative grammar reference

---

**Maintained by**: Asthra Development Team  
**Last Updated**: 2025-01-08  
**Next Review**: 2025-02-08 
