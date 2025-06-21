# Fixing Bugs Workflow

**Complete workflow for investigating and fixing bugs in Asthra**

This workflow guides you through the entire process of bug fixing, from initial reproduction to successful resolution and prevention.

## Overview

- **Purpose**: Investigate, fix, and prevent bugs in the Asthra compiler
- **Time**: 30 minutes - 4 hours (depending on complexity)
- **Prerequisites**: Debugging skills, understanding of relevant components
- **Outcome**: Bug fixed, tested, and prevented from recurring

## Workflow Steps

### Step 1: Bug Investigation (15-45 minutes)

#### 1.1 Reproduce the Issue
```bash
# First, confirm you can reproduce the bug
# Create a minimal test case that demonstrates the issue

# Example: Compiler crash
echo 'fn main() -&gt; i32 { problematic_code; }' &gt; bug_test.asthra
./bin/asthra bug_test.asthra
```

**Reproduction Checklist:**
- &#91; &#93; Can consistently reproduce the issue
- &#91; &#93; Have minimal test case that triggers the bug
- &#91; &#93; Understand the expected vs actual behavior
- &#91; &#93; Know which component is likely affected

#### 1.2 Gather Information
```bash
# Collect system information
uname -a
./bin/asthra --version
clang --version

# Enable debug output
export ASTHRA_DEBUG=1
export ASTHRA_VERBOSE=1

# Run with debug information
./bin/asthra bug_test.asthra 2>&amp;1 | tee debug_output.log
```

**Information to Collect:**
- **Error Messages**: Exact error text and codes
- **System Environment**: OS, compiler version, build configuration
- **Input Data**: Minimal code that triggers the issue
- **Expected Behavior**: What should happen instead
- **Frequency**: Always, sometimes, or specific conditions

#### 1.3 Categorize the Bug
**Crash Bugs:**
- Segmentation faults
- Assertion failures
- Infinite loops or hangs

**Compilation Errors:**
- Incorrect error messages
- False positives/negatives in error detection
- Parser or lexer failures

**Runtime Issues:**
- Incorrect code generation
- Memory leaks or corruption
- Performance regressions

**Usability Issues:**
- Confusing error messages
- Missing helpful diagnostics
- Poor user experience

### Step 2: Root Cause Analysis (15-60 minutes)

#### 2.1 Identify the Component
```bash
# Determine which component is failing
# Use compiler flags to isolate the issue

# Test lexer only
echo 'problematic_code' | ./bin/asthra --tokenize

# Test parser only
echo 'problematic_code' | ./bin/asthra --parse --ast

# Test semantic analysis only
echo 'problematic_code' | ./bin/asthra --check

# Test code generation only
echo 'problematic_code' | ./bin/asthra --compile --output test.c
```

#### 2.2 Use Debugging Tools

**For Crashes (Segmentation Faults):**
```bash
# Use GDB for detailed crash analysis
gdb ./bin/asthra
(gdb) run bug_test.asthra
# When it crashes:
(gdb) bt          # Get backtrace
(gdb) info locals # Check local variables
(gdb) print var   # Examine specific variables
```

**For Memory Issues:**
```bash
# Use AddressSanitizer
make clean &amp;&amp; make asan
./bin/asthra bug_test.asthra

# Use Valgrind for detailed memory analysis
make clean &amp;&amp; make debug
valgrind --tool=memcheck --leak-check=full \
         --show-leak-kinds=all --track-origins=yes \
         ./bin/asthra bug_test.asthra
```

**For Logic Issues:**
```bash
# Add debug prints to narrow down the issue
# Edit relevant source files to add debugging output

# Example: Adding debug prints to parser
// In src/parser/parser.c
printf("DEBUG: Parsing token type %d, value '%s'\n", 
       parser->current_token.type, parser->current_token.value);
```

