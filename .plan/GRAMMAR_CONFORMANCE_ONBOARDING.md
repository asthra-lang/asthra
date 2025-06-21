# Grammar Conformance Onboarding Guide

**Version**: 1.0  
**Created**: 2025-01-08  
**Target Audience**: New Team Members, Contributors, Maintainers  
**Prerequisites**: Basic understanding of Asthra language and PEG grammars  

## Welcome to Grammar Conformance Excellence

This guide provides everything you need to understand, maintain, and contribute to Asthra's world-class grammar conformance system. Our project has achieved **100% grammar compliance** across all documentation and examples, setting new industry standards for language implementation quality.

## Quick Start (5 Minutes)

### 1. Validate Your Environment
```bash
# Check if validation tools are working
python3 scripts/validate_single_file.py docs/spec/grammar.md

# Expected output: "✅ All examples are grammar-compliant!"
```

### 2. Run a Complete Validation
```bash
# Validate all documentation
python3 scripts/grammar_conformance_checker.py --scan-docs

# Expected result: 0 violations found
```

### 3. Test the Fix Pipeline
```bash
# Create a test file with violations (for learning)
echo 'fn test() -> void { return; }' > test_violation.asthra

# Run validation (should show violation)
python3 scripts/validate_single_file.py test_violation.asthra

# Apply fix
python3 scripts/fix_remaining_grammar_violations.py test_violation.asthra

# Validate again (should be clean)
python3 scripts/validate_single_file.py test_violation.asthra

# Clean up
rm test_violation.asthra
```

## Understanding Grammar Conformance

### What is Grammar Conformance?

Grammar conformance ensures that all Asthra code examples, documentation, and source files strictly adhere to the formal PEG grammar specification (currently v1.22). This provides:

- **AI Generation Reliability**: Consistent patterns for AI code generation
- **Developer Clarity**: Unambiguous syntax with only one way to express each construct
- **Parser Determinism**: Predictable compilation behavior
- **Documentation Quality**: Examples that actually work and compile

### Current Grammar Version: v1.22

Our grammar has evolved through systematic improvements:

| Version | Key Changes | Impact |
|---------|-------------|---------|
| **v1.22** | Import aliases (`as identifier`) | Conflict resolution, AI-friendly imports |
| **v1.21** | String interpolation removal | Simplified lexer, better AI generation |
| **v1.20** | Postfix expression ambiguity fix | Clear `::` usage semantics |
| **v1.19** | Void semantic overloading fix | `none` vs `void` distinction |
| **v1.18** | Optional elements simplification | Explicit syntax requirements |

## Violation Pattern Recognition

### Common Violation Types

#### 1. Empty Parameter Lists (HIGH PRIORITY)
```asthra
// ❌ VIOLATION: Missing (none) for empty parameters
fn main() -> i32 {
pub fn create_buffer() -> *mut u8 {

// ✅ CORRECT: Explicit (none) parameter list
fn main(none) -> i32 {
pub fn create_buffer(none) -> *mut u8 {
```

#### 2. String Interpolation (CRITICAL)
```asthra
// ❌ VIOLATION: String interpolation removed in v1.21
log("Worker {id} starting");
log("Result: {value}");

// ✅ CORRECT: Use concatenation
log("Worker " + id + " starting");
log("Result: " + value);
```

#### 3. Auto Type Annotations (MEDIUM PRIORITY)
```asthra
// ❌ VIOLATION: Auto types not allowed since v1.15
let result: auto = calculate();
let data: auto = [1, 2, 3];

// ✅ CORRECT: Explicit type annotations
let result: i32 = calculate();
let data: [3]i32 = [1, 2, 3];
```

#### 4. Return Statements (LOW PRIORITY)
```asthra
// ❌ VIOLATION: Inconsistent return syntax
return void;

// ✅ CORRECT: Use unit type for void returns
return;
```

#### 5. Empty Struct Content (HIGH PRIORITY)
```asthra
// ❌ VIOLATION: Void semantic overloading
struct Empty { void }

// ✅ CORRECT: Use none for structural absence
struct Empty { none }
```

### Recognition Techniques

