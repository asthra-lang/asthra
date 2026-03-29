---
name: implement-feature
description: "Implement the next Asthra language feature from the roadmap using the example-driven development strategy. Use this skill whenever the user wants to add a new language feature, asks to implement the next roadmap item, says 'implement feature', mentions adding floats/structs/methods/enums/arrays/options/results/tuples/FFI, or references the development roadmap. Also use when the user says things like 'next feature', 'what's next on the roadmap', or 'let's add X to the language'."
user_invocable: true
---

# Implement Feature

Implement the next Asthra language feature following the example-driven development strategy. Accepts an optional argument: the feature number (1-10) or name from the roadmap. If no argument, auto-detect the next unimplemented feature.

## Step 0: Determine which feature to implement

1. Read `docs/roadmap.md` to get the feature sequence
2. List files in `examples/` to see which features already have examples
3. Match roadmap entries against existing examples — the first roadmap entry without a corresponding example is the next feature
4. If the user provided a feature number or name as an argument, use that instead
5. Read `grammar.txt` to understand the exact syntax rules for the target feature. Check if grammar defines features not yet in the roadmap — if so, note them for a roadmap update at the end

Display the feature name and what it unlocks, then proceed.

## Step 1: Write the example program

Create `examples/<feature_name>.ast` with a program that exercises the new feature. The example should:

- Start with `package main;`
- Have a `pub fn main() -> void { ... }` entry point
- Use `log()` to print results so output can be verified
- End with `return;`
- Cover the main use cases of the feature (basic usage, edge cases, combinations with existing features)
- Follow the syntax defined in `grammar.txt` exactly

Use the existing examples as style reference. Keep it clear and demonstrative — this example serves as both a test and documentation.

## Step 2: Explore the compiler to understand current state

Before writing code, read the relevant compiler files to understand what already exists:

- `src/token.zig` — check if tokens for the feature already exist
- `src/lexer.zig` — check if lexer handles relevant characters/keywords
- `src/ast.zig` — check existing AST node patterns
- `src/parser.zig` — check existing parse functions for similar features
- `src/codegen.zig` — check existing codegen patterns, LLVM API usage

Understanding what's already there prevents duplicate work and ensures consistency.

## Step 3: Implement the feature

Work through the compiler pipeline in order. Each layer builds on the previous one.

### 3a: Tokens (`src/token.zig`)

Add any new token types to the `Tag` enum. Follow the existing naming convention:
- Keywords: `keyword_struct`, `keyword_enum`, `keyword_match`, etc.
- Operators/punctuation: `dot`, `double_colon`, `arrow`, etc.

If the token is a keyword, add it to the `keywords` StaticStringMap in `token.zig`.
If it's a type keyword, update `isTypeKeyword()`.

### 3b: Lexer (`src/lexer.zig`)

If new tokens require lexer changes:
- New keywords are handled automatically via the keyword map (no lexer change needed)
- New operators/punctuation: add cases to the relevant `switch` in `readOperator()` or create new `readX()` methods
- New literal formats: add recognition logic

### 3c: AST (`src/ast.zig`)

Add new node types to the AST. Follow existing patterns:
- Statements go in the `Stmt` union (tagged union with data)
- Expressions go in the `Expr` union
- New declaration types go in `Decl`
- Supporting structures (like field lists, parameters) are separate structs
- Use `ExprIndex` for references to expressions (arena-allocated)
- Use `std.ArrayList` for variable-length collections

### 3d: Parser (`src/parser.zig`)

Implement parsing for the new feature using recursive descent. Key patterns:
- Each grammar production gets a `parseX()` function
- Use `self.expect(.token_type)` to consume required tokens
- Use `self.check(.token_type)` to lookahead without consuming
- Use `self.advance()` to consume the current token
- Return the appropriate AST node type
- Report errors via `self.diagnostics.report()`
- Use `ParseError` error set

The parser MUST conform to `grammar.txt` exactly. If you find a discrepancy between what seems right and what the grammar says, flag it to the user rather than deviating.

### 3e: Code Generation (`src/codegen.zig`)

Implement LLVM IR generation for the new AST nodes. Key patterns:
- Use the LLVM C API via the `c` namespace
- String arguments to LLVM must be null-terminated — use `self.allocator.dupeZ()` and defer free
- Track types with `TypeTag` enum — add new tags if needed
- Use `self.builder` for LLVM IR builder operations
- Use `self.context` for type creation
- Handle all new AST nodes — no silent no-ops allowed. Either implement the codegen or emit a clear "not yet implemented" error