#### 2.3 Analyze the Code Path
```bash
# Trace through the code to understand the bug
# Use grep to find relevant functions and data structures

# Find where error occurs
grep -r "error_message_text" src/

# Find function definitions
grep -r "function_name" src/

# Find data structure definitions
grep -r "struct_name" src/
```

### Step 3: Develop the Fix (30-120 minutes)

#### 3.1 Plan the Fix
**Before coding, consider:**
- **Root Cause**: What is the fundamental issue?
- **Scope**: How widespread is this problem?
- **Side Effects**: What else might this change affect?
- **Testing**: How will you verify the fix works?

#### 3.2 Implement the Fix

**Example: Fixing a Parser Bug**
```c
// src/parser/parser.c
// Before (buggy code):
static ASTNode* parse_expression(Parser *parser) {
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        // Bug: Not checking for NULL pointer
        return create_identifier_node(parser->current_token.value);
    }
    return NULL;
}

// After (fixed code):
static ASTNode* parse_expression(Parser *parser) {
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        // Fix: Add NULL check and proper error handling
        if (!parser->current_token.value) {
            parser_error(parser, "Invalid identifier token");
            return NULL;
        }
        return create_identifier_node(parser->current_token.value);
    }
    return NULL;
}
```

**Example: Fixing a Memory Leak**
```c
// src/analysis/semantic.c
// Before (leaky code):
static bool analyze_function(ASTNode *node, SemanticContext *ctx) {
    Symbol *symbol = symbol_create(node->name);
    symbol_table_insert(ctx->symbol_table, symbol);
    
    // Bug: Not freeing symbol on error path
    if (some_error_condition) {
        return false;  // Memory leak!
    }
    
    return true;
}

// After (fixed code):
static bool analyze_function(ASTNode *node, SemanticContext *ctx) {
    Symbol *symbol = symbol_create(node->name);
    
    if (some_error_condition) {
        symbol_free(symbol);  // Fix: Clean up on error
        return false;
    }
    
    symbol_table_insert(ctx->symbol_table, symbol);
    return true;
}
```

**Example: Fixing an Error Message**
```c
// src/analysis/semantic.c
// Before (unhelpful error):
static bool check_type_compatibility(Type *expected, Type *actual) {
    if (!types_compatible(expected, actual)) {
        error("Type error");  // Bug: Vague error message
        return false;
    }
    return true;
}

// After (helpful error):
static bool check_type_compatibility(Type *expected, Type *actual, 
                                   ASTNode *node) {
    if (!types_compatible(expected, actual)) {
        // Fix: Specific, actionable error message
        error_at_node(node, 
            "Type mismatch: expected '%s', got '%s'",
            type_to_string(expected), 
            type_to_string(actual));
        return false;
    }
    return true;
}
```

#### 3.3 Test the Fix
```bash
# Build with your fix
make clean &amp;&amp; make -j$(nproc)

# Test the original bug case
./bin/asthra bug_test.asthra
echo "Exit code: $?"

# Test that the fix doesn't break existing functionality
make test-curated
```

### Step 4: Comprehensive Testing (20-45 minutes)

#### 4.1 Create Regression Test
```c
// tests/regression/test_bug_fix_NNNN.c
#include <assert.h>
#include <stdio.h>
#include "../../src/parser/parser.h"

void test_bug_NNNN_reproduction() {
    // Test that the original bug is fixed
    Parser parser;
    parser_init(&amp;parser, "problematic_code");
    
    ASTNode *node = parse_expression(&amp;parser);
    
    // Should not crash and should handle error gracefully
    assert(node != NULL || parser.has_error);
    
    if (node) ast_node_free(node);
    parser_cleanup(&amp;parser);
    printf("✅ Bug NNNN regression test passed\n");
}

void test_bug_NNNN_edge_cases() {
    // Test related edge cases to ensure comprehensive fix
    // ... additional test cases
    printf("✅ Bug NNNN edge cases test passed\n");
}

int main() {
    test_bug_NNNN_reproduction();
    test_bug_NNNN_edge_cases();
    printf("All bug NNNN tests passed!\n");
    return 0;
}
```