#### Visual Patterns
- **Empty parentheses**: `()` after function names
- **Curly braces in strings**: `"text {variable} more"`
- **Auto keyword**: `: auto =` in variable declarations
- **Return void**: `return void;` statements
- **Void in structs**: `{ void }` content

#### Automated Detection
```bash
# Quick violation scan
grep -r "fn.*() ->" docs/
grep -r '"{[^}]*}"' docs/
grep -r ": auto =" docs/
grep -r "return void;" docs/
grep -r "{ void }" docs/
```

## Tools and Workflows

### Core Validation Tools

#### 1. Single File Validation
```bash
# Validate one file
python3 scripts/validate_single_file.py <file_path>

# Options:
# --verbose    : Show detailed violation information
# --json       : Output results in JSON format
# --quiet      : Only show summary
```

#### 2. Comprehensive Scanning
```bash
# Scan all documentation
python3 scripts/grammar_conformance_checker.py --scan-docs

# Scan specific directories
python3 scripts/grammar_conformance_checker.py --scan examples/ stdlib/

# Generate detailed reports
python3 scripts/grammar_conformance_checker.py --report-json violations.json
```

#### 3. Automated Fixing
```bash
# Fix markdown documentation
python3 scripts/fix_markdown_examples.py <file_path>

# Fix Asthra source files
python3 scripts/grammar_conformance_fixer.py <file_path>

# Targeted fixes for specific patterns
python3 scripts/fix_remaining_grammar_violations.py <file_path>
```

### Workflow for New Code

#### Before Committing
1. **Validate your changes**:
   ```bash
   python3 scripts/validate_single_file.py your_file.asthra
   ```

2. **Fix any violations**:
   ```bash
   python3 scripts/fix_remaining_grammar_violations.py your_file.asthra
   ```

3. **Re-validate**:
   ```bash
   python3 scripts/validate_single_file.py your_file.asthra
   ```

#### CI/CD Integration
Our pre-commit hooks automatically check grammar conformance:
```yaml
# .pre-commit-config.yaml
repos:
  - repo: local
    hooks:
      - id: grammar-conformance
        name: Grammar Conformance Check
        entry: python3 scripts/grammar_conformance_checker.py
        language: system
        files: \.(asthra|md)$
```

### Emergency Procedures

#### Rollback Process
If fixes introduce issues:
```bash
# Find your backup
ls -la ../backups/

# Restore from backup
cp ../backups/targeted_grammar_fixes/your_file.md docs/spec/your_file.md

# Verify restoration
python3 scripts/validate_single_file.py docs/spec/your_file.md
```

#### Manual Fix Guidelines
When automated tools fail:

1. **Identify the violation type** using validation output
2. **Apply the correct pattern** from the examples above
3. **Test incrementally** - fix one violation at a time
4. **Validate after each fix** to ensure correctness
5. **Create backup** before making changes

## Best Practices for Writing Grammar-Compliant Code

### Documentation Examples

#### DO: Write Clear, Compliant Examples
```asthra
// ✅ EXCELLENT: Clear, compliant documentation example
package example;

pub fn process_data(input: []u8) -> Result<ProcessedData, string> {
    let validator: DataValidator = DataValidator::new();
    let result: ValidationResult = validator.validate(input);
    
    match result {
        ValidationResult.Valid(data) => {
            let processed: ProcessedData = ProcessedData::from(data);
            return Result.Ok(processed);
        },
        ValidationResult.Invalid(error) => {
            return Result.Err("Validation failed: " + error);
        }
    }
}
```

#### DON'T: Use Deprecated or Ambiguous Patterns
```asthra
// ❌ POOR: Multiple violations
package example;

fn process_data(input: []u8) -> Result<ProcessedData, string> {  // Missing (none)
    let validator: auto = DataValidator::new();                   // Auto type
    let result: auto = validator.validate(input);                // Auto type
    
    match result {
        ValidationResult.Valid(data) => {
            let processed: auto = ProcessedData::from(data);      // Auto type
            return Result.Ok(processed);
        },
        ValidationResult.Invalid(error) => {
            return Result.Err("Validation failed: {error}");     // String interpolation
        }
    }
}
```

### Code Review Checklist

