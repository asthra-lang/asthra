# Building Asthra

## Prerequisites

The Asthra compiler requires the following dependencies:

- **CMake 3.20+**: Modern CMake for build configuration
- **C Compiler**: Clang with C17 support and LLVM
- **LLVM 18.0+**: Required for the LLVM backend (version 18.0 or later)
- **json-c**: Optional, will be downloaded automatically if not found

## Installing Dependencies

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required dependencies
brew install llvm cmake git
```

### Ubuntu/Debian

```bash
# Update package list
sudo apt update

# Install required dependencies
sudo apt install -y llvm-dev cmake git clang
```

### Fedora

```bash
# Install required dependencies
sudo dnf install -y llvm-devel cmake git clang
```

### CentOS/RHEL

```bash
# Enable EPEL repository
sudo yum install -y epel-release

# Install required dependencies
sudo yum install -y llvm-devel cmake git clang
```

## Building from Source

### Quick Build

```bash
# Clone the repository
git clone https://github.com/asthra-lang/asthra.git
cd asthra

# Build with CMake
cmake -B build
cmake --build build -j$(nproc)
```

### Build Options

#### Debug Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

#### Release Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

#### With Sanitizers
```bash
# AddressSanitizer
cmake -B build -DSANITIZER=Address
cmake --build build -j$(nproc)

# ThreadSanitizer
cmake -B build -DSANITIZER=Thread
cmake --build build -j$(nproc)
```

## Running Tests

```bash
# Build test executables first
cmake --build build --target build-tests

# Run all tests
ctest --test-dir build

# Run specific test category
ctest --test-dir build -L semantic -j8
```

## Troubleshooting

### LLVM Not Found

If CMake cannot find LLVM, you may need to specify the LLVM directory:

```bash
# macOS with Homebrew
cmake -B build -DLLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm

# Linux with custom LLVM installation
cmake -B build -DLLVM_DIR=/usr/local/llvm/lib/cmake/llvm
```

### Build Errors

If you encounter build errors:

1. Ensure all prerequisites are installed
2. Clean the build directory: `rm -rf build`
3. Try a fresh build with verbose output: `cmake --build build -j1 --verbose`

## Backend Selection

The LLVM backend is now the default. To use other backends:

```bash
# Use C backend
./build/asthra --backend c input.asthra

# Use Assembly backend
./build/asthra --backend asm input.asthra

# Use LLVM backend (default)
./build/asthra input.asthra
```

For more detailed build instructions and development setup, see the [Development Setup Guide](docs/contributor/quick-start/development-setup.md).