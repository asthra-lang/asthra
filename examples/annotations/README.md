# Annotation Examples

This directory contains comprehensive examples demonstrating the proper usage of Asthra's annotation system for AI-human collaboration, security, and memory management.

## Overview

Asthra's annotation system provides semantic metadata that guides AI code generation, enables human review workflows, and provides compiler hints for optimization and safety. These examples show practical usage patterns for all annotation types.

## Example Files

### `human_review_examples.asthra`

Comprehensive demonstration of human review annotations across all priority levels:

- **Low Priority Examples** - Standard operations with well-established patterns
- **Medium Priority Examples** - Complex logic that benefits from human review
- **High Priority Examples** - Critical systems and security-sensitive code
- **Context-Specific Examples** - Domain-specific review contexts (crypto, concurrency, FFI, security)

**Key Learning Points**:
- How to choose appropriate priority levels
- When to use context specifications
- Proper error handling in reviewed code
- Complex business logic patterns

### `security_annotations.asthra`

Focused examples of security-critical annotations:

- **Constant Time Annotations** - Timing attack prevention
- **Volatile Memory Annotations** - Memory optimization control
- **Combined Security Annotations** - Maximum protection patterns
- **Security Best Practices** - Real-world security patterns

**Key Learning Points**:
- Preventing timing-based side-channel attacks
- Ensuring sensitive data is properly cleared from memory
- Combining multiple security annotations effectively
- Cryptographic operation patterns

### `ffi_annotations.asthra`

Comprehensive FFI annotation examples:

- **Transfer Full Annotations** - Full ownership transfer patterns
- **Transfer None Annotations** - No ownership transfer patterns
- **Borrowed Annotations** - Lifetime annotation patterns
- **Complex FFI Combinations** - Multiple annotation types in single functions
- **FFI Best Practices** - Error handling and safety patterns

**Key Learning Points**:
- Managing memory ownership across FFI boundaries
- Proper error handling in FFI code
- Safety patterns for C library integration
- Complex ownership transfer scenarios

## Usage Guidelines

### For AI Code Generators

When generating code with annotations, follow these patterns:

1. **Start with appropriate priority levels**:
   - `low` for simple, well-understood operations
   - `medium` for complex logic or multiple error paths
   - `high` for security-sensitive or system-critical operations

2. **Use context specifications when relevant**:
   - `context = "crypto"` for cryptographic operations
   - `context = "concurrency"` for parallel/async code
   - `context = "ffi"` for foreign function interfaces
   - `context = "security"` for input validation and sanitization

3. **Combine annotations appropriately**:
   ```asthra
   #[human_review(high, context = "crypto")]
   #[constant_time]
   #[volatile_memory]
   fn secure_operation() -> Result<void, string> {
       // Implementation with maximum security
   }
   ```

### For Human Reviewers

Use these examples to understand review priorities and focus areas:

#### High Priority Review Checklist
- [ ] Security analysis for vulnerabilities
- [ ] Error handling verification
- [ ] Resource management and cleanup
- [ ] Input validation completeness
- [ ] Cryptographic correctness (for crypto context)
- [ ] Concurrency safety (for concurrency context)
- [ ] Memory safety (for FFI context)

#### Medium Priority Review Checklist
- [ ] Logic correctness verification
- [ ] Major error path checking
- [ ] Performance consideration
- [ ] Code maintainability assessment

#### Low Priority Review Checklist
- [ ] Basic correctness spot-checking
- [ ] Style consistency verification

### For Compiler Integration

These examples demonstrate how annotations should be processed:

1. **Parse-time Processing** - All annotations are parsed as first-class constructs
2. **Semantic Analysis** - Annotation semantics are validated during compilation
3. **Code Generation** - Annotations influence compiler output for security and performance
4. **Metadata Export** - Annotation information is available to external tooling

## Compilation and Testing

To compile and test these examples:

```bash
# Compile all examples
asthra build docs/examples/annotations/

# Check annotation processing
asthra analyze --annotations docs/examples/annotations/ --output annotation-report.json

# Generate review report
asthra review-report --input docs/examples/annotations/ --format markdown
```

## Integration with Build Systems

Example build configuration for annotation processing:

```toml
# Asthra.toml
[build]
process_annotations = true
generate_review_reports = true

[annotations]
human_review_output = "review-report.json"
security_analysis = true
generate_documentation = true

[review_workflow]
high_priority_blocking = true
medium_priority_warning = true
context_specific_routing = true
```

## Best Practices Summary

### Annotation Placement
- Function-level annotations precede the function declaration
- Parameter-level annotations precede the parameter type
- Multiple annotations can be combined on the same declaration
- Order of annotations does not affect semantics

### Priority Assignment
1. **Low Priority**: Simple operations, well-established patterns, minimal risk
2. **Medium Priority**: Complex logic, multiple error paths, non-trivial algorithms
3. **High Priority**: Security-sensitive, financial, system-critical operations
4. **Critical Priority**: Life-safety, national security (future extension)

### Context Selection
- **crypto**: Cryptographic operations, key management, security primitives
- **concurrency**: Parallel processing, async operations, synchronization
- **ffi**: Foreign function interfaces, C library integration, unsafe operations
- **security**: Input validation, authentication, authorization, data sanitization

### Security Annotations
- Use `#[constant_time]` for operations that must execute in constant time
- Use `#[volatile_memory]` to prevent compiler optimizations on sensitive data
- Combine security annotations with human review for maximum protection
- Always clear sensitive data from memory, especially in error paths

### FFI Annotations
- Use `#[transfer_full]` when ownership is transferred to/from C code
- Use `#[transfer_none]` when no ownership transfer occurs
- Use `#[borrowed]` for temporary references with limited lifetime
- Always validate pointers and handle C error codes properly

## Performance Considerations

- `#[human_review]` has no runtime impact (compile-time only)
- `#[constant_time]` may prevent certain compiler optimizations
- `#[volatile_memory]` prevents memory access optimizations
- `#[ownership]` affects memory allocation strategy
- FFI annotations help optimize memory management across language boundaries

## Tooling Integration

These examples work with:

- **Static Analysis Tools**: Process annotations for enhanced analysis
- **IDE Plugins**: Highlight annotated functions with priority indicators
- **Documentation Generators**: Include annotation metadata in generated docs
- **Code Review Tools**: Automatically flag high-priority items for human review
- **CI/CD Pipelines**: Extract and process annotations for automated workflows

## Contributing

When adding new annotation examples:

1. Follow the established file organization pattern
2. Include comprehensive comments explaining the annotation usage
3. Provide both simple and complex examples for each annotation type
4. Ensure all examples compile successfully
5. Update this README with any new patterns or best practices

## See Also

- [Annotations & Semantic Tags](../../user-manual/annotations.md) - Complete annotation documentation
- [Security Features](../../user-manual/security.md) - Security-focused annotation usage
- [FFI Interoperability](../../user-manual/ffi-interop.md) - FFI annotation patterns
- [Building Projects](../../user-manual/building-projects.md) - Build system integration 
