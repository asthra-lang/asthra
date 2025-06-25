# LLVM Coverage Integration

This document describes the LLVM coverage support integrated into the Asthra compiler.

## Overview

The Asthra compiler now supports code coverage instrumentation using LLVM's built-in coverage tools (`llvm-cov` and `llvm-profdata`). This allows developers to measure test coverage and identify untested code paths.

**Current Status**: The coverage infrastructure is implemented but requires additional work in the LLVM backend to properly instrument the generated code. The `--coverage` flag is accepted and propagated through the compilation pipeline.

## Usage

### Building with Coverage

To enable coverage instrumentation, use the `--coverage` flag when compiling:

```bash
asthra --coverage -o program program.asthra
```

This will:
1. Add `-fprofile-instr-generate` and `-fcoverage-mapping` flags during compilation
2. Link with the necessary runtime libraries for coverage collection

### Running and Collecting Coverage

1. **Run your program** to generate raw profile data:
```bash
LLVM_PROFILE_FILE=program.profraw ./program
```

2. **Merge profile data** into an indexed format:
```bash
llvm-profdata merge -sparse program.profraw -o program.profdata
```

3. **Generate coverage reports**:

HTML Report:
```bash
llvm-cov show ./program -instr-profile=program.profdata -format=html -output-dir=coverage_html
```

Text Report:
```bash
llvm-cov report ./program -instr-profile=program.profdata
```

LCOV Format (for CI integration):
```bash
llvm-cov export ./program -instr-profile=program.profdata -format=lcov > coverage.lcov
```

## CMake Integration

The coverage system is integrated into the CMake build system:

### Enabling Coverage for Tests

```bash
cmake -B build -DENABLE_COVERAGE=ON
cmake --build build
```

### Coverage Targets

- `make coverage` - Generate full coverage report
- `make coverage-summary` - Display coverage summary
- `make coverage-check` - Check against threshold
- `make coverage-clean` - Clean coverage data
- `make coverage-open` - Open HTML report

Module-specific coverage:
- `make coverage-parser` - Parser module coverage
- `make coverage-codegen` - Code generation coverage
- `make coverage-analysis` - Semantic analysis coverage

## API Integration

### C API Functions

The following functions are available in `llvm_tools.h`:

```c
// Check if coverage tools are available
bool asthra_llvm_coverage_available(void);

// Merge raw profile data
AsthraLLVMToolResult asthra_llvm_merge_profile_data(
    const char **profraw_files,
    size_t num_files,
    const char *output_profdata
);

// Generate coverage report
AsthraLLVMToolResult asthra_llvm_coverage_report(
    const char *executable,
    const char *profdata,
    const char *format,  // "html", "text", "lcov", "json"
    const char *output_path,
    const char **source_filters,
    size_t num_filters
);

// Get coverage statistics
bool asthra_llvm_coverage_summary(
    const char *executable,
    const char *profdata,
    double *line_coverage,
    double *function_coverage,
    double *region_coverage
);
```

## Implementation Details

### Compiler Options

The coverage flag is stored in `AsthraCompilerOptions`:
```c
typedef struct AsthraCompilerOptions {
    // ... other fields ...
    bool coverage;  // Enable coverage instrumentation
} AsthraCompilerOptions;
```

### LLVM Tool Integration

Coverage instrumentation is handled by the LLVM tools module:
- When coverage is enabled, the compiler uses clang directly to compile LLVM IR with coverage instrumentation
- The flags `-fprofile-instr-generate` and `-fcoverage-mapping` are added during compilation
- The compilation pipeline detects coverage mode and generates a temporary `.ll` file for proper instrumentation

### Current Implementation

The coverage support is implemented in:
- `src/cli.c`: Accepts the `--coverage` flag
- `src/codegen/llvm_tools.c`: Special handling for coverage compilation using clang
- `src/compiler/compilation_pipeline.c`: Generates temporary IR files when coverage is enabled
- `cmake/Coverage.cmake`: CMake infrastructure for coverage targets

### Known Limitations

- The LLVM backend needs to generate source location metadata for proper coverage mapping
- Profile data generation requires the LLVM profiling runtime to be properly linked

### Coverage Workflow

1. **Compilation**: Add instrumentation during code generation
2. **Execution**: Program writes `.profraw` files
3. **Processing**: `llvm-profdata` merges raw data
4. **Reporting**: `llvm-cov` generates human-readable reports

## Best Practices

1. **Separate Coverage Builds**: Use a dedicated build directory for coverage
2. **Clean Between Runs**: Remove old `.profraw` files before new test runs
3. **Merge Incrementally**: For large test suites, merge profile data incrementally
4. **Filter Appropriately**: Exclude test code and third-party libraries from reports

## Troubleshooting

### No Coverage Data Generated

- Ensure the program was built with `--coverage`
- Check that `LLVM_PROFILE_FILE` is set when running
- Verify the program exits cleanly (coverage data is written on exit)

### Tools Not Found

- Install LLVM 18.0 or later
- Ensure `llvm-cov` and `llvm-profdata` are in PATH
- On macOS: `brew install llvm@18`
- On Linux: `apt install llvm-18`

### Low Coverage Numbers

- Check that all source files are included in the report
- Verify test execution covers expected code paths
- Use `-show-instantiations` for template coverage