#### 4.2 Integration Testing
```bash
# Create integration test files
mkdir -p tests/integration/bug_fixes/

# Test case that should work after fix
cat &gt; tests/integration/bug_fixes/bug_NNNN_fixed.asthra &lt;&lt; 'EOF'
fn main() -&gt; i32 {
    // Code that previously caused the bug
    let x = previously_problematic_code;
    return 0;
}
EOF

# Test case that should still catch real errors
cat &gt; tests/integration/bug_fixes/bug_NNNN_still_errors.asthra &lt;&lt; 'EOF'
fn main() -&gt; i32 {
    // Code that should still produce an error
    let x = actually_invalid_code;
    return 0;
}
EOF
```

#### 4.3 Memory Safety Testing
```bash
# Test with AddressSanitizer
make clean &amp;&amp; make asan
./bin/asthra tests/integration/bug_fixes/bug_NNNN_fixed.asthra

# Test with Valgrind
make clean &amp;&amp; make debug
valgrind --tool=memcheck --leak-check=full \
         ./bin/asthra tests/integration/bug_fixes/bug_NNNN_fixed.asthra

# Test for performance regressions
make clean &amp;&amp; make release
time ./bin/asthra large_test_file.asthra
```

### Step 5: Documentation &amp; Prevention (10-20 minutes)

#### 5.1 Document the Fix
```c
/**
 * @brief Parse expression with proper error handling
 * @param parser The parser instance
 * @return AST node or NULL on error
 * 
 * Fixed bug NNNN: Added NULL pointer check to prevent crash
 * when parsing malformed identifier tokens.
 * 
 * @see https://github.com/asthra-lang/asthra/issues/NNNN
 */
static ASTNode* parse_expression(Parser *parser) {
    // ... implementation with fix
}
```

#### 5.2 Update Error Handling
```c
// Add better error messages to prevent similar issues
static void parser_error_with_context(Parser *parser, const char *message) {
    fprintf(stderr, "Parse error at line %d, column %d: %s\n",
            parser->current_token.line,
            parser->current_token.column,
            message);
    
    // Show context around the error
    show_error_context(parser);
}
```

#### 5.3 Add Defensive Programming
```c
// Add assertions to catch similar issues early
static ASTNode* parse_expression(Parser *parser) {
    assert(parser != NULL);
    assert(parser->current_token.value != NULL);
    
    // ... rest of implementation
}
```

### Step 6: Code Review Preparation (10-15 minutes)

#### 6.1 Self-Review Checklist
- &#91; &#93; **Fix is minimal**: Only changes what's necessary
- &#91; &#93; **No side effects**: Doesn't break existing functionality
- &#91; &#93; **Well tested**: Includes regression test and edge cases
- &#91; &#93; **Good error messages**: Clear and actionable
- &#91; &#93; **Documented**: Code comments explain the fix
- &#91; &#93; **Consistent style**: Follows project coding standards

#### 6.2 Prepare Commit
```bash
# Stage your changes
git add src/ tests/

# Write descriptive commit message
git commit -m "Fix bug NNNN: Handle NULL pointer in parse_expression

- Add NULL check for token value before processing
- Improve error message to be more specific
- Add regression test to prevent recurrence
- Add edge case tests for similar scenarios

The bug occurred when parsing malformed identifier tokens
that had NULL values, causing a segmentation fault.

Fixes #NNNN"
```

### Step 7: Submit &amp; Follow-up (10-15 minutes)

