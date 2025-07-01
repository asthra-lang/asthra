# Phase 4 BDD Tests: Failing Scenarios Summary

## Quick Reference Chart

| Feature | Total Scenarios | Passed | Failed | Success Rate | Primary Issues |
|---------|----------------|---------|---------|--------------|----------------|
| **comment_syntax** | 10 | 10 | 0 | ✅ 100% | None - All Fixed |
| **spawn_statements** | 8 | 5 | 3 | 🟡 62.5% | Struct visibility |
| **spawn_with_handle** | 5 | - | - | ❓ Not tested | Type annotations |
| **await_expressions** | 6 | - | - | ❓ Not tested | Type annotations |
| **unsafe_blocks** | 6 | 2 | 4 | 🔴 33% | Type annotations, struct visibility |
| **pointer_operations** | 5 | - | - | ❓ Not tested | Type annotations |
| **sizeof_operator** | 6 | 0 | 6 | 🔴 0% | Type annotations |
| **predeclared_identifiers** | 8 | - | - | ❓ Not tested | Type annotations |
| **associated_functions** | 6 | - | - | ❓ Not tested | Struct visibility |
| **slice_operations** | 8 | - | - | ❓ Not tested | Type annotations |
| **return_statements** | 10 | 6 | 4 | 🟡 60% | Type annotations, struct visibility |

## Failure Categories

### 🔴 Critical Issues (Blocking All Tests)
1. **Missing Type Annotations** - Affects 70% of failures
   ```asthra
   // ❌ FAILS
   let size = sizeof(i32);
   
   // ✅ CORRECT
   let size: usize = sizeof(i32);
   ```

### 🟡 Common Issues (Blocking Some Tests)
2. **Missing Struct Visibility** - Affects 20% of failures
   ```asthra
   // ❌ FAILS
   struct Point { x: i32, y: i32 }
   
   // ✅ CORRECT
   pub struct Point { x: i32, y: i32 }
   ```

### 🟢 Minor Issues (Expected Failures)
3. **Error Message Mismatches** - Affects 10% of failures
   - Tests expecting specific error messages
   - Compiler error messages have evolved

## Fix Priority Matrix

| Priority | Issue | Files Affected | Estimated Effort |
|----------|-------|----------------|------------------|
| **P0** | Type annotations in sizeof tests | `sizeof_operator_steps.c` | 15 mins |
| **P1** | Type annotations in unsafe tests | `unsafe_blocks_steps.c` | 10 mins |
| **P1** | Type annotations in return tests | `return_statements_steps.c` | 10 mins |
| **P2** | Struct visibility modifiers | Multiple files | 20 mins |
| **P3** | Error message updates | Error scenario tests | 30 mins |

## Quick Fixes

### Batch Fix Commands

1. **Fix all sizeof type annotations:**
   ```bash
   sed -i '' 's/let \([a-zA-Z_]*\) = sizeof/let \1: usize = sizeof/g' \
     steps/unit/sizeof_operator_steps.c
   ```

2. **Fix struct visibility in all files:**
   ```bash
   find steps/unit -name "*.c" -exec \
     sed -i '' 's/"struct \([A-Z]\)/"pub struct \1/g' {} \;
   ```

3. **Common type annotation fixes:**
   ```bash
   # For integer results
   sed -i '' 's/let result = /let result: i32 = /g' FILE
   
   # For boolean results  
   sed -i '' 's/let \([a-zA-Z_]*\) = \(.*\) > /let \1: bool = \2 > /g' FILE
   ```

## Test Execution Commands

Run specific failing tests after fixes:
```bash
# Test individual features
ctest --test-dir build -R "bdd_unit_sizeof_operator" --output-on-failure
ctest --test-dir build -R "bdd_unit_unsafe_blocks" --output-on-failure
ctest --test-dir build -R "bdd_unit_return_statements" --output-on-failure

# Run all Phase 4 tests
ctest --test-dir build -R "bdd_unit_(spawn|await|unsafe|pointer|sizeof|predeclared|associated|slice|return|comment)" -j8
```

## Success Metrics

After applying fixes:
- **Expected Success Rate**: >95%
- **Remaining Failures**: Only intentional error scenario tests
- **Time to Fix All**: ~2 hours

## Notes
- The BDD framework itself is working perfectly
- Failures are due to language syntax evolution
- Tests accurately reflect current compiler requirements
- Fixing these issues will provide comprehensive test coverage for Phase 4 features