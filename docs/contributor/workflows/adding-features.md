# Adding New Features Workflow

**Complete workflow for implementing new features in Asthra**

This workflow guides you through the entire process of adding new features to the Asthra compiler, from initial planning to successful integration.

## Overview

- **Purpose**: Add new functionality to the Asthra compiler or language
- **Time**: 2-8 hours (depending on complexity)
- **Prerequisites**: Development environment setup, understanding of codebase architecture
- **Outcome**: Feature successfully integrated and tested

## Workflow Steps

### Step 1: Planning & Design (30-60 minutes)

#### 1.1 Requirements Analysis
```bash
# Before starting, clearly define:
# - What problem does this feature solve?
# - Who will use this feature?
# - How does it fit with existing features?
# - What are the acceptance criteria?
```

**Questions to Answer:**
- **User Need**: What specific problem does this solve?
- **Use Cases**: How will developers use this feature?
- **Scope**: What's included and what's not?
- **Compatibility**: Does this break existing code?
- **Performance**: What are the performance implications?

#### 1.2 Impact Analysis
```bash
# Analyze impact on major components:
grep -r "related_functionality" src/
```

**Components to Consider:**
- **Lexer**: New tokens or keywords needed?
- **Parser**: Grammar changes required?
- **Semantic Analysis**: Type checking implications?
- **Code Generation**: C code generation changes?
- **Runtime**: Runtime support needed?
- **Standard Library**: New library functions?

#### 1.3 Design Documentation
Create a brief design document:

```markdown
# Feature: [Feature Name]

## Summary
[Brief description of the feature]

## Motivation
[Why this feature is needed]

## Detailed Design
[How the feature will work]

## Implementation Plan
1. [Step 1]
2. [Step 2]
3. [Step 3]

## Testing Strategy
[How the feature will be tested]

## Alternatives Considered
[Other approaches that were considered]
```

### Step 2: Environment Preparation (10-15 minutes)

#### 2.1 Create Feature Branch
```bash
# Create and switch to feature branch
git checkout main
git pull upstream main
git checkout -b feature/your-feature-name

# Verify clean state
git status
```

#### 2.2 Verify Build Environment
```bash
# Ensure clean build
make clean && make -j$(nproc)

# Run baseline tests
make test-curated

# Verify compiler works
./bin/asthra --version
```

#### 2.3 Set Up Development Environment
```bash
# Enable debug mode for development
export ASTHRA_DEBUG=1
export ASTHRA_VERBOSE=1

# Set up any feature-specific environment
export ASTHRA_FEATURE_DEBUG=1  # If applicable
```

### Step 3: Implementation (1-6 hours)

#### 3.1 Lexer Changes (if needed)

**Add New Tokens:**
```c
// src/lexer/token.h
typedef enum {
    // ... existing tokens
    TOKEN_YOUR_NEW_TOKEN,
    // ... rest of tokens
} TokenType;
```

**Add Token Recognition:**
```c
// src/lexer/lexer.c
static Token scan_identifier_or_keyword(Lexer *lexer) {
    // ... existing code
    
    // Add new keyword recognition
    if (strcmp(lexer->current_token.value, "your_keyword") == 0) {
        lexer->current_token.type = TOKEN_YOUR_NEW_TOKEN;
    }
    
    // ... rest of function
}
```

**Test Lexer Changes:**
```bash
# Build and test lexer
make test-lexer

# Manual testing
echo 'your_keyword' | ./bin/asthra --tokenize
```

#### 3.2 Parser Changes (if needed)

**Update Grammar:**
```c
// src/parser/grammar.h
// Add new grammar rules or update existing ones

// Example: Adding a new statement type
typedef enum {
    // ... existing statement types
    STMT_YOUR_NEW_STATEMENT,
    // ... rest of statement types
} StatementType;
```

**Implement Parsing Logic:**
```c
// src/parser/parser.c
static ASTNode* parse_your_new_statement(Parser *parser) {
    // Consume expected tokens
    expect_token(parser, TOKEN_YOUR_NEW_TOKEN);
    
    // Parse statement components
    ASTNode *node = ast_node_create(STMT_YOUR_NEW_STATEMENT);
    
    // ... parsing logic
    
    return node;
}

// Add to main parsing switch
static ASTNode* parse_statement(Parser *parser) {
    switch (parser->current_token.type) {
        // ... existing cases
        case TOKEN_YOUR_NEW_TOKEN:
            return parse_your_new_statement(parser);
        // ... rest of cases
    }
}
```

**Test Parser Changes:**
```bash
# Build and test parser
make test-parser

# Manual testing
echo 'your_keyword example;' | ./bin/asthra --parse --ast
```

#### 3.3 Semantic Analysis Changes (if needed)

