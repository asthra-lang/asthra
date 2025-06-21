# Your First Contribution Walkthrough

**Make your first meaningful contribution to Asthra in 30 minutes!**

This guide walks you through making your first pull request with a real example, from finding an issue to getting it merged.

## Prerequisites

Before starting, make sure you've completed the &#91;Quick Start Guide&#93;(README.md) and have:
- ✅ Asthra compiler building successfully
- ✅ Basic tests passing
- ✅ GitHub account set up

## Step 1: Find Your First Issue (5 minutes)

### Good First Issue Types

**Documentation Improvements** (Easiest)
- Fix typos or unclear explanations
- Add missing code examples
- Improve error messages
- Update outdated information

**Test Additions** (Beginner-friendly)
- Add test cases for edge cases
- Improve test coverage
- Add regression tests
- Create example programs

**Error Message Enhancements** (Great learning)
- Make error messages more helpful
- Add suggestions for common mistakes
- Improve error location reporting
- Add context to error messages

### Where to Look

```bash
# GitHub issue labels to search for:
good-first-issue        # Specifically marked for beginners
documentation          # Documentation improvements
testing                # Test-related work
help-wanted            # Community help needed
error-messages         # Error message improvements
```

**Example Search:**
1\. Go to &#91;GitHub Issues&#93;(https://github.com/asthra-lang/asthra/issues)
2\. Filter by label: `good-first-issue`
3\. Look for issues with clear descriptions
4\. Choose something that interests you

### Example Issue

For this walkthrough, let's use a real example:

**Issue #456: "Improve undefined variable error message"**
```
Current error message:
Error: undefined variable

Desired improvement:
Error: undefined variable 'myVar' at line 15
Hint: Did you mean 'myVariable'? (similar variable found)
```

## Step 2: Set Up Your Fork (5 minutes)

### Fork the Repository

1\. **Go to GitHub**: https://github.com/asthra-lang/asthra
2\. **Click "Fork"** in the top-right corner
3\. **Choose your account** as the destination

### Clone Your Fork

```bash
# Clone your fork (replace YOUR_USERNAME)
git clone https://github.com/YOUR_USERNAME/asthra.git
cd asthra

# Add upstream remote for staying updated
git remote add upstream https://github.com/asthra-lang/asthra.git

# Verify remotes
git remote -v
# Should show:
# origin    https://github.com/YOUR_USERNAME/asthra.git (fetch)
# origin    https://github.com/YOUR_USERNAME/asthra.git (push)
# upstream  https://github.com/asthra-lang/asthra.git (fetch)
# upstream  https://github.com/asthra-lang/asthra.git (push)
```

### Create Feature Branch

```bash
# Create and switch to feature branch
git checkout -b improve-undefined-variable-error

# Verify you're on the right branch
git branch
# Should show: * improve-undefined-variable-error
```

## Step 3: Understand the Issue (5 minutes)

### Reproduce the Problem

```bash
# Create a test file with undefined variable
echo 'fn main() -&gt; i32 {
    let x = undefined_var;
    return 0;
}' &gt; test_undefined.asthra

# Try to compile it
./bin/asthra test_undefined.asthra
```

**Current output:**
```
Error: undefined variable
```

**Goal:** Make this error more helpful by showing:
- Variable name
- Line number
- Suggestions for similar variables

### Find the Relevant Code

```bash
# Search for error message in codebase
grep -r "undefined variable" src/
# This will show you where the error is generated
```

**Expected result:**
```
src/analysis/semantic_core.c:245:    fprintf(stderr, "Error: undefined variable\n");
```

## Step 4: Implement Your Fix (10 minutes)

### Examine the Current Code

```bash
# Look at the file containing the error
vim src/analysis/semantic_core.c
# Or use your preferred editor
```

**Find the function around line 245:**
```c
// Current implementation (simplified)
static bool analyze_identifier_expression(ASTNode* node, SemanticContext* ctx) {
    const char* var_name = node->data.identifier.name;
    Symbol* symbol = symbol_table_lookup(ctx->symbol_table, var_name);
    
    if (!symbol) {
        fprintf(stderr, "Error: undefined variable\n");  // Line 245
        return false;
    }
    
    // ... rest of function
}
```

### Implement the Improvement

