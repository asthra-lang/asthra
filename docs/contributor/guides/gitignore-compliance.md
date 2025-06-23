# Gitignore Compliance Guide

This guide explains how to work with Asthra's `.gitignore` patterns to avoid accidentally creating files that won't be tracked by Git or that will be lost during cleanup operations.

## Critical Warning

**Files matching `.gitignore` patterns will NOT be committed to Git and WILL BE LOST when running `git clean -fdx`**

## Problematic Patterns for Source Files

### Debug Files
**NEVER use these patterns for permanent source files:**
- `debug_*.c`, `debug_*.h` - Debug prefix pattern
- `*_debug.c`, `*_debug.h` - Debug suffix pattern
- `debug_*.*` - Any debug-prefixed file

These patterns are reserved for temporary debugging files that should not be committed.

### Test Executables
**NEVER name source files with these patterns:**
- `*_test` (without extension) - This matches test executables
- Always use `.c` or `.h` extensions for source files

### Documentation
**Avoid these patterns for permanent documentation:**
- `*_error_analysis.md` - Reserved for temporary error analysis
- `*_migration_summary.md` - Reserved for migration notes
- `TODO.local.md` - Reserved for personal notes
- `local-notes.txt` - Reserved for local development notes

## Safe Patterns for Source Files

### Test Source Files
✅ **CORRECT patterns:**
- `test_*.c`, `test_*.h` - Test files with test_ prefix
- `*_test_*.c`, `*_test_*.h` - Files with _test_ in the middle
- `test_semantic.c` - Good test file name
- `parser_test_utils.c` - Good utility file name

❌ **INCORRECT patterns:**
- `semantic_test` - Missing extension (will be ignored)
- `debug_test.c` - Starts with debug_ (will be ignored)
- `test_debug.c` - Contains debug (will be ignored)

### Regular Source Files
✅ **Always safe:**
- Standard names: `parser.c`, `semantic.h`, `codegen.c`
- Module names: `ast_types.h`, `symbol_table.c`
- Implementation files: `parser_impl.c`, `semantic_analysis.h`

## Using Gitignore Patterns for Temporary Files

### Recommended Temporary File Patterns

These patterns are ENCOURAGED for temporary work:

```bash
# Debugging files
debug_parser_issue.c      # Temporary debugging code
parser_debug.c           # Debug version of parser
debug_analysis.md        # Debug notes

# Analysis documents  
semantic_error_analysis.md    # Error investigation
parser_migration_summary.md   # Migration notes

# Test outputs
parser_test_output.txt       # Test results
semantic_test_output.log     # Test logs

# Personal notes
TODO.local.md               # Personal todo list
local-notes.txt             # Local development notes
```

### Benefits of Using Temporary Patterns

1. **No accidental commits** - Files are automatically ignored
2. **Easy cleanup** - `git clean -fdx` removes all temporary files
3. **Clear separation** - Temporary vs permanent files are obvious
4. **Team-friendly** - Personal files don't clutter the repository

## Verification Commands

### Check if a File Will Be Ignored

Before creating a file, verify it won't be ignored:

```bash
# Check if a file would be ignored
git check-ignore -v proposed_filename.c

# No output = file will be tracked
# Output shows which .gitignore rule matches
```

### List All Ignored Files

```bash
# Show all ignored files in the repository
git status --ignored

# Show ignored files in a specific directory
git status --ignored src/
```

### Clean Temporary Files

```bash
# Preview what will be deleted
git clean -fdx --dry-run

# Remove all untracked and ignored files
git clean -fdx

# Remove only ignored files (keep untracked)
git clean -fdX
```

## Best Practices

1. **Always add extensions** to source files (`.c`, `.h`, `.md`)
2. **Check before creating** - Use `git check-ignore -v` for new filenames
3. **Use temporary patterns** for debugging and analysis work
4. **Document exceptions** if you must use a special pattern
5. **Review `.gitignore`** periodically to understand current patterns

## Common Mistakes to Avoid

### Mistake 1: Debug Prefixed Source Files
```bash
# ❌ WRONG - Will be ignored
touch debug_parser.c
git add debug_parser.c  # Won't work!

# ✅ CORRECT - Will be tracked
touch parser_debug_info.c
git add parser_debug_info.c  # Works!
```

### Mistake 2: Missing Extensions
```bash
# ❌ WRONG - Looks like executable
touch parser_test
git add parser_test  # Won't work!

# ✅ CORRECT - Clear source file
touch test_parser.c
git add test_parser.c  # Works!
```

### Mistake 3: Using Reserved Patterns
```bash
# ❌ WRONG - Reserved for temporary analysis
touch parser_error_analysis.md
git add parser_error_analysis.md  # Won't work!

# ✅ CORRECT - Different naming
touch parser_error_investigation.md
git add parser_error_investigation.md  # Works!
```

## Summary

- **Know the patterns** - Understand what `.gitignore` will ignore
- **Use safe names** - Follow established naming conventions
- **Leverage temporary patterns** - Use ignored patterns for temporary work
- **Verify before creating** - Check if files will be tracked
- **Clean regularly** - Use `git clean` to remove temporary files

Following these guidelines ensures your important work is never lost and your repository stays clean and organized.