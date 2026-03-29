# Asthra Development Roadmap

Development follows an example-driven approach. Each example builds on the previous, unlocking new language features incrementally.

## Phase 1: Core Language (Complete)

| # | Example | Features Unlocked | Status |
|---|---------|-------------------|--------|
| 1 | `break_continue.ast` | `break` and `continue` in for loops | Done |
| 2 | `floats.ast` | `f64` type, float literals, type conversions (`f64(x)`, `i32(y)`) | Done |
| 3 | `structs.ast` | struct declarations, field access, struct literals | Done |
| 4 | `methods.ast` | impl blocks, `self` parameter, method calls | Done |
| 5 | `enums.ast` | enum declarations, enum constructors, match statements | Done |
| 6 | `arrays.ast` | fixed arrays `[N]T`, array literals, `len()`, indexing | Done |
| 7 | `option.ast` | `Option<T>`, match destructuring | Done |
| 8 | `result.ast` | `Result<T,E>`, error handling patterns | Done |
| 9 | `tuples.ast` | tuple types, tuple literals, tuple element access (`.0`, `.1`) | Done |
| 10 | `ffi.ast` | extern functions, unsafe blocks | Done |
| 11 | `constants.ast` | `const` declarations, compile-time expressions, `sizeof()` | Done |
| 12 | `imports.ast` | `import` declarations, module system, `as` aliases | Done |
| 13 | `generics.ast` | type parameters `<T>`, generic structs, monomorphization | Done |
| 14 | `annotations.ast` | `#[...]` tags: human_review, security, ownership, semantic | Done |
| 15 | `concurrency.ast` | `spawn`, `spawn_with_handle`, `await` (synchronous) | Done |
| 16 | `multiline_strings.ast` | triple-quoted strings `"""..."""`, raw strings `r"""..."""` | Done |

## Phase 2: Language Completeness

Features defined in `grammar.txt` but not yet fully implemented, plus semantic analysis.

| # | Example | Features Unlocked | Status |
|---|---------|-------------------|--------|
| 17 | `slices.ast` | `[]T` slice types, slice literals, Go-style slicing (`arr[1:3]`, `arr[:]`) | Done |
| 18 | `pointers.ast` | `*mut T`, `*const T` pointer types, `&` address-of, `*` dereference | Done |
| 19 | `if_let.ast` | `if let` pattern matching: `if let Some(x) = opt { ... } else { ... }` | Done |
| 20 | `string_ops.ast` | string concatenation (`+`), `len()` for strings, escape sequence processing | Done |
| 21 | `type_inference.ast` | type inference in `let` declarations (omit type annotation when inferable) | Done |
| 22 | `semantic_checks.ast` | semantic analysis pass: type checking, mutability enforcement, undefined variable detection | Done |
| 23 | `generic_enums.ast` | generic enums with type parameters, generic impl blocks | Done |
| 24 | `associated_funcs.ast` | associated functions via `::` syntax: `Type::new()`, `Type<T>::create()` | Done |
| 25 | `nested_match.ast` | nested patterns, tuple patterns in match, wildcard `_` pattern |
| 26 | `char_type.ast` | `char` type, char literals, char-to-int conversion | Done |
| 27 | `bitwise_ops.ast` | bitwise operators in codegen: `&`, `\|`, `^`, `~`, `<<`, `>>` on all int types |
| 28 | `predeclared.ast` | predeclared builtins: `panic()`, `exit()`, `args()`, `infinite()` |
| 29 | `module_resolution.ast` | multi-file compilation, import path resolution, symbol linking |
| 30 | `async_runtime.ast` | real async concurrency runtime for `spawn`/`await` (currently synchronous) |
| 31 | `repeated_arrays.ast` | repeated array initializer `[value; count]`, empty arrays `[none]` |
| 32 | `ownership.ast` | ownership annotations (`#[ownership(gc\|c\|pinned)]`) with runtime effect |