**Enhanced version:**
```c
static bool analyze_identifier_expression(ASTNode* node, SemanticContext* ctx) {
    const char* var_name = node->data.identifier.name;
    Symbol* symbol = symbol_table_lookup(ctx->symbol_table, var_name);
    
    if (!symbol) {
        // Improved error message with variable name and location
        fprintf(stderr, "Error: undefined variable '%s' at line %d\n", 
                var_name, node->location.line);
        
        // Try to find similar variable names
        const char* suggestion = find_similar_variable(ctx->symbol_table, var_name);
        if (suggestion) {
            fprintf(stderr, "Hint: Did you mean '%s'?\n", suggestion);
        }
        
        return false;
    }
    
    // ... rest of function
}
```

### Add Helper Function (if needed)

If `find_similar_variable` doesn't exist, you might need to add it:

```c
// Add this function above analyze_identifier_expression
static const char* find_similar_variable(SymbolTable* table, const char* target) {
    // Simple implementation: find variables with similar names
    // This is a simplified version - real implementation would be more sophisticated
    
    SymbolIterator iter;
    symbol_table_iterator_init(&amp;iter, table);
    
    const char* best_match = NULL;
    int best_distance = INT_MAX;
    
    Symbol* symbol;
    while ((symbol = symbol_table_iterator_next(&amp;iter)) != NULL) {
        if (symbol->type == SYMBOL_VARIABLE) {
            int distance = levenshtein_distance(target, symbol->name);
            if (distance &lt; best_distance &amp;&amp; distance &lt;= 2) {  // Max 2 character difference
                best_distance = distance;
                best_match = symbol->name;
            }
        }
    }
    
    return best_match;
}
```

**Note:** For a first contribution, you might implement a simpler version or ask for guidance on the implementation approach.

## Step 5: Test Your Changes (3 minutes)

### Build with Your Changes

```bash
# Clean build to ensure everything compiles
make clean &amp;&amp; make

# Check for compilation errors
echo $?  # Should be 0 if successful
```

### Test the Improvement

```bash
# Test with the same undefined variable file
./bin/asthra test_undefined.asthra
```

**Expected improved output:**
```
Error: undefined variable 'undefined_var' at line 2
```

### Test with Similar Variables

```bash
# Create a test with similar variable names
echo 'fn main() -&gt; i32 {
    let myVariable = 42;
    let x = myVar;  // Typo: should be myVariable
    return 0;
}' &gt; test_similar.asthra

./bin/asthra test_similar.asthra
```

**Expected output:**
```
Error: undefined variable 'myVar' at line 3
Hint: Did you mean 'myVariable'?
```

### Run Relevant Tests

```bash
# Run semantic analysis tests to ensure no regressions
make test-semantic

# If tests fail, investigate and fix
```

## Step 6: Commit Your Changes (2 minutes)

### Stage Your Changes

```bash
# Check what files you've modified
git status

# Add the modified files
git add src/analysis/semantic_core.c

# If you added new files or tests, add them too
git add tests/semantic/test_undefined_variables.c  # If you added tests
```

### Write a Good Commit Message

```bash
git commit -m "Improve undefined variable error messages

- Add variable name and line number to error output
- Suggest similar variable names when available
- Enhance user experience for common typos
- Improve debugging efficiency for developers

Fixes #456"
```

**Good commit message structure:**
- **First line**: Brief summary (50 characters or less)
- **Blank line**
- **Body**: Detailed explanation of what and why
- **Footer**: Reference to issue number

## Step 7: Push and Create Pull Request (5 minutes)

### Push to Your Fork

```bash
# Push your feature branch to your fork
git push origin improve-undefined-variable-error
```

### Create Pull Request

1\. **Go to GitHub**: Navigate to your fork
2\. **Click "Compare &amp; pull request"** (should appear automatically)
3\. **Fill out the PR template:**