Common LLVM patterns in this codebase:
- `c.LLVMBuildAlloca` — stack allocation
- `c.LLVMBuildStore` / `c.LLVMBuildLoad2` — memory operations
- `c.LLVMBuildBr` / `c.LLVMBuildCondBr` — branches
- `c.LLVMAppendBasicBlockInContext` — new basic blocks
- `c.LLVMBuildGEP2` — struct field / array element access
- `c.LLVMBuildCall2` — function calls
- `c.LLVMGetBasicBlockTerminator` — check if block is terminated

## Step 4: Add unit tests

Add tests in the relevant source files following existing patterns:

- Parser tests go at the bottom of `src/parser.zig` using `testParse()` helper
- Pattern: parse a minimal program, assert no errors, check AST structure
- Test both valid syntax and error cases where appropriate
- Use `const testing = @import("std").testing;`

## Step 5: Build and verify

Run these commands in sequence. If any step fails, fix the issue before continuing:

```sh
zig build                        # Must compile cleanly
zig build test                   # All unit tests must pass
bash tests/run_examples.sh       # ALL examples must pass, including the new one
```

Then manually compile and run the new example to verify correct output:

```sh
./zig-out/bin/asthra examples/<feature_name>.ast -o /tmp/feature_test && /tmp/feature_test
```

Verify the output matches expectations. Also try `--emit-ir` to inspect the generated LLVM IR if helpful for debugging.

## Step 6: Update documentation

After the feature is fully working:

1. **Update `CLAUDE.md`**: Move the feature from "Not Yet Implemented" to "Currently Implemented"
2. **Update `docs/roadmap.md`**: If grammar.txt defines features not in the roadmap, add them
3. **Update unit test count** in CLAUDE.md's CI/CD section if tests were added

## Step 7: Commit, push, and verify CI

Every feature must be committed, pushed, and verified green in CI before moving on.

1. **Stage and commit** the changes with a descriptive commit message:
   ```sh
   git add <changed files>
   git commit -m "Implement <feature_name>: <short description>"
   ```
   Only stage the files you changed — don't use `git add -A` or `git add .`.

2. **Push** to the remote:
   ```sh
   git push
   ```

3. **Watch GitHub Actions** — poll the CI workflow until it completes:
   ```sh
   gh run list --branch main --limit 1   # Find the latest run
   gh run watch <run-id>                  # Watch it live
   ```
   Alternatively, check status with:
   ```sh
   gh run list --branch main --limit 1 --json status,conclusion,name,databaseId
   ```

4. **If CI fails**:
   - Fetch the failure logs: `gh run view <run-id> --log-failed`
   - Diagnose the root cause (often platform differences between macOS and Ubuntu — e.g., LLVM paths, linker behavior)
   - Fix the issue locally, verify with `zig build && zig build test && bash tests/run_examples.sh`
   - Commit the fix and push again
   - Watch CI again — repeat until green

5. **CI must be green before the next feature**. Do NOT proceed to the next roadmap item if CI is failing. A red CI means the feature is not done.

## Step 8: Next feature (if implementing multiple)

If the user asked to implement multiple features (e.g., `/implement-feature` repeatedly or "implement the next 3 features"):

1. Confirm CI is green from Step 7
2. Go back to Step 0 and auto-detect the next unimplemented feature
3. Repeat the full cycle

Each feature is one atomic commit-push-verify cycle. Never batch multiple features into a single commit.

## Mandatory Rules (from CLAUDE.md)

These rules are non-negotiable:

- **Do NOT modify `grammar.txt`** — it is the authoritative language specification
- **Parser must conform to `grammar.txt`** — if a conflict is found, flag it rather than deviating
- **Don't break existing examples** — all previously-passing examples must continue to pass
- **New features require tests** — unit tests and an integration example
- **Codegen must handle all AST nodes** — no silent no-ops; implement or emit a clear error

## Feature Reference (from roadmap)

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

## Compiler Architecture Quick Reference

```
Source (.ast) --> Lexer (src/lexer.zig) --> tokens
             --> Parser (src/parser.zig) --> AST (src/ast.zig)
             --> CodeGen (src/codegen.zig) --> LLVM IR --> object file
             --> Driver (src/main.zig) --> cc link --> executable
```

Key design choices:
- **Zig 0.15 APIs**: unmanaged `ArrayList` (allocator passed per-call)
- **LLVM 20**: C API via `@cImport`
- **Arena allocation**: all AST nodes via `ArenaAllocator`
- **Explicit error sets**: `ParseError` / `GenError`
- **Null-terminated strings**: `dupeZ()` for LLVM C API calls