**Add Semantic Checks:**
```c
// src/analysis/semantic.c
static bool analyze_your_new_statement(ASTNode *node, SemanticContext *ctx) {
    // Type checking
    // Symbol resolution
    // Scope validation
    // Error reporting
    
    return true;  // or false if errors
}

// Add to main analysis switch
static bool analyze_statement(ASTNode *node, SemanticContext *ctx) {
    switch (node->type) {
        // ... existing cases
        case STMT_YOUR_NEW_STATEMENT:
            return analyze_your_new_statement(node, ctx);
        // ... rest of cases
    }
}
```

**Test Semantic Analysis:**
```bash
# Build and test semantic analysis
make test-semantic

# Manual testing with type errors
echo 'your_keyword invalid_usage;' | ./bin/asthra --check
```

#### 3.4 Code Generation Changes (if needed)

**Add Code Generation:**
```c
// src/codegen/codegen.c
static void generate_your_new_statement(ASTNode *node, CodegenContext *ctx) {
    // Generate appropriate C code
    fprintf(ctx->output, "/* Your feature implementation */\n");
    
    // ... code generation logic
}

// Add to main generation switch
static void generate_statement(ASTNode *node, CodegenContext *ctx) {
    switch (node->type) {
        // ... existing cases
        case STMT_YOUR_NEW_STATEMENT:
            generate_your_new_statement(node, ctx);
            break;
        // ... rest of cases
    }
}
```

**Test Code Generation:**
```bash
# Build and test code generation
make test-codegen

# Manual testing
echo 'your_keyword example;' | ./bin/asthra --compile --output test.c
cat test.c
```

#### 3.5 Runtime Support (if needed)

**Add Runtime Functions:**
```c
// runtime/src/your_feature.c
#include "asthra_runtime.h"

void asthra_your_feature_init(void) {
    // Initialize feature runtime support
}

void asthra_your_feature_cleanup(void) {
    // Cleanup feature resources
}
```

**Update Runtime Headers:**
```c
// runtime/include/asthra_runtime.h
// Add function declarations
void asthra_your_feature_init(void);
void asthra_your_feature_cleanup(void);
```

### Step 4: Testing (30-90 minutes)

#### 4.1 Unit Tests
```c
// tests/feature/test_your_feature.c
#include <assert.h>
#include <stdio.h>
#include "../../src/parser/parser.h"

void test_your_feature_parsing() {
    // Test basic parsing
    Parser parser;
    parser_init(&parser, "your_keyword example;");
    
    ASTNode *node = parse_statement(&parser);
    assert(node != NULL);
    assert(node->type == STMT_YOUR_NEW_STATEMENT);
    
    ast_node_free(node);
    parser_cleanup(&parser);
    printf("✅ Feature parsing test passed\n");
}

void test_your_feature_semantics() {
    // Test semantic analysis
    // ... semantic test logic
    printf("✅ Feature semantics test passed\n");
}

void test_your_feature_codegen() {
    // Test code generation
    // ... codegen test logic
    printf("✅ Feature codegen test passed\n");
}

int main() {
    test_your_feature_parsing();
    test_your_feature_semantics();
    test_your_feature_codegen();
    printf("All feature tests passed!\n");
    return 0;
}
```

#### 4.2 Integration Tests
```bash
# Create integration test files
mkdir -p tests/integration/your_feature/

# Basic functionality test
cat > tests/integration/your_feature/basic.asthra << 'EOF'
fn main() -> i32 {
    your_keyword example;
    return 0;
}
EOF

# Error handling test
cat > tests/integration/your_feature/error.asthra << 'EOF'
fn main() -> i32 {
    your_keyword invalid_usage;  // Should cause error
    return 0;
}
EOF
```

#### 4.3 Run Test Suite
```bash
# Build with your changes
make clean && make -j$(nproc)

# Run component tests
make test-lexer test-parser test-semantic test-codegen

# Run integration tests
make test-integration

# Run your specific feature tests
./tests/feature/test_your_feature_main

# Test compilation of feature examples
./bin/asthra tests/integration/your_feature/basic.asthra
```

#### 4.4 Performance Testing (if applicable)
```bash
# Build optimized version
make clean && make release

# Run performance benchmarks
make benchmark

# Profile your feature if performance-critical
make profile
./bin/asthra large_test_file.asthra
gprof ./bin/asthra gmon.out > profile.txt
```

### Step 5: Documentation (15-30 minutes)

#### 5.1 Code Documentation
```c
/**
 * @brief Parse a new statement type
 * @param parser The parser instance
 * @return AST node representing the statement, or NULL on error
 * 
 * This function parses the new statement syntax:
 * your_keyword expression;
 * 
 * @example
 * your_keyword variable_name;
 */
static ASTNode* parse_your_new_statement(Parser *parser) {
    // ... implementation
}
```

