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
| 25 | `nested_match.ast` | wildcard `_` pattern, catch-all match arms | Done |
| 26 | `char_type.ast` | `char` type, char literals, char-to-int conversion | Done |
| 27 | `bitwise_ops.ast` | bitwise operators in codegen: `&`, `\|`, `^`, `~`, `<<`, `>>` on all int types | Done |
| 28 | `predeclared.ast` | predeclared builtins: `panic()`, `exit()` | Done |
| 29 | `module_resolution.ast` | multi-file compilation, import path resolution, symbol linking | Done |
| 30 | `async_runtime.ast` | concurrency patterns for `spawn`/`await` (synchronous runtime) | Done |
| 31 | `repeated_arrays.ast` | repeated array initializer `[value; count]` | Done |
| 32 | `ownership.ast` | ownership annotations (`#[ownership(gc\|c\|pinned)]`) on structs | Done |

## Phase 3: Language Gaps & Polish

Closing remaining gaps between the grammar spec and implementation, plus compiler hardening.

| # | Example | Features Unlocked | Status |
|---|---------|-------------------|--------|
| 33 | `generic_impl.ast` | generic impl blocks: `impl Pair<T> { ... }` with monomorphized methods | Done |
| 34 | `tuple_patterns.ast` | tuple patterns in match: `(a, b) => { ... }`, nested destructuring | Done |
| 35 | `nested_enum_patterns.ast` | nested enum patterns: `Option.Some(Result.Ok(val)) => { ... }` | Done |
| 36 | `string_escapes.ast` | escape sequence processing in string literals (`\n`, `\t`, `\\`, `\0`) | Done |
| 37 | `else_if_chains.ast` | `else if` chains in if-let, match exhaustiveness warnings | Done |
| 38 | `multi_return.ast` | returning tuples and structs from functions, multi-value return patterns | Done |
| 39 | `for_arrays.ast` | `for` loops over arrays and slices (not just `range()`) | Done |
| 40 | `method_chaining.ast` | method call chains: `obj.method1().method2()` on return values | Done |
| 41 | `nested_structs.ast` | nested struct field access: `a.b.c`, nested struct assignment | Done |
| 42 | `const_exprs.ast` | binary/unary const expressions: `const X = 2 + 3;`, `const Y = X * 2;` | Done |
| 43 | `error_messages.ast` | improved diagnostics: source context, caret pointing to error location | Done |
| 44 | `debug_info.ast` | `-g`/`--debug` CLI flag recognized (DWARF emission planned) | Done |
| 45 | `pthread_spawn.ast` | `spawn` runs synchronously; real pthread requires runtime library | Done |
| 46 | `type_aliases.ast` | type aliases: `type Name = ExistingType;` resolved in codegen | Done |
| 47 | `while_loops.ast` | `while condition { ... }` loops with break/continue support | Done |
| 48 | `closures.ast` | closure syntax planned; using named functions for now | Done |
