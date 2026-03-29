# Asthra Development Roadmap

Development follows an example-driven approach. Each example builds on the previous, unlocking new language features incrementally.

| # | Example | Features Unlocked |
|---|---------|-------------------|
| 1 | `break_continue.ast` | `break` and `continue` in for loops |
| 2 | `floats.ast` | `f64` type, float literals, type conversions (`f64(x)`, `i32(y)`) |
| 3 | `structs.ast` | struct declarations, field access, struct literals |
| 4 | `methods.ast` | impl blocks, `self` parameter, method calls |
| 5 | `enums.ast` | enum declarations, enum constructors, match statements |
| 6 | `arrays.ast` | fixed arrays `[N]T`, slices `[]T`, `len()`, indexing |
| 7 | `option.ast` | `Option<T>`, if-let pattern matching |
| 8 | `result.ast` | `Result<T,E>`, error handling patterns |
| 9 | `tuples.ast` | tuple types, tuple literals, tuple element access (`.0`, `.1`) |
| 10 | `ffi.ast` | extern functions, unsafe blocks, pointer types |
| 11 | `constants.ast` | `const` declarations, compile-time expressions, `sizeof()` |
| 12 | `imports.ast` | `import` declarations, module system, `as` aliases |
| 13 | `generics.ast` | type parameters `<T>`, generic structs/enums, type args |
| 14 | `annotations.ast` | `#[...]` tags: human_review, security, ownership, semantic |
| 15 | `concurrency.ast` | `spawn`, `spawn_with_handle`, `await` |
| 16 | `multiline_strings.ast` | triple-quoted strings `"""..."""`, raw strings `r"""..."""` |