#### 5.2 User Documentation
```markdown
# Update language documentation
# Add to docs/language/features/your_feature.md

## Your Feature

### Syntax
```asthra
your_keyword expression;
```

### Description
[Describe what the feature does]

### Examples
```asthra
fn main() -> i32 {
    your_keyword example;
    return 0;
}
```

### Error Handling
[Describe error conditions and messages]
```

#### 5.3 Update Changelog
```markdown
# Add to CHANGELOG.md
## [Unreleased]

### Added
- New feature: your_keyword statement for [purpose]
```

### Step 6: Quality Assurance (15-30 minutes)

#### 6.1 Code Style Check
```bash
# Check code formatting
make format-check

# Run static analysis
make static-analysis

# Check for common issues
cppcheck --enable=all src/
```

#### 6.2 Memory Safety Check
```bash
# Build with AddressSanitizer
make clean && make asan

# Test with memory checking
./bin/asthra tests/integration/your_feature/basic.asthra

# Check for leaks
valgrind --leak-check=full ./bin/asthra tests/integration/your_feature/basic.asthra
```

#### 6.3 Cross-Platform Testing
```bash
# Test on different platforms if available
# macOS
make clean && make -j$(nproc)
make test-curated

# Linux
make clean && make -j$(nproc)
make test-curated

# Windows (if applicable)
make clean && make -j$(nproc)
make test-curated
```

### Step 7: Commit & Push (10-15 minutes)

#### 7.1 Prepare Commit
```bash
# Stage your changes
git add src/ tests/ docs/

# Check what you're committing
git status
git diff --cached
```

#### 7.2 Write Good Commit Message
```bash
git commit -m "Add [feature name] support

- Implement lexer support for new keyword
- Add parser rules for new statement type
- Include semantic analysis and type checking
- Generate appropriate C code
- Add comprehensive test coverage
- Update documentation

Fixes #123"
```

#### 7.3 Push Feature Branch
```bash
# Push to your fork
git push origin feature/your-feature-name
```

### Step 8: Create Pull Request (10-15 minutes)

#### 8.1 PR Description Template
```markdown
## Description
Brief description of the feature and its purpose.

## Changes Made
- [ ] Lexer changes for new tokens
- [ ] Parser updates for new grammar
- [ ] Semantic analysis implementation
- [ ] Code generation support
- [ ] Runtime support (if needed)
- [ ] Comprehensive test coverage
- [ ] Documentation updates

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Manual testing completed
- [ ] Performance impact assessed
- [ ] Memory safety verified

## Examples
```asthra
// Example usage of the new feature
your_keyword example;
```

## Breaking Changes
None / [List any breaking changes]

## Related Issues
Fixes #123
```

#### 8.2 Pre-PR Checklist
- [ ] All tests pass
- [ ] Code follows style guidelines
- [ ] Documentation is updated
- [ ] Commit message is clear
- [ ] No merge conflicts with main
- [ ] Feature is complete and ready for review

## Common Issues & Solutions

### Build Issues
**Problem**: Compilation errors after changes
```bash
# Solution: Clean rebuild
make clean && make -j1  # Single-threaded for better error messages
```

**Problem**: Linker errors
```bash
# Solution: Check for missing function definitions
grep -r "your_function_name" src/
```

### Test Failures
**Problem**: Existing tests fail
```bash
# Solution: Check if your changes break existing functionality
git diff main..HEAD
make test-curated
```

**Problem**: New tests don't pass
```bash
# Solution: Debug test logic
export ASTHRA_TEST_VERBOSE=1
./tests/feature/test_your_feature_main
```

### Parser Issues
**Problem**: Grammar conflicts
```bash
# Solution: Check grammar precedence and associativity
./bin/asthra --parse --debug your_test.asthra
```

**Problem**: AST construction errors
```bash
# Solution: Verify AST node creation and cleanup
valgrind ./bin/asthra --parse your_test.asthra
```

## Success Criteria

Before considering the feature complete, verify:

- [ ] **Functionality**: Feature works as designed
- [ ] **Testing**: Comprehensive test coverage
- [ ] **Documentation**: User and developer docs updated
- [ ] **Quality**: Code follows project standards
- [ ] **Performance**: No significant performance regression
- [ ] **Compatibility**: Doesn't break existing code
- [ ] **Review**: Code review feedback addressed

## Related Workflows

- **[Fixing Bugs](fixing-bugs.md)** - If issues are found during development
- **[Testing Workflows](testing-workflows.md)** - Comprehensive testing strategies
- **[Code Review Process](code-review.md)** - Getting your feature reviewed
- **[Performance Optimization](performance-optimization.md)** - If performance tuning is needed

---

**🚀 Feature Development Complete!**

*Your new feature is now ready for review and integration. Follow up on PR feedback and iterate as needed to get your feature merged.* 