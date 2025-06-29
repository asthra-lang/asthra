# Compiler Reference

This reference documents the command-line options for the Asthra compiler executable.

## Synopsis

```bash
asthra [options] <input-file>
```

## Description

The `asthra` compiler transforms Asthra source files (`.asthra`) into executable programs. It uses LLVM as its backend for code generation and optimization.

## Options

### Output Options

**`-o, --output <file>`**  
Specify the output file name. Default is `a.out` on Unix-like systems or `a.exe` on Windows.

```bash
asthra -o myprogram source.asthra
```

### Optimization Options

**`-O, --optimize <level>`**  
Set optimization level (0-3). Default is 2.
- `-O0`: No optimization (fastest compilation, best for debugging)
- `-O1`: Basic optimizations
- `-O2`: Standard optimizations (default, recommended)
- `-O3`: Aggressive optimizations (may increase code size)

```bash
asthra -O3 performance-critical.asthra
```

### Debug Options

**`-g, --debug`**  
Include debug information in the output. This enables source-level debugging with tools like `gdb` or `lldb`.

```bash
asthra -g -O0 debug-me.asthra
```

### Target Options

**`-t, --target <arch>`**  
Specify target architecture. Supported values:
- `x86_64`: 64-bit x86 processors
- `arm64` or `aarch64`: 64-bit ARM processors
- `wasm32`: WebAssembly 32-bit
- `native`: Use host architecture (default)

```bash
asthra --target wasm32 web-app.asthra
```

### Backend Options

**`-b, --backend <type>`**  
Select code generation backend. Currently only `llvm` is supported.

**`--emit <format>`**  
Specify output format:
- `llvm-ir`: LLVM IR text format (.ll)
- `llvm-bc`: LLVM bitcode (.bc)
- `asm`: Native assembly (.s)
- `obj`: Object file (.o)
- `exe`: Executable (default)

```bash
asthra --emit llvm-ir source.asthra  # Generate LLVM IR
asthra --emit asm source.asthra      # Generate assembly
asthra --emit obj source.asthra      # Generate object file
```

### Position Independent Executable (PIE) Options

**`--pie`**  
Force generation of position-independent executables. This explicitly enables PIE/PIC code generation, adding `-pie` and `-fPIE` flags during linking.

**`--no-pie`**  
Disable PIE generation. This explicitly disables PIE/PIC code generation, adding `-no-pie` and `-fno-PIE` flags during linking.

By default, the compiler uses platform-specific defaults for PIE. On modern Linux distributions (like Ubuntu 24.04), PIE is typically enabled by default for security. Use these flags when you need explicit control over PIE behavior.

```bash
# Force PIE (recommended for security-critical applications)
asthra --pie secure-app.asthra

# Disable PIE (may be needed for certain embedded or performance-critical scenarios)
asthra --no-pie embedded-app.asthra
```

### Library and Include Options

**`-I, --include <path>`**  
Add a directory to the include search path for imports.

**`-L, --library-path <path>`**  
Add a directory to the library search path for linking.

**`-l, --library <name>`**  
Link with the specified library.

```bash
asthra -I./include -L./lib -lcrypto main.asthra
```

### Standard Library Options

**`--no-stdlib`**  
Don't link the Asthra standard library. Useful for embedded systems or when providing a custom runtime.

```bash
asthra --no-stdlib embedded.asthra
```

### Coverage Options

**`--coverage`**  
Enable code coverage instrumentation. This adds profiling code to track which lines of code are executed during program runs.

```bash
asthra --coverage test-suite.asthra
./test-suite
llvm-profdata merge default.profraw -o coverage.profdata
llvm-cov show ./test-suite -instr-profile=coverage.profdata
```

### Informational Options

**`-v, --verbose`**  
Enable verbose output. Shows detailed compilation steps, including:
- Input and output files
- Target architecture
- Optimization level
- Compilation phases

```bash
asthra -v source.asthra
```

**`--version`**  
Display compiler version information and exit.

**`-h, --help`**  
Show help message with all available options.

### Deprecated Options

These options are maintained for backward compatibility but should not be used in new code:

**`--emit-llvm`**  
Deprecated. Use `--emit llvm-ir` instead.

**`--emit-asm`**  
Deprecated. Use `--emit asm` instead.

## Examples

### Basic Compilation

```bash
# Compile to default executable
asthra hello.asthra

# Compile with custom output name
asthra -o hello hello.asthra

# Compile with optimizations and debug info
asthra -O2 -g hello.asthra
```

### Cross-Compilation

```bash
# Compile for ARM64
asthra --target arm64 -o app-arm64 app.asthra

# Compile for WebAssembly
asthra --target wasm32 --emit obj -o app.wasm app.asthra
```

### Development Workflow

```bash
# Debug build with no optimization
asthra -g -O0 -o myapp-debug myapp.asthra

# Release build with full optimization
asthra -O3 -o myapp-release myapp.asthra

# Generate LLVM IR for inspection
asthra --emit llvm-ir -o myapp.ll myapp.asthra
```

### Security-Focused Compilation

```bash
# Build with PIE for ASLR support
asthra --pie -O2 -o secure-app secure-app.asthra

# Build with coverage for security testing
asthra --coverage --pie -g -o test-app test-app.asthra
```

### Library Development

```bash
# Compile to object file for library
asthra --emit obj -o mylib.o mylib.asthra

# Create position-independent code for shared library
asthra --emit obj --pie -o mylib-pic.o mylib.asthra
```

## Environment Variables

**`ASTHRA_PATH`**  
Directories to search for imported packages, separated by `:` on Unix or `;` on Windows.

**`ASTHRA_STDLIB`**  
Override the default path to the Asthra standard library.

**`ASTHRA_VERBOSE`**  
Set to `1` to enable verbose output by default.

## Exit Status

- `0`: Successful compilation
- `1`: Compilation error (syntax, semantic, or type errors)
- `2`: Invalid command-line arguments
- `3`: File not found or I/O error
- `4`: Internal compiler error

## See Also

- [Getting Started](getting-started.md) - Introduction to Asthra programming
- [Building Projects](building-projects.md) - Using the Asthra build system
- [Language Reference](syntax-reference.md) - Asthra language syntax
- [FFI Interop](ffi-interop.md) - Working with C libraries