# Asthra Programming Language Compiler

A modern, safe, and performant systems programming language with LLVM-based compiler infrastructure.

## 🚀 Quick Start

### Prerequisites

- **C17-compatible compiler** (Clang 15+ or GCC 11+ recommended)
- **CMake 3.20+**
- **LLVM 18.0+** (required)
- **Python 3.8+** (for tooling)
- **json-c** (optional, for AI server)

### Platform Support

- **macOS** (Apple Silicon & Intel)
- **Linux** (x86_64 & ARM64)
- **Windows** (MSVC - experimental)

### Installation

#### macOS
```bash
# Install dependencies
brew install cmake llvm@18 python3

# Clone and build
git clone https://github.com/asthra-lang/asthra.git
cd asthra
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake python3 llvm-18-dev clang-18

# Clone and build
git clone https://github.com/asthra-lang/asthra.git
cd asthra
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

#### Windows (MSVC)
```powershell
# Install dependencies via vcpkg or manually
# Ensure LLVM 18+ is installed and in PATH

# Clone and build
git clone https://github.com/asthra-lang/asthra.git
cd asthra
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

### Verify Installation
```bash
./build/bin/asthra --version
```

## 🔧 Building from Source

### Basic Build
```bash
cmake -B build
cmake --build build -j8
```

### Build with Optimizations
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

### Debug Build with Sanitizers
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DSANITIZER=Address
cmake --build build -j8
```

### Run Tests
```bash
# Build test executables first
cmake --build build --target build-tests

# Run all tests
ctest --test-dir build

# Run specific test category
ctest --test-dir build -L semantic -j8
```

## 📦 LLVM Integration

Asthra uses LLVM as its primary code generation backend. The compiler leverages:

- **LLVM-C API** for IR generation
- **LLVM tools** (llc, opt, clang) for optimization and linking
- **Multiple output formats**: LLVM IR (.ll), LLVM bitcode (.bc), assembly (.s), object files (.o), executables

### Output Formats

```bash
# Generate LLVM IR
asthra program.asthra --emit llvm-ir

# Generate assembly
asthra program.asthra --emit asm

# Generate object file
asthra program.asthra --emit obj

# Generate executable (default)
asthra program.asthra -o program
```

### Optimization Levels

```bash
# No optimization (default)
asthra program.asthra

# Basic optimizations
asthra program.asthra --optimize 1

# Standard optimizations
asthra program.asthra --optimize 2

# Aggressive optimizations
asthra program.asthra --optimize 3
```

## 🛠️ Development

### Building Documentation
```bash
cmake --build build --target docs
```

### Code Formatting
```bash
cmake --build build --target format
```

### Static Analysis
```bash
cmake --build build --target analyze
```

### Running Benchmarks
```bash
cmake --build build --target benchmark
./build/bin/asthra-benchmark
```

## 📚 Project Structure

```
asthra/
├── src/               # Compiler source code
│   ├── parser/       # Lexer and parser
│   ├── analysis/     # Semantic analysis
│   ├── codegen/      # LLVM code generation
│   └── compiler/     # Compilation pipeline
├── runtime/          # Runtime system
├── stdlib/           # Standard library
├── tests/            # Test suite
├── docs/             # Documentation
└── tools/            # Development tools
```

## 🤝 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for contribution guidelines.

## 📄 License

Licensed under the terms specified in [LICENSE](LICENSE).

## 🔗 Links

- [Language Specification](docs/spec/)
- [User Manual](docs/user-manual/)
- [Contributor Guide](docs/contributor/)
- [Standard Library](docs/stdlib/)