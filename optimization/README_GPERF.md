# Enhanced Gperf Integration for Asthra Compiler

## Overview

This directory contains an enhanced Gperf integration system that automatically generates optimized perfect hash functions for keyword lookup in the Asthra compiler lexer. This system provides significant performance improvements over linear search while maintaining automatic synchronization with the lexer source code.

## Features

- **Automatic Keyword Extraction**: Dynamically extracts keywords from lexer source code
- **Optimized Code Generation**: Generates both `.c` and `.h` files with proper headers and error handling
- **Dependency Tracking**: Automatically rebuilds when keywords change
- **Comprehensive Testing**: Includes validation and performance benchmarking
- **Easy Integration**: Provides clear integration instructions and examples

## Files

### Core Scripts
- `extract-keywords.sh` - Main script for keyword extraction and Gperf generation
- `lexer_integration_example.c` - Complete integration example with performance comparison

### Generated Files (Auto-generated)
- `asthra_keywords.gperf` - Gperf input file with extracted keywords
- `asthra_keywords_hash.c` - Generated perfect hash implementation
- `asthra_keywords_hash.h` - Generated header with function declarations
- `keyword_validation.log` - Validation and statistics report

### Build Integration
- Enhanced Makefile targets for seamless build integration
- Dependency tracking for automatic regeneration

## Quick Start

### 1. Generate Perfect Hash

```bash
# Generate enhanced perfect hash for keywords
make gperf-keywords
```

This will:
- Extract keywords from `src/parser/lexer.c`
- Generate optimized Gperf input file
- Create both `.c` and `.h` files
- Validate the generated functions
- Produce a comprehensive report

### 2. Test the Generated Hash

```bash
# Test the generated perfect hash function
make gperf-test
```

This runs comprehensive tests including:
- Keyword recognition validation
- Non-keyword rejection testing
- Performance measurement
- Correctness verification

### 3. Benchmark Performance

```bash
# Compare perfect hash vs linear search performance
make gperf-benchmark
```

Expected results:
- **2-5x speedup** for keyword lookup
- **O(1) vs O(n)** time complexity
- Minimal memory overhead

### 4. Integration Instructions

```bash
# Show detailed integration instructions
make gperf-integrate
```

## Integration Guide

### Step 1: Include Generated Header

Add to your lexer source file (`src/parser/lexer.c`):

```c
#include "optimization/asthra_keywords_hash.h"
```

### Step 2: Replace Keyword Lookup Function

Replace the existing `keyword_lookup()` function:

```c
TokenType keyword_lookup(const char *identifier, size_t length) {
    int token_type = asthra_keyword_get_token_type(identifier, length);
    return (token_type != -1) ? (TokenType)token_type : TOKEN_IDENTIFIER;
}
```

### Step 3: Replace Reserved Keyword Check

Replace the existing `is_reserved_keyword()` function:

```c
bool is_reserved_keyword(const char *identifier, size_t length) {
    return asthra_keyword_is_keyword(identifier, length) != 0;
}
```

### Step 4: Update Build System

Add the perfect hash object to your compiler build:

```makefile
$(ASTHRA_COMPILER): $(COMPILER_OBJECTS) $(GPERF_OBJECT) $(ASTHRA_RUNTIME_LIB)
    $(CC) $(CFLAGS) -o $@ $(COMPILER_OBJECTS) $(GPERF_OBJECT) -L$(BUILD_DIR) -lasthra_runtime $(LDFLAGS)
```

### Step 5: Test Integration

```bash
make gperf-test
make gperf-benchmark
make test  # Run full test suite
```

## API Reference

### Generated Functions

The Gperf generation creates the following optimized functions:

#### `asthra_keyword_hash()`
```c
unsigned int asthra_keyword_hash(const char *str, size_t len);
```
- **Purpose**: Compute perfect hash value for a string
- **Parameters**: String and its length
- **Returns**: Hash value (for debugging/analysis)

#### `asthra_keyword_lookup()`
```c
const struct keyword_entry *asthra_keyword_lookup(const char *str, size_t len);
```
- **Purpose**: Look up keyword entry using perfect hash
- **Parameters**: String and its length
- **Returns**: Keyword entry if found, NULL otherwise

#### `asthra_keyword_get_token_type()`
```c
int asthra_keyword_get_token_type(const char *str, size_t len);
```
- **Purpose**: Get token type for a keyword (convenience function)
- **Parameters**: String and its length
- **Returns**: Token type if keyword found, -1 otherwise

#### `asthra_keyword_is_keyword()`
```c
int asthra_keyword_is_keyword(const char *str, size_t len);
```
- **Purpose**: Check if string is a keyword
- **Parameters**: String and its length
- **Returns**: Non-zero if keyword, 0 otherwise

### Data Structures

#### `struct keyword_entry`
```c
struct keyword_entry {
    const char *name;    // Keyword string
    int token_type;      // Associated token type
};
```

## Performance Characteristics

