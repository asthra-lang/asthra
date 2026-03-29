# CLAUDE.md

## Mandatory Rules

- **Do NOT modify `grammar.txt`** — This file is the authoritative language specification. You may recommend changes, but must wait for explicit user approval before editing it.
- **Parser must conform to `grammar.txt`** — The parser (`src/parser.zig`) must always implement the grammar exactly as specified in `grammar.txt`. Any parser change must be verified against the grammar. If a conflict is found, flag it rather than deviating from the spec.
- **Don't break existing examples** — Any compiler change must keep all `examples/*.ast` files compiling and producing correct output. Run `bash tests/run_examples.sh` to verify.
- **Follow the example sequence** — New features must follow the roadmap order in `docs/roadmap.md`. Don't skip ahead.
- **New features require tests** — Every new compiler feature must include unit tests and an integration example in `examples/`.
- **Codegen must handle all AST nodes it accepts** — No silent no-ops. If the parser produces an AST node, codegen must either implement it or emit a clear "not yet implemented" error.

## Project Overview

Asthra is a compiled programming language with a Zig-based compiler and LLVM backend. The language grammar is defined in `grammar.txt` and is the authoritative specification for all language features.

## Build & Test

```sh
zig build                                      # Build (macOS Homebrew default)
zig build -Dllvm-prefix=/usr/lib/llvm-20       # Build with custom LLVM path
zig build test                                 # Run unit tests
bash tests/run_examples.sh                     # Run integration tests (all examples)
./zig-out/bin/asthra <file.ast> -o <output>    # Compile an Asthra source file
./zig-out/bin/asthra <file.ast> --emit-ir      # Print LLVM IR
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
- **LLVM 20** — configurable via `zig build -Dllvm-prefix=<path> -Dllvm-lib=<name>`. Defaults to `/opt/homebrew/opt/llvm@20/` and `LLVM-20`
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
- `log()` builtin for i32, string, f64, bool, char
- String literals
- Boolean literals
- Char type (`char`), char literals (`'a'`, `'\n'`), char-to-int conversion (`i32(c)`)
- Float types (`f64`) and type conversions (`f64(x)`, `i32(y)`)
- Struct declarations, struct literals, field access, field assignment
- Impl blocks, methods with `self`, method calls

- Enum declarations, enum constructors, match statements
- Fixed-size arrays (`[N]T`), array literals, indexing, `len()` builtin, element assignment
- `Option<T>` built-in type (`Option.Some(x)`, `Option.None()`, match destructuring)
- Slices (`[]T`) and Go-style slicing (`arr[1:3]`)
- Pointer types (`*mut T`, `*const T`), address-of (`&x`), dereference (`*p`), deref assignment (`*p = value`)
- Standard library (namespaced builtins, no import required):
  - `math`: `sqrt`, `pow`, `abs`, `min`, `max`, `floor`, `ceil`, `PI`, `E`
  - `str`: `upper`, `lower`, `contains`, `starts_with`, `ends_with`, `trim`, `to_int`, `to_float`, `from_int`, `from_float`
  - `io`: `print` (no newline), `println` (with newline), `read_line`
  - `os`: `getenv`, `clock`

## Not Yet Implemented

- Generic impl blocks (`impl Pair<T> { ... }`)
- Tuple patterns in match (`(a, b) => { ... }`)
- Nested enum patterns (`Option.Some(Result.Ok(val))`)
- String escape processing (`\n` → actual newline in output)
- `for` loops over arrays/slices (currently only `range()`)
- Method chaining on return values (`obj.a().b()`)
- Const binary expressions (`const X = 2 + 3;`)
- DWARF debug info for lldb/gdb
- Real pthread-based async runtime
- Type aliases (`type Name = Type;`)
- While loops (`while cond { ... }`)
- Closures / anonymous functions

## Development Strategy: Example-Driven

Development follows an example-driven approach. Each new feature is implemented by:

1. Write an example `.ast` program that uses the not-yet-implemented feature
2. Implement everything in the compiler (lexer, parser, codegen) to make it compile and run
3. Add unit tests for the new functionality
4. Add the example to `examples/` and verify with `bash tests/run_examples.sh`
5. Move to the next example

See [docs/roadmap.md](docs/roadmap.md) for the feature development sequence.

## CI/CD

- **CI** (`.github/workflows/ci.yml`) — builds, unit tests, and integration tests on Ubuntu + macOS for every push to `main` and PRs
- **Coverage** — kcov traces the compiler binary against all examples, uploads to Codecov
- **Release** (`.github/workflows/release.yml`) — triggered by `v*` tags, builds ReleaseFast binaries for Linux x86_64, Linux ARM64, macOS ARM64, publishes to GitHub Releases
- **Unit tests** — 50 tests across `token.zig`, `lexer.zig`, `parser.zig`, `diagnostics.zig` (run via `zig build test`)
- **Integration tests** (`tests/run_examples.sh`) — compiles and runs all `examples/*.ast` files, reports pass/fail
- **Pre-commit hook** (`.claude/settings.json`) — runs `zig build` before commits via Claude Code

To cut a release: `git tag v0.1.0 && git push --tags`
