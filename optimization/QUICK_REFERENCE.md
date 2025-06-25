# Modern Optimization Tools - Quick Reference

Quick command reference for Asthra's modern Clang-native optimization tools.

## Performance Optimization (PGO)

### Complete PGO Workflow
```bash
make pgo-optimize                    # Full PGO optimization workflow
```

### Individual PGO Steps
```bash
make pgo-generate                    # Build instrumented binary
make pgo-run                         # Collect profile data
make pgo-merge                       # Process profile data
make pgo-use                         # Build optimized binary
```

### macOS Instruments Integration
```bash
# Time profiling
instruments -t 'Time Profiler' ./bin/asthra-pgo-optimized [args]

# Memory profiling
instruments -t 'Allocations' ./bin/asthra-pgo-optimized [args]

# System trace
instruments -t 'System Trace' ./bin/asthra-pgo-optimized [args]

# Using the Instruments profiler script
./optimization/instruments-profiler.sh profile
./optimization/instruments-profiler.sh auto
```

## Memory & Thread Analysis (Sanitizers)

### Quick Development Build
```bash
make debug-sanitized                 # Build with recommended sanitizers
```

### Individual Sanitizers
```bash
make sanitizer-asan                  # AddressSanitizer (memory errors)
make sanitizer-tsan                  # ThreadSanitizer (data races)
make sanitizer-ubsan                 # UndefinedBehaviorSanitizer
make sanitizer-msan                  # MemorySanitizer (Clang only)
```

### Combined Analysis
```bash
make sanitizer-asan-ubsan           # Recommended combination
make sanitizer-comprehensive        # All available sanitizers
```

### Sanitizer Environment Variables
```bash
# AddressSanitizer options
export ASAN_OPTIONS="detect_leaks=1:abort_on_error=1:check_initialization_order=1"

# ThreadSanitizer options
export TSAN_OPTIONS="halt_on_error=1:abort_on_error=1"

# UndefinedBehaviorSanitizer options
export UBSAN_OPTIONS="print_stacktrace=1:abort_on_error=1"
```

## Hash Optimization (Enhanced Gperf)

### Enhanced Gperf Workflow
```bash
make gperf-keywords                  # Generate optimized hash functions
make gperf-test                      # Test generated hash function
make gperf-benchmark                 # Benchmark performance
make gperf-integrate                 # Show integration instructions
```

## Complete Analysis Workflows

### Development Workflow
```bash
# 1. Build with sanitizers for development
make debug-sanitized

# 2. Run tests with sanitized binary
./bin/asthra-asan-ubsan examples/hello.asthra

# 3. Optimize for performance
make pgo-optimize
```

### CI/CD Workflow
```bash
# Quick sanitizer check
make sanitizer-asan-ubsan

# Performance optimization
make pgo-optimize

# Hash optimization
make gperf-keywords
```

### Complete Analysis
```bash
# Run all modern optimization tools
make analyze-comprehensive
```

## Optimization Script Usage

### Using the Optimization Script
```bash
./optimization/optimize.sh profile   # PGO optimization
./optimization/optimize.sh memory    # Sanitizer analysis
./optimization/optimize.sh gperf     # Hash optimization
./optimization/optimize.sh all       # Complete workflow
./optimization/optimize.sh check     # Check tool availability
```

### Using Individual Scripts
```bash
# Instruments profiler (macOS)
./optimization/instruments-profiler.sh check
./optimization/instruments-profiler.sh profile
./optimization/instruments-profiler.sh auto

# Sanitizer runner
./optimization/sanitizer-runner.sh
```

## Platform-Specific Notes

### macOS (Primary Platform)
- Full Clang toolchain support
- Native Instruments integration
- All sanitizers available
- Optimal performance on Apple Silicon

### Linux (Secondary Platform)
- Clang support
- All sanitizers available
- Alternative profiling tools available

## Performance Benefits

### Execution Speed
- **Sanitizers**: 5-10x faster than traditional tools
- **PGO**: 10-20% performance improvements
- **Native execution**: No emulation overhead

### Development Experience
- **Fast feedback**: Quick analysis suitable for frequent use
- **Integrated workflow**: Works with existing build systems
- **Modern tooling**: GUI-based analysis and reporting

## Common Use Cases

### Finding Performance Bottlenecks
```bash
# 1. Build with PGO
make pgo-optimize

# 2. Profile with Instruments (macOS)
instruments -t 'Time Profiler' ./bin/asthra-pgo-optimized

# 3. Analyze hot paths in Instruments GUI
```

### Debugging Memory Issues
```bash
# 1. Build with AddressSanitizer
make sanitizer-asan

# 2. Run problematic code
./bin/asthra-asan problematic_input.asthra

# 3. Review sanitizer output for issues
```

### Checking Thread Safety
```bash
# 1. Build with ThreadSanitizer
make sanitizer-tsan

# 2. Run concurrent tests
./bin/asthra-tsan concurrent_test.asthra

# 3. Review race condition reports
```

## File Locations

### Build Artifacts
```
bin/
├── asthra-pgo-optimized         # PGO-optimized binary
├── asthra-asan                  # AddressSanitizer build
├── asthra-tsan                  # ThreadSanitizer build
├── asthra-ubsan                 # UBSan build
├── asthra-msan                  # MemorySanitizer build
└── asthra-asan-ubsan           # Combined ASan+UBSan build
```

### Analysis Reports
```
optimization/
├── pgo_profile_*.profdata       # PGO profile data
├── *_compiler_*.log            # Sanitizer analysis logs
├── instruments_*.trace         # Instruments trace files
├── asthra_keywords_hash.c       # Generated hash implementation
├── asthra_keywords_hash.h       # Generated hash header
└── sanitizer_reports_*/         # Sanitizer report directories
```

## Troubleshooting

### PGO Issues
- Ensure `llvm-profdata` is available
- Check profile data generation in `default.profraw`
- Verify instrumented binary runs successfully

### Sanitizer Issues
- Use debug builds for more accurate results
- Check environment variables for sanitizer options
- Review sanitizer documentation for specific error types

### Instruments Issues (macOS)
- Install Xcode command line tools
- Ensure binary has debug symbols
- Check system permissions for profiling

### Gperf Issues
- Ensure `gperf` is installed and available
- Check keyword extraction from source files
- Verify generated hash function compilation

## Cleaning Up

### Clean Optimization Artifacts
```bash
make clean-optimization          # Clean all optimization artifacts
make clean-gperf                # Clean only Gperf files
```

### Clean Test Artifacts
```bash
make clean-optimization-tests    # Clean optimization test artifacts
``` 