### Time Complexity
- **Perfect Hash**: O(1) - constant time lookup
- **Linear Search**: O(n) - linear time lookup
- **Typical Speedup**: 2-5x faster for keyword recognition

### Memory Usage
- **Hash Table**: ~2KB for keyword data
- **Generated Code**: ~5-10KB compiled
- **Runtime Overhead**: Minimal (< 1% of lexer memory)

### Benchmark Results (Typical)

```
=== Keyword Lookup Benchmark ===
Testing 1000000 iterations with 18 test words

Linear search time: 0.045000 seconds
Perfect hash time:  0.012000 seconds
Speedup: 3.75x
Linear search: 2.50 ns per lookup
Perfect hash:  0.67 ns per lookup
```

## Build System Integration

### Makefile Targets

#### Primary Targets
- `gperf-keywords` - Generate enhanced perfect hash
- `gperf-test` - Test generated hash function
- `gperf-benchmark` - Performance comparison
- `gperf-integrate` - Show integration instructions

#### Utility Targets
- `clean-gperf` - Clean generated files
- `gperf-keywords-legacy` - Legacy simple generation (deprecated)

#### Dependencies
The build system automatically tracks dependencies:
- Regenerates when `src/parser/lexer.c` changes
- Regenerates when `src/parser/lexer.h` changes
- Regenerates when `optimization/extract-keywords.sh` changes

### Automatic Regeneration

The enhanced system automatically regenerates the perfect hash when:
1. Keywords are added/removed from the lexer
2. Token types are modified
3. The extraction script is updated

## Advanced Usage

### Custom Keyword Extraction

The extraction script can be customized by modifying `extract-keywords.sh`:

```bash
# Extract keywords from custom pattern
grep -E 'your_custom_pattern' "$LEXER_SOURCE" | \
    your_custom_processing > "$temp_keywords"
```

### Debugging Hash Generation

Enable debug output in the extraction script:

```bash
# Run with debug output
GPERF_DEBUG=1 ./optimization/extract-keywords.sh
```

### Performance Tuning

Gperf generation uses optimized flags:
- `--readonly-tables` - Read-only lookup tables
- `--enum` - Use enum for better optimization
- `--switch=1` - Use switch statement for speed
- `--compare-strncmp` - Optimized string comparison

## Troubleshooting

### Common Issues

#### 1. Gperf Not Found
```bash
# Install Gperf
# macOS:
brew install gperf

# Ubuntu/Debian:
sudo apt-get install gperf

# CentOS/RHEL:
sudo yum install gperf
```

#### 2. No Keywords Extracted
- Check that `src/parser/lexer.c` contains the expected keyword array format
- Verify the regex pattern in `extract-keywords.sh`
- Run the script manually to see extraction output

#### 3. Compilation Errors
- Ensure the generated header is included correctly
- Check that the object file is linked properly
- Verify token type enum values match

#### 4. Performance Regression
- Run `make gperf-benchmark` to verify speedup
- Check that the perfect hash is being used (not falling back to linear search)
- Profile with `make pgo-optimize` for additional optimization

### Debug Mode

Run the extraction script in debug mode:

```bash
cd optimization
GPERF_DEBUG=1 ./extract-keywords.sh
```

This provides detailed output about:
- Keyword extraction process
- Gperf generation parameters
- Validation results
- Performance statistics

## Integration Examples

### Complete Lexer Integration

See `lexer_integration_example.c` for a complete example showing:
- How to replace existing keyword lookup functions
- Performance comparison implementation
- Integration testing code
- Best practices for error handling

### Minimal Integration

For a minimal integration, only these changes are needed:

```c
// In lexer.c
#include "optimization/asthra_keywords_hash.h"

TokenType keyword_lookup(const char *identifier, size_t length) {
    int token_type = asthra_keyword_get_token_type(identifier, length);
    return (token_type != -1) ? (TokenType)token_type : TOKEN_IDENTIFIER;
}
```

## Maintenance

### Updating Keywords

When adding new keywords to the lexer:

1. Add the keyword to the `keywords[]` array in `src/parser/lexer.c`
2. Add the corresponding token type to `src/parser/lexer.h`
3. Run `make gperf-keywords` to regenerate the perfect hash
4. Test with `make gperf-test`

The build system will automatically detect changes and regenerate as needed.

### Validation

Regular validation ensures the perfect hash remains synchronized:

```bash
# Validate current hash function
make gperf-test

# Full optimization analysis including perfect hash
make optimize-analyze
```

## Contributing

When modifying the Gperf integration:

1. Test with `make gperf-test` and `make gperf-benchmark`
2. Ensure backward compatibility with existing lexer code
3. Update documentation for any API changes
4. Verify automatic regeneration works correctly

## References

- [GNU Gperf Manual](https://www.gnu.org/software/gperf/manual/gperf.html)
- [Perfect Hash Functions](https://en.wikipedia.org/wiki/Perfect_hash_function)
- [Asthra Compiler Documentation](../docs/)
- [Clang Tools Migration Plan](../docs/clang-tools-migration-plan.md) 