```markdown
## Description
Improves undefined variable error messages by adding variable name, line number, and suggestions for similar variables.

## Changes Made
- Enhanced `analyze_identifier_expression()` to include variable name and line number in error messages
- Added `find_similar_variable()` helper function to suggest similar variable names
- Improved user experience for common typos and debugging

## Testing
- &#91;x&#93; Builds successfully
- &#91;x&#93; Semantic tests pass
- &#91;x&#93; Manual testing with undefined variables
- &#91;x&#93; Manual testing with similar variable names

## Screenshots/Examples

**Before:**
```
Error: undefined variable
```

**After:**
```
Error: undefined variable 'myVar' at line 3
Hint: Did you mean 'myVariable'?
```

## Related Issues
Fixes #456

## Checklist
- &#91;x&#93; Code follows project style guidelines
- &#91;x&#93; Tests pass
- &#91;x&#93; Documentation updated (if needed)
- &#91;x&#93; Commit message follows conventions
```

4\. **Click "Create pull request"**

## Step 8: Respond to Review (Ongoing)

### What to Expect

- **Automated checks**: CI will run tests automatically
- **Code review**: Maintainers will review your code
- **Feedback**: You may receive suggestions for improvements
- **Iteration**: You might need to make changes

### Responding to Feedback

```bash
# If you need to make changes:
git checkout improve-undefined-variable-error

# Make the requested changes
vim src/analysis/semantic_core.c

# Commit the changes
git add .
git commit -m "Address review feedback: improve error handling"

# Push the updates
git push origin improve-undefined-variable-error
```

**The PR will automatically update with your new commits.**

## Common First Contribution Pitfalls

### Build Issues
- **Forgot to clean build**: Always `make clean &amp;&amp; make` after changes
- **Missing dependencies**: Ensure all required tools are installed
- **Compiler warnings**: Fix all warnings, don't ignore them

### Code Quality Issues
- **No error handling**: Always handle edge cases
- **Memory leaks**: Clean up allocated memory
- **Inconsistent style**: Follow existing code patterns

### Testing Issues
- **Didn't run tests**: Always run relevant test categories
- **Broke existing functionality**: Check for regressions
- **No test coverage**: Add tests for new functionality

### Git/GitHub Issues
- **Wrong branch**: Make sure you're on your feature branch
- **Bad commit messages**: Follow the project's commit conventions
- **Forgot to link issue**: Always reference the issue number

## Success Checklist

Before submitting your PR, verify:

- &#91; &#93; **Code compiles** without warnings
- &#91; &#93; **Tests pass** (`make test-semantic` or relevant category)
- &#91; &#93; **Manual testing** confirms the fix works
- &#91; &#93; **No regressions** in existing functionality
- &#91; &#93; **Code follows style** guidelines
- &#91; &#93; **Commit message** is clear and follows conventions
- &#91; &#93; **PR description** explains the change clearly
- &#91; &#93; **Issue is referenced** in the PR

## Getting Help

**If you get stuck:**

1\. **Check existing documentation**
   - &#91;Troubleshooting Guide&#93;(../troubleshooting/README.md)
   - &#91;Development Environment Setup&#93;(development-setup.md)
   - &#91;Contributor Handbook&#93;(../HANDBOOK.md)

2\. **Ask for help**
   - Comment on the GitHub issue
   - Ask in GitHub Discussions
   - Join the Discord/Slack channel

3\. **Look at similar PRs**
   - Search for merged PRs with similar changes
   - Learn from successful contributions

## What Happens Next?

1\. **Review process** (1-3 days)
   - Automated tests run
   - Maintainers review your code
   - Feedback provided if needed

2\. **Iteration** (if needed)
   - Address review comments
   - Make requested changes
   - Push updates to your branch

3\. **Merge** 🎉
   - PR gets approved and merged
   - Your contribution becomes part of Asthra
   - You're officially an Asthra contributor!

4\. **Cleanup**
   ```bash
   # After merge, clean up your local branches
   git checkout main
   git pull upstream main
   git branch -d improve-undefined-variable-error
   ```

## Celebrating Your Success

**Congratulations!** You've just:
- ✅ Made your first open source contribution
- ✅ Improved the Asthra compiler for all users
- ✅ Learned the contribution workflow
- ✅ Joined the Asthra contributor community

**What's next?**
- Look for more issues to tackle
- Help other new contributors
- Explore more complex features
- Become a regular contributor

---

**🎉 Welcome to the Asthra contributor community!**

*Ready for more? Check out &#91;Development Workflows&#93;(../HANDBOOK.md#development-workflows) or find your next issue to tackle.* 