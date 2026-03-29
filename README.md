# Asthra

A compiled programming language with Rust/Go influences, built with a Zig compiler and LLVM backend.

## Features

- **Immutable by default** — `let` for immutable, `let mut` for mutable variables
- **Explicit visibility** — `pub` / `priv` required on all declarations
- **Strong typing** — `i32`, `i64`, `f64`, `bool`, `string`, `void`
- **Functions** — with parameters, return types, and calls
- **Control flow** — `if`/`else`, `for` loops with `range()`
- **Pattern matching** — `match` statements (grammar-ready)
- **Error handling** — `Result<T,E>` and `Option<T>` types (grammar-ready)
- **FFI** — `extern` functions and `unsafe` blocks (grammar-ready)
- **Concurrency** — `spawn` / `await` primitives (grammar-ready)
- **Predeclared identifiers** — `log()`, `range()`, `panic()`, `exit()` available without imports

## Quick Start

### Prerequisites

- [Zig](https://ziglang.org/) 0.15+
- LLVM 20 (`brew install llvm@20` on macOS)

### Build

```sh
zig build
```

### Compile and Run

```sh
./zig-out/bin/asthra examples/fibonacci.ast -o fibonacci
./fibonacci
```

### Options

```
asthra <source.ast> [-o output] [--emit-ir]
```

- `-o output` — set output executable name (default: `output`)
- `--emit-ir` — print LLVM IR to stderr

## Language Examples

### Hello World

```
package main;

pub fn main() -> void {
    log("Hello, Asthra!");
    return;
}
```

### Variables and Arithmetic

```
package main;

pub fn main() -> void {
    let x: i32 = 10;
    let y: i32 = 20;
    let z: i32 = x + y * 2;
    log(z);  // 50
    return;
}
```

### Mutability

```
package main;

pub fn main() -> void {
    let mut count: i32 = 0;
    count = count + 1;
    log(count);  // 1
    return;
}
```

### Functions

```
package main;

pub fn fibonacci(n: i32) -> i32 {
    if n <= 1 {
        return n;
    }
    let mut a: i32 = 0;
    let mut b: i32 = 1;
    for i in range(2, n + 1) {
        let temp: i32 = a + b;
        a = b;
        b = temp;
    }
    return b;
}

pub fn main() -> void {
    for i in range(10) {
        log(fibonacci(i));
    }
    return;
}
```

### FizzBuzz

```
package main;

pub fn main() -> void {
    for i in range(1, 21) {
        if i % 15 == 0 {
            log("FizzBuzz");
        } else {
            if i % 3 == 0 {
                log("Fizz");
            } else {
                if i % 5 == 0 {
                    log("Buzz");
                } else {
                    log(i);
                }
            }
        }
    }
    return;
}
```

## Project Structure

```
asthra/
├── build.zig          # Build configuration with LLVM linking
├── build.zig.zon      # Zig package manifest
├── grammar.txt        # Complete language grammar specification
├── src/
│   ├── main.zig       # CLI driver
│   ├── token.zig      # Token types and keyword map
│   ├── lexer.zig      # Single-pass tokenizer
│   ├── ast.zig        # AST node definitions
│   ├── parser.zig     # Recursive descent parser
│   ├── codegen.zig    # LLVM IR generation
│   └── diagnostics.zig # Error reporting
└── examples/          # Example Asthra programs
```

## Compiler Pipeline

```
Source (.ast) → Lexer → Parser → AST → LLVM IR → Object File → Executable
```

The compiler uses LLVM for optimization and native code generation, and invokes the system C compiler (`cc`) as a linker.

## License

TBD
