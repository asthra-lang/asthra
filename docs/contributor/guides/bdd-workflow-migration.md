# BDD Workflow Migration Guide

This guide documents the migration from the manual BDD test workflow to using the `run-bdd-tests-local.sh` script in GitHub Actions.

## Overview

The new approach simplifies the BDD test workflow by using the same script for both local development and CI, ensuring consistency and reducing maintenance overhead.

## Migration Status

✅ **Migration Complete** - The BDD workflow has been successfully migrated to use the unified script approach.

## Key Improvements

### 1. **Unified Test Execution**
- **Before**: 687 lines of workflow with hardcoded test names and manual execution
- **After**: ~250 lines using the script that auto-discovers tests

### 2. **Consistent @wip Handling**
- **Before**: No special handling for work-in-progress scenarios
- **After**: Automatic @wip filtering with proper exit codes

### 3. **Single Source of Truth**
- **Before**: Separate logic for local and CI testing
- **After**: Same script used everywhere

### 4. **Automatic Test Discovery**
- **Before**: Manually listing each test binary
- **After**: Script automatically finds all `bdd_unit_*` tests

## Migration Steps

### Step 1: Update the Local Script

The `run-bdd-tests-local.sh` script has been enhanced with:

1. **LLVM Path Setup for Linux**:
```bash
# Setup LLVM path for Linux (GitHub Actions compatibility)
if [[ "$OSTYPE" == "linux-gnu"* ]] && [[ -d "/usr/lib/llvm-18/bin" ]]; then
    export PATH="/usr/lib/llvm-18/bin:$PATH"
fi
```

2. **Asthra Compiler Discovery**:
```bash
find_asthra_compiler() {
    local paths=(
        "$BUILD_DIR/asthra"
        "$BUILD_DIR/bin/asthra"
        "$BUILD_DIR/src/asthra"
    )
    
    for path in "${paths[@]}"; do
        if [ -f "$path" ] && [ -x "$path" ]; then
            export ASTHRA_COMPILER_PATH="$path"
            return 0
        fi
    done
    
    # Fallback to find
    local found=$(find "$BUILD_DIR" -name "asthra" -type f -executable | head -1)
    if [ -n "$found" ]; then
        export ASTHRA_COMPILER_PATH="$found"
        return 0
    fi
}
```

3. **CI Environment Variables**:
```bash
export BDD_DEBUG=${BDD_DEBUG:-0}
export ASTHRA_DEBUG=${ASTHRA_DEBUG:-0}
```

### Step 2: Use Simplified Workflow

The new workflow (`bdd-simplified.yml`) is much simpler:

```yaml
- name: Run BDD tests with script
  run: |
    # The script handles everything:
    # - Finding the Asthra compiler
    # - Setting up LLVM paths for Linux
    # - Running all test suites
    # - Skipping @wip scenarios by default
    # - Aggregating results
    # - Proper exit codes
    ./run-bdd-tests-local.sh --verbose
```

### Step 3: Test the Migration

1. **Test locally first**:
```bash
# Run without @wip scenarios (default)
./run-bdd-tests-local.sh

# Run all tests including @wip
./run-bdd-tests-local.sh --all

# Run with verbose output
./run-bdd-tests-local.sh --verbose
```

2. **Test in CI**:
- Create a test branch
- Update `.github/workflows/bdd.yml` to use `bdd-simplified.yml`
- Verify both Linux and macOS runs succeed

## Workflow Comparison

### Old Approach (687 lines)
- Manually runs each test: `./bdd/bin/bdd_unit_compiler_basic`
- Custom `extract_test_counts` function
- Hardcoded test names require manual updates
- No @wip filtering
- Platform-specific test execution logic

### New Approach (10 lines in bdd.yml + implementation)
- `bdd.yml` simply calls `bdd-implementation.yml`
- Single script invocation: `./run-bdd-tests-local.sh --verbose`
- Built-in test aggregation
- Automatic test discovery
- @wip filtering by default
- Platform differences handled by the script

## Benefits

1. **Reduced Maintenance**: Changes to test logic only need to be made in one place
2. **Consistency**: Same behavior locally and in CI
3. **Flexibility**: Easy to add new options or change behavior
4. **Better Error Handling**: Script provides clear feedback about failures
5. **@wip Support**: Proper handling of work-in-progress scenarios

## Rollback Plan

If issues arise:
1. The original `bdd.yml` workflow remains unchanged
2. Can switch back by updating the workflow reference
3. Both approaches can coexist during transition

## Future Enhancements

1. **Parallel Test Execution**: Run test suites in parallel for faster CI
2. **Test Result Caching**: Cache successful test results
3. **Selective Testing**: Only run tests affected by changes
4. **Better Reporting**: Generate HTML reports with trends

## Conclusion

This migration simplifies the BDD test workflow while maintaining all existing functionality and adding new capabilities like @wip filtering. The unified approach ensures consistency between local development and CI environments.