When reviewing code, check for:

- [ ] **Empty parameter lists use `(none)`**
- [ ] **No string interpolation patterns `{variable}`**
- [ ] **Explicit type annotations (no `: auto`)**
- [ ] **Consistent return statements**
- [ ] **Proper struct content (`none` not `void`)**
- [ ] **Visibility modifiers on all declarations**
- [ ] **Import aliases when needed**

## Troubleshooting Common Issues

### Issue: "String interpolation found"
**Cause**: Using `{variable}` syntax in strings  
**Solution**: Replace with concatenation
```asthra
// Before: "Hello {name}!"
// After:  "Hello " + name + "!"
```

### Issue: "Empty parameter list should use (none)"
**Cause**: Function declared with `()` instead of `(none)`  
**Solution**: Add explicit `none` parameter
```asthra
// Before: fn main() -> i32
// After:  fn main(none) -> i32
```

### Issue: "Auto type annotation should be explicit"
**Cause**: Using `: auto` type annotation  
**Solution**: Determine and specify the actual type
```asthra
// Before: let result: auto = calculate();
// After:  let result: i32 = calculate();
```

### Issue: "Empty struct should use { none }"
**Cause**: Using `{ void }` for empty struct content  
**Solution**: Replace with `{ none }`
```asthra
// Before: struct Empty { void }
// After:  struct Empty { none }
```

### Issue: Validation script fails
**Cause**: Missing dependencies or incorrect Python version  
**Solution**: 
```bash
# Check Python version (need 3.6+)
python3 --version

# Install dependencies if needed
pip3 install -r requirements.txt

# Verify script permissions
chmod +x scripts/validate_single_file.py
```

## Advanced Topics

### Custom Validation Rules
To add new violation patterns:

1. **Update detection patterns** in `scripts/grammar_conformance_checker.py`
2. **Add fix patterns** in `scripts/fix_remaining_grammar_violations.py`
3. **Test thoroughly** with various examples
4. **Update documentation** with new patterns

### Performance Optimization
For large codebases:
```bash
# Parallel validation
find . -name "*.asthra" | xargs -P 4 -I {} python3 scripts/validate_single_file.py {}

# Incremental validation (only changed files)
git diff --name-only | grep -E '\.(asthra|md)$' | xargs python3 scripts/validate_single_file.py
```

### Integration with IDEs
Configure your IDE to run validation on save:
```json
// VS Code settings.json
{
    "files.associations": {
        "*.asthra": "asthra"
    },
    "asthra.validation.onSave": true,
    "asthra.validation.command": "python3 scripts/validate_single_file.py"
}
```

## Getting Help

### Resources
- **Grammar Specification**: `docs/spec/grammar.md`
- **Maintenance Guide**: `docs/GRAMMAR_MAINTENANCE_GUIDE.md`
- **Implementation Plan**: `docs/GRAMMAR_CONFORMANCE_FIX_PLAN.md`
- **CI/CD Documentation**: `.github/workflows/grammar-conformance.yml`

### Support Channels
- **Team Chat**: #grammar-conformance channel
- **Issue Tracker**: GitHub issues with `grammar` label
- **Code Reviews**: Tag `@grammar-team` for expert review
- **Documentation**: Update this guide with new patterns you discover

### Contributing
To improve the grammar conformance system:

1. **Identify improvement opportunities** through daily usage
2. **Propose enhancements** via GitHub issues
3. **Submit pull requests** with comprehensive tests
4. **Update documentation** to reflect changes
5. **Share knowledge** with the team

## Success Metrics

Our grammar conformance system has achieved:

- ✅ **100% documentation compliance** (0 violations across all files)
- ✅ **Complete automation coverage** (all violation types automated)
- ✅ **Production-ready CI/CD** (automated prevention active)
- ✅ **Outstanding developer experience** (comprehensive tooling)
- ✅ **Industry-leading quality** (setting new standards)

Welcome to the team! With this guide, you're ready to maintain and contribute to Asthra's world-class grammar conformance system.

---

**Document Maintainer**: Grammar Conformance Team  
**Last Updated**: 2025-01-08  
**Next Review**: 2025-02-08 
