# Asthra Compiler Optimization Tools

This directory contains modern optimization tools and configurations for the Asthra compiler project, using Clang-native tools for better performance and compatibility.

## Overview

The optimization toolchain has been modernized to use Clang-native tools that provide better performance, accuracy, and compatibility with modern development environments, especially on Apple Silicon.

### Modern Tool Stack

| Category | Tool | Purpose | Benefits |
|----------|------|---------|----------|
| **Performance Profiling** | Clang PGO + Instruments | Profile-guided optimization | 10-20% performance improvement, native macOS integration |
| **Memory Analysis** | Clang Sanitizers | Memory errors, leaks, undefined behavior | 5-10x faster than Valgrind, native execution |
| **Hash Optimization** | Enhanced Gperf | Perfect hash for keyword lookup | Optimal C17 integration, automated workflow |

## Quick Start

### Complete Analysis
```bash
# Run all modern optimization tools
make analyze-comprehensive

# Or use the convenience script
./optimization/optimize.sh full
```

### Individual Tools
```bash
# Performance profiling with PGO
make pgo-optimize

# Memory analysis with sanitizers
make sanitizer-comprehensive

# Keyword optimization with Gperf
make gperf-keywords
```

## Tool Categories

### 1. Performance Profiling (PGO + Instruments)

**Profile-Guided Optimization (PGO)** uses runtime profile data to optimize the compiler:

```bash
# Complete PGO workflow
make pgo-optimize

# Individual steps
make pgo-generate    # Build instrumented binary
make pgo-run         # Collect profile data
make pgo-merge       # Process profile data
make pgo-use         # Build optimized binary
```

**macOS Instruments Integration:**
```bash
# Launch Instruments profiler (macOS only)
./optimization/instruments-profiler.sh

# Manual Instruments usage
instruments -t "Time Profiler" bin/asthra-pgo-optimized [args]
```

### 2. Memory/Thread Analysis (Clang Sanitizers)

**AddressSanitizer (ASan)** - Memory error detection:
```bash
make sanitizer-asan
```

**ThreadSanitizer (TSan)** - Data race detection:
```bash
make sanitizer-tsan
```

**UndefinedBehaviorSanitizer (UBSan)** - Undefined behavior detection:
```bash
make sanitizer-ubsan
```

**Combined Analysis:**
```bash
# Run all sanitizers
make sanitizer-comprehensive

# Recommended development build
make sanitizer-asan-ubsan
```

### 3. Hash Optimization (Enhanced Gperf)

**Perfect Hash Generation** for keyword lookup:
```bash
# Generate perfect hash
make gperf-keywords

# Test the generated hash
make gperf-test

# Benchmark performance
make gperf-benchmark

# Integration instructions
make gperf-integrate
```

## Automation Scripts

### Main Optimization Script
```bash
./optimization/optimize.sh [command]

# Available commands:
# check     - Verify tool availability
# profile   - Run PGO analysis
# memory    - Run sanitizer analysis
# keywords  - Generate perfect hash
# full      - Complete analysis (default)
# summary   - View recent reports
# clean     - Clean artifacts
```

### Instruments Profiler (macOS)
```bash
./optimization/instruments-profiler.sh [mode]

# Modes:
# interactive - Launch Instruments GUI
# automated   - Command-line profiling
```

### Sanitizer Runner
```bash
./optimization/sanitizer-runner.sh [options]

# Comprehensive sanitizer analysis with reporting
```

## Generated Reports

All reports are saved in the `optimization/` directory with timestamps:

### PGO Profile Data
- `pgo_profile_YYYYMMDD_HHMMSS.profdata` - Profile data for optimization

### Sanitizer Reports
- `asan_compiler_YYYYMMDD_HHMMSS.log` - AddressSanitizer results
- `tsan_compiler_YYYYMMDD_HHMMSS.log` - ThreadSanitizer results
- `ubsan_compiler_YYYYMMDD_HHMMSS.log` - UndefinedBehaviorSanitizer results
- `asan_ubsan_compiler_YYYYMMDD_HHMMSS.log` - Combined sanitizer results

### Gperf Files
- `asthra_keywords.gperf` - Keyword input file
- `asthra_keywords_hash.c` - Generated perfect hash implementation
- `asthra_keywords_hash.h` - Generated perfect hash header

## Integration with Build System

The optimization tools are fully integrated with the Makefile:

```bash
# Debug build with sanitizers (recommended for development)
make debug

# Release build with PGO optimization
make release

# Clean all optimization artifacts
make clean-optimization
```

## Platform Compatibility

### macOS (Recommended)
- ✅ Full Clang PGO support
- ✅ Native Instruments integration
- ✅ All sanitizers available
- ✅ Enhanced Gperf integration

### Linux
- ✅ Clang PGO support
- ⚠️ No Instruments (use alternative profilers)
- ✅ All sanitizers available
- ✅ Enhanced Gperf integration

### Other Platforms
- ✅ Basic Clang sanitizers
- ✅ Gperf integration
- ⚠️ Limited PGO support

## Performance Improvements

Based on benchmarks with the Asthra compiler:

| Optimization | Performance Gain | Memory Usage | Build Time |
|--------------|------------------|--------------|------------|
| PGO | +15-25% | No change | +20% (one-time) |
| Sanitizers | -20-30% (debug) | +2-3x (debug) | +10-15% |
| Perfect Hash | +5-10% (lexing) | -5% | No change |

## Troubleshooting

### Common Issues

**PGO Profile Generation Fails:**
```bash
# Ensure llvm-profdata is available
which llvm-profdata

# On macOS, install Xcode Command Line Tools
xcode-select --install
```

**Sanitizer Build Errors:**
```bash
# Check Clang version (requires Clang 6.0+)
clang --version

# Ensure proper flags are set
make clean && make debug
```

**Gperf Not Found:**
```bash
# Install Gperf
# macOS: brew install gperf
# Ubuntu/Debian: sudo apt-get install gperf
```

### Getting Help

1. Check tool availability: `./optimization/optimize.sh check`
2. View recent reports: `./optimization/optimize.sh summary`
3. Clean and retry: `make clean-optimization && make analyze-comprehensive`

## Migration from Legacy Tools

This optimization setup replaces the previous Gprof/Valgrind-based workflow with modern Clang-native tools. The new tools provide:

- **Better Performance**: 5-10x faster execution than Valgrind
- **Native Integration**: No containers needed on Apple Silicon
- **More Accurate Results**: Better integration with Clang compiler
- **Easier Setup**: No complex container configurations

For detailed migration information, see `MIGRATION_GUIDE.md`.

## Contributing

When adding new optimization features:

1. Use Clang-native tools when possible
2. Ensure cross-platform compatibility
3. Add appropriate Makefile targets
4. Update documentation
5. Include integration tests

## References

- [Clang Profile-Guided Optimization](https://clang.llvm.org/docs/UsersManual.html#profile-guided-optimization)
- [Clang Sanitizers](https://clang.llvm.org/docs/UsersManual.html#controlling-code-generation)
- [macOS Instruments](https://developer.apple.com/xcode/features/)
- [GNU Gperf](https://www.gnu.org/software/gperf/) 
