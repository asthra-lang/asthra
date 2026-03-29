# CLAUDE.md

## Project Overview

Asthra is a compiled programming language with a Zig-based compiler and LLVM backend. The language grammar is defined in `grammar.txt` and is the authoritative specification for all language features.

## Build & Test

```sh
zig build                    # Build the compiler
zig build test               # Run unit tests
./zig-out/bin/asthra <file.ast> -o <output>   # Compile an Asthra source file
./zig-out/bin/asthra <file.ast> --emit-ir     # Print LLVM IR
```

## Architecture

The compiler is a single-pass pipeline:

1. **Lexer** (`src/lexer.zig`) — streaming tokenizer, on-demand `next()`
2. **Parser** (`src/parser.zig`) — recursive descent, produces AST
3. **AST** (`src/ast.zig`) — arena-allocated node types
4. **CodeGen** (`src/codegen.zig`) — walks AST, emits LLVM IR via C API
5. **Diagnostics** (`src/diagnostics.zig`) — collects errors with source locations
6. **Driver** (`src/main.zig`) — CLI, orchestrates pipeline, invokes `cc` to link

No intermediate representation between AST and LLVM IR. LLVM handles optimization.

## Key Design Decisions

- **Zig 0.15 APIs** — uses unmanaged `ArrayList` (allocator passed per-call), `std.fs.File.stdout()`/`.stderr()` with `.deprecatedWriter()`
- **LLVM 20** — linked from `/opt/homebrew/opt/llvm@20/`, library name `LLVM-20`
- **`main()` ABI bridge** — Asthra `fn main() -> void` compiles to C `int main()` returning 0
- **`range()` intrinsic** — not a general iterator; lowered directly to a counting loop in codegen
- **`log()` builtin** — maps to `printf` with format string selected by argument type (i32, f64, string, bool)
- **Arena allocation** — all AST nodes allocated via `ArenaAllocator`, freed after codegen
- **Explicit error sets** — recursive parser/codegen functions use `ParseError`/`GenError` to satisfy Zig 0.15's stricter error set resolution
- **Null-terminated strings** — when passing Zig slices to LLVM C API, must `dupeZ` to get sentinel-terminated pointers

## File Conventions

- Source files use `.ast` extension
- All programs start with `package main;`
- Visibility modifiers (`pub`/`priv`) are required on all top-level declarations
- `return` statements always require an expression (use bare `return;` only for void functions)
- `none` keyword used for empty parameter lists and struct bodies

## Grammar

`grammar.txt` is the complete PEG grammar specification. When implementing new features, always refer to it for syntax rules. Key conventions:
- `void` is only a type (for functions returning nothing)
- `none` is only a structural marker (empty params, empty structs)
- Function parameters are always immutable (no `mut` allowed)
- Struct field mutability follows the containing variable's mutability

## Currently Implemented

- Package declarations
- Functions with params and return values
- `let` / `let mut` variable declarations and assignment
- Integer types (`i32`) and arithmetic (`+`, `-`, `*`, `/`, `%`)
- Comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
- Logical operators (`&&`, `||`, `!`)
- `if`/`else` statements (including nested)
- `for` loops with `range(end)` and `range(start, end)`
- `log()` builtin for i32, string, f64, bool
- String literals
- Boolean literals

## Not Yet Implemented

- Structs, enums, impl blocks
- Generics
- Pattern matching (`match`)
- `Result<T,E>` / `Option<T>`
- Slices and arrays
- Tuples
- FFI (`extern`, `unsafe`)
- Concurrency (`spawn`, `await`)
- Annotations (`#[...]`)
- `break` / `continue` (parsed but not yet wired in codegen)
- Float types and type conversions
- Import system
- Semantic analysis (type checking, mutability enforcement)
