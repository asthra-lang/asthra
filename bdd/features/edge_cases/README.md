# Negative Test Cases for Syntax Validation

This directory contains negative test cases designed to prevent syntax regressions and AI hallucination from other programming languages. These tests ensure that Asthra maintains its unique syntax and doesn't accidentally accept syntax from other languages.

## Purpose

When AI models (including code completion tools and language models) generate Asthra code, they often hallucinate syntax from more popular languages like Rust, Go, Python, TypeScript, etc. These negative tests help catch such mistakes early and provide clear error messages to guide users toward correct Asthra syntax.

## Test Files

### 1. `enum_variant_syntax.feature`
**Focus**: Enum variant construction and pattern matching
- ✅ Correct: `Status.Active` (dot notation)
- ❌ Wrong: `Status::Active` (Rust-style double colon)

### 2. `function_syntax_validation.feature`
**Focus**: Function declarations and parameters
- ✅ Correct: `fn name(param: Type) -> ReturnType`
- ❌ Wrong: 
  - Go-style: `fn name(param Type) ReturnType`
  - C-style: `ReturnType name(Type param)`
  - TypeScript optional: `fn name(param?: Type)`
  - Default parameters: `fn name(param: Type = default)`
  - Empty params: `fn name()` (must use `none`)
  - Mutable params: `fn name(mut param: Type)`

### 3. `struct_syntax_validation.feature`
**Focus**: Struct declarations and initialization
- ✅ Correct: Field separator with commas, visibility before field name
- ❌ Wrong:
  - Semicolons after fields (C-style)
  - `interface` keyword (TypeScript)
  - `pub`/`priv` after field name (Rust position)
  - Mutable fields with `mut`
  - Initialization with `=` instead of `:`
  - Missing struct name in literals
  - Empty struct without `none`

### 4. `array_syntax_validation.feature`
**Focus**: Array and slice syntax
- ✅ Correct: `[Type; size]` for fixed, `[Type]` for slices
- ❌ Wrong:
  - C-style: `arr[5]: Type`
  - Go-style: `[5]Type`
  - JavaScript: `Array(1, 2, 3)`
  - TypeScript: `Array<Type>`
  - Rust: `vec![...]`
  - Range with `..` instead of `:`

### 5. `type_annotation_validation.feature`
**Focus**: Variable declarations and type annotations
- ✅ Correct: `let name: Type = value`
- ❌ Wrong:
  - Go-style: `let name Type = value`
  - C-style: `Type name = value`
  - Using `var` instead of `let`
  - Using `const` for local variables
  - `auto` keyword
  - Nullable with `?` (use `Option<T>`)
  - Union types with `|`
  - References with `&`

### 6. `method_syntax_validation.feature`
**Focus**: Methods and associated functions
- ✅ Correct: Methods in `impl` blocks, `::` for associated functions
- ❌ Wrong:
  - Type params on `impl` block
  - Missing visibility on methods
  - Mutable `self`
  - Reference `&self` or `&mut self`
  - `static` keyword
  - Using `.` for associated functions
  - `this` instead of `self`
  - Methods outside `impl` blocks

### 7. `import_syntax_validation.feature`
**Focus**: Import and module declarations
- ✅ Correct: `import "path"` with optional `as alias`
- ❌ Wrong:
  - Python: `from ... import ...`
  - JavaScript: `import { ... } from ...`
  - Rust: `use ...`
  - C++: `#include`
  - Java: dotted paths without quotes
  - Wildcard imports
  - `module` or `namespace` keywords

## How to Use These Tests

1. **During Development**: Run these tests to ensure parser changes don't accidentally accept invalid syntax
2. **For AI Tools**: Use error messages from these tests to train AI models on correct Asthra syntax
3. **Documentation**: Reference these tests when documenting syntax differences from other languages

## Adding New Tests

When adding new negative test cases:

1. Identify common syntax patterns from other languages that differ from Asthra
2. Create scenarios that test rejection of incorrect syntax
3. Ensure error messages are helpful and suggest correct syntax
4. Include positive test cases to show the correct way
5. Tag with `@negative` to identify as negative tests

## Running Tests

```bash
# Run all negative syntax tests
ctest --test-dir build -R "edge_cases.*syntax"

# Run specific feature
ctest --test-dir build -R "enum_variant_syntax"
```