#### 7.1 Create Pull Request
```markdown
## Bug Fix: Handle NULL pointer in parse_expression

### Problem
Compiler crashes with segmentation fault when parsing malformed 
identifier tokens that have NULL values.

### Root Cause
The `parse_expression` function was not checking for NULL pointer
before accessing `parser->current_token.value`.

### Solution
- Added NULL pointer check with proper error handling
- Improved error message to be more specific and helpful
- Added defensive programming with assertions

### Testing
- &#91;x&#93; Reproduces original bug before fix
- &#91;x&#93; Bug is fixed after changes
- &#91;x&#93; Regression test added
- &#91;x&#93; Edge cases tested
- &#91;x&#93; Memory safety verified with AddressSanitizer
- &#91;x&#93; No performance regression

### Files Changed
- `src/parser/parser.c` - Added NULL check and error handling
- `tests/regression/test_bug_fix_NNNN.c` - Regression test
- `tests/integration/bug_fixes/` - Integration test cases

Fixes #NNNN
```

#### 7.2 Monitor and Follow-up
```bash
# After PR is merged, verify fix in main branch
git checkout main
git pull upstream main
make clean &amp;&amp; make -j$(nproc)

# Test the fix one more time
./bin/asthra bug_test.asthra

# Clean up your feature branch
git branch -d fix/bug-NNNN
```

## Bug Categories &amp; Specific Approaches

### Compiler Crashes
**Common Causes:**
- NULL pointer dereferences
- Buffer overflows
- Stack overflows from infinite recursion
- Use after free

**Debugging Approach:**
1\. Use GDB to get exact crash location
2\. Use AddressSanitizer for memory issues
3\. Add assertions for invariants
4\. Check all pointer dereferences

### Logic Errors
**Common Causes:**
- Incorrect algorithm implementation
- Wrong conditional logic
- Off-by-one errors
- State management issues

**Debugging Approach:**
1\. Add debug prints to trace execution
2\. Use unit tests to isolate the issue
3\. Review algorithm against specification
4\. Test edge cases thoroughly

### Memory Issues
**Common Causes:**
- Memory leaks
- Double free
- Use after free
- Buffer overflows

**Debugging Approach:**
1\. Use Valgrind for detailed analysis
2\. Use AddressSanitizer for real-time detection
3\. Review memory management patterns
4\. Add RAII-style cleanup functions

### Performance Issues
**Common Causes:**
- Inefficient algorithms (O(n²) instead of O(n))
- Excessive memory allocations
- Poor cache locality
- Unnecessary work in hot paths

**Debugging Approach:**
1\. Use profiling tools (gprof, perf)
2\. Benchmark before and after changes
3\. Analyze algorithmic complexity
4\. Profile memory usage patterns

## Common Pitfalls &amp; Solutions

### Incomplete Fixes
**Problem**: Fixing symptoms instead of root cause
**Solution**: Always trace back to the fundamental issue

### Breaking Changes
**Problem**: Fix breaks existing functionality
**Solution**: Run comprehensive test suite before submitting

### Poor Error Messages
**Problem**: Fix doesn't improve user experience
**Solution**: Always improve error messages when fixing bugs

### Missing Tests
**Problem**: Bug could reoccur without detection
**Solution**: Always add regression tests for fixed bugs

## Success Criteria

Before considering the bug fix complete:

- &#91; &#93; **Root Cause**: Fundamental issue is identified and fixed
- &#91; &#93; **Reproduction**: Original bug no longer occurs
- &#91; &#93; **Testing**: Comprehensive test coverage including edge cases
- &#91; &#93; **Regression Prevention**: Test added to prevent recurrence
- &#91; &#93; **No Side Effects**: Existing functionality still works
- &#91; &#93; **Documentation**: Fix is properly documented
- &#91; &#93; **Code Quality**: Fix follows project standards

## Related Workflows

- **&#91;Adding New Features&#93;(adding-features.md)** - If bug fix requires new functionality
- **&#91;Testing Workflows&#93;(testing-workflows.md)** - Comprehensive testing strategies
- **&#91;Performance Optimization&#93;(performance-optimization.md)** - If bug is performance-related
- **&#91;Code Review Process&#93;(code-review.md)** - Getting your fix reviewed

---

**🐛 Bug Fixed Successfully!**

*Your bug fix is now ready for review. Remember that good bug fixes not only solve the immediate problem but also prevent similar issues from occurring in the future.* 