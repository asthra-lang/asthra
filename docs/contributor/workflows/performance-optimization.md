# Performance Optimization Workflow

**Complete workflow for identifying and optimizing performance bottlenecks in Asthra**

This workflow guides you through systematic performance analysis and optimization, from initial profiling to measurable improvements.

## Overview

- **Purpose**: Identify, analyze, and optimize performance bottlenecks in the Asthra compiler
- **Time**: 1-8 hours (depending on complexity)
- **Prerequisites**: Understanding of profiling tools, performance analysis, and optimization techniques
- **Outcome**: Measurable performance improvements with maintained correctness

## Workflow Steps

### Step 1: Performance Baseline (20-30 minutes)

#### 1.1 Establish Current Performance
```bash
# Create comprehensive test suite for benchmarking
mkdir -p benchmarks/

# Small file benchmark
cat > benchmarks/small.asthra << 'EOF'
fn fibonacci(n: i32) -> i32 {
    if n <= 1 {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

fn main() -> i32 {
    let result = fibonacci(10);
    return result;
}
EOF

# Medium file benchmark
cat > benchmarks/medium.asthra << 'EOF'
// 500-line file with various language constructs
struct Point {
    x: f64,
    y: f64,
}

impl Point {
    fn new(x: f64, y: f64) -> Point {
        return Point { x: x, y: y };
    }
    
    fn distance(&self, other: &Point) -> f64 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
}

// ... continue with more complex code
EOF

# Large file benchmark (generate programmatically)
python3 << 'EOF'
with open('benchmarks/large.asthra', 'w') as f:
    f.write('// Large file with 10,000+ lines\n')
    for i in range(1000):
        f.write(f'''
fn function_{i}(param: i32) -> i32 {{
    let local_var = param * 2;
    if local_var > 100 {{
        return local_var - 50;
    }} else {{
        return local_var + 25;
    }}
}}
''')
    f.write('\nfn main() -> i32 { return function_999(42); }\n')
EOF
```

#### 1.2 Measure Baseline Performance
```bash
# Build optimized version for baseline
make clean && make release

# Time compilation of different file sizes
echo "=== Baseline Performance Measurements ==="
echo "Small file:"
time ./bin/asthra benchmarks/small.asthra -o /tmp/small.out
echo "Medium file:"
time ./bin/asthra benchmarks/medium.asthra -o /tmp/medium.out
echo "Large file:"
time ./bin/asthra benchmarks/large.asthra -o /tmp/large.out

# Memory usage baseline
echo "=== Memory Usage Baseline ==="
/usr/bin/time -v ./bin/asthra benchmarks/large.asthra -o /tmp/large.out 2>&1 | grep -E "(Maximum resident|User time|System time)"
```

#### 1.3 Document Baseline Metrics
```bash
# Create performance tracking file
cat > benchmarks/performance_log.md << 'EOF'
# Asthra Compiler Performance Log

## Baseline Measurements ($(date))

### Compilation Times
- Small file (50 lines): X.XXs
- Medium file (500 lines): X.XXs  
- Large file (10,000+ lines): X.XXs

### Memory Usage
- Peak memory usage: XXX MB
- User time: X.XXs
- System time: X.XXs

### System Configuration
- CPU: $(lscpu | grep "Model name" | cut -d: -f2 | xargs)
- RAM: $(free -h | grep Mem | awk '{print $2}')
- OS: $(uname -a)
- Compiler: $(clang --version | head -1)

EOF
```

### Step 2: Performance Profiling (30-60 minutes)

#### 2.1 CPU Profiling with Perf
```bash
# Build with debug symbols for profiling
make clean && make profile

# Profile CPU usage during compilation
perf record -g --call-graph=dwarf ./bin/asthra benchmarks/large.asthra -o /tmp/large.out

# Generate detailed report
perf report --stdio > profile_report.txt

# Generate flame graph (if available)
perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg

# Analyze hotspots
echo "=== Top CPU Hotspots ==="
perf report --stdio | head -50
```

#### 2.2 Memory Profiling with Valgrind
```bash
# Profile memory usage patterns
valgrind --tool=massif --detailed-freq=1 \
         ./bin/asthra benchmarks/large.asthra -o /tmp/large.out

# Generate memory usage graph
ms_print massif.out.* > memory_profile.txt

# Profile memory allocations
valgrind --tool=callgrind \
         ./bin/asthra benchmarks/large.asthra -o /tmp/large.out

# Analyze allocation patterns
callgrind_annotate callgrind.out.* > allocation_profile.txt
```

#### 2.3 Custom Profiling with Timing
```bash
# Add timing instrumentation to source code
# Example: Profile lexer performance
cat > src/profiling/timer.h << 'EOF'
#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <stdio.h>

typedef struct {
    struct timespec start;
    struct timespec end;
    const char* name;
} Timer;

static inline void timer_start(Timer* timer, const char* name) {
    timer->name = name;
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

static inline void timer_end(Timer* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end);
    double elapsed = (timer->end.tv_sec - timer->start.tv_sec) + 
                    (timer->end.tv_nsec - timer->start.tv_nsec) / 1e9;
    printf("PROFILE: %s took %.6f seconds\n", timer->name, elapsed);
}

#define PROFILE_BLOCK(name) \
    Timer _timer; \
    timer_start(&_timer, name); \
    for(int _i = 0; _i < 1; _i++, timer_end(&_timer))

#endif
EOF

# Add profiling to key functions
# Example: Profile lexer tokenization
```

#### 2.4 Analyze Profiling Results
```bash
# Identify performance bottlenecks
echo "=== Performance Analysis Summary ==="
echo "1. CPU Hotspots (from perf):"
grep -A 10 "Overhead.*Command" profile_report.txt

echo "2. Memory Hotspots (from massif):"
grep -A 5 "peak" memory_profile.txt

echo "3. Allocation Patterns (from callgrind):"
head -20 allocation_profile.txt
```

### Step 3: Bottleneck Analysis (30-45 minutes)

#### 3.1 Categorize Performance Issues

**CPU-Bound Issues:**
- Inefficient algorithms (O(n²) instead of O(n))
- Excessive string operations
- Redundant computations
- Poor loop optimization

**Memory-Bound Issues:**
- Excessive memory allocations
- Memory fragmentation
- Poor cache locality
- Memory leaks affecting performance

**I/O-Bound Issues:**
- Inefficient file reading
- Excessive system calls
- Poor buffer management

#### 3.2 Deep Dive Analysis
```bash
# Analyze specific functions identified as hotspots
# Example: If parser is a hotspot

# Count function calls
grep -r "parse_expression" src/ | wc -l

# Analyze algorithmic complexity
# Look for nested loops, recursive calls, etc.
grep -A 20 -B 5 "parse_expression" src/parser/parser.c

# Check for memory allocation patterns
grep -r "malloc\|calloc\|realloc" src/parser/
```

#### 3.3 Identify Optimization Opportunities
```c
// Example analysis of a performance hotspot
// src/parser/parser.c - parse_expression function

// Current implementation (potentially slow):
static ASTNode* parse_expression(Parser *parser) {
    // Problem 1: String comparison in hot path
    if (strcmp(parser->current_token.value, "identifier") == 0) {
        // Problem 2: Unnecessary string duplication
        char *name = strdup(parser->current_token.value);
        
        // Problem 3: Recursive calls without memoization
        ASTNode *node = create_node();
        node->name = name;
        
        // Problem 4: Linear search through symbol table
        Symbol *symbol = symbol_table_lookup(parser->symbols, name);
        
        return node;
    }
    return NULL;
}

// Optimization opportunities identified:
// 1. Use token types instead of string comparison
// 2. Avoid unnecessary string duplication
// 3. Add memoization for recursive parsing
// 4. Use hash table for symbol lookup
```

### Step 4: Implement Optimizations (60-180 minutes)

#### 4.1 Algorithm Optimizations

**Example: Optimize Symbol Table Lookup**
```c
// Before: Linear search O(n)
Symbol* symbol_table_lookup(SymbolTable *table, const char *name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;
}

// After: Hash table lookup O(1) average case
#define SYMBOL_TABLE_SIZE 1024

typedef struct {
    Symbol *buckets[SYMBOL_TABLE_SIZE];
    int count;
} HashSymbolTable;

static unsigned int hash_string(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % SYMBOL_TABLE_SIZE;
}

Symbol* symbol_table_lookup(HashSymbolTable *table, const char *name) {
    unsigned int index = hash_string(name);
    Symbol *symbol = table->buckets[index];
    
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    return NULL;
}
```

**Example: Optimize String Operations**
```c
// Before: Excessive string operations
static bool is_keyword(const char *str) {
    return strcmp(str, "fn") == 0 ||
           strcmp(str, "let") == 0 ||
           strcmp(str, "if") == 0 ||
           strcmp(str, "else") == 0 ||
           strcmp(str, "return") == 0;
}

// After: Perfect hash or trie for keywords
static const char* keywords[] = {
    "fn", "let", "if", "else", "return", NULL
};

static bool is_keyword(const char *str) {
    // Use perfect hash or binary search
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Even better: Use token types instead of string comparison
typedef enum {
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER
} TokenType;
```

#### 4.2 Memory Optimizations

**Example: Reduce Memory Allocations**
```c
// Before: Frequent small allocations
ASTNode* create_node(NodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    return node;
}

// After: Memory pool allocation
#define POOL_SIZE 4096

typedef struct {
    char memory[POOL_SIZE];
    size_t offset;
} MemoryPool;

static MemoryPool node_pool = {0};

ASTNode* create_node(NodeType type) {
    if (node_pool.offset + sizeof(ASTNode) > POOL_SIZE) {
        // Handle pool exhaustion
        return NULL;
    }
    
    ASTNode *node = (ASTNode*)(node_pool.memory + node_pool.offset);
    node_pool.offset += sizeof(ASTNode);
    
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    return node;
}
```

**Example: Improve Cache Locality**
```c
// Before: Poor cache locality
typedef struct ASTNode {
    NodeType type;
    char *name;           // Pointer to heap
    struct ASTNode *left;
    struct ASTNode *right;
    SourceLocation *loc;  // Another pointer to heap
} ASTNode;

// After: Better cache locality
typedef struct ASTNode {
    NodeType type;
    char name[32];        // Inline small strings
    struct ASTNode *left;
    struct ASTNode *right;
    SourceLocation loc;   // Inline location data
} ASTNode;
```

#### 4.3 I/O Optimizations

**Example: Optimize File Reading**
```c
// Before: Character-by-character reading
char read_char(FILE *file) {
    return fgetc(file);
}

// After: Buffered reading
#define BUFFER_SIZE 8192

typedef struct {
    FILE *file;
    char buffer[BUFFER_SIZE];
    size_t pos;
    size_t size;
} BufferedReader;

char read_char(BufferedReader *reader) {
    if (reader->pos >= reader->size) {
        reader->size = fread(reader->buffer, 1, BUFFER_SIZE, reader->file);
        reader->pos = 0;
        if (reader->size == 0) {
            return EOF;
        }
    }
    return reader->buffer[reader->pos++];
}
```

### Step 5: Performance Validation (30-45 minutes)

#### 5.1 Measure Optimized Performance
```bash
# Rebuild with optimizations
make clean && make release

# Measure new performance
echo "=== Optimized Performance Measurements ==="
echo "Small file:"
time ./bin/asthra benchmarks/small.asthra -o /tmp/small_opt.out
echo "Medium file:"
time ./bin/asthra benchmarks/medium.asthra -o /tmp/medium_opt.out
echo "Large file:"
time ./bin/asthra benchmarks/large.asthra -o /tmp/large_opt.out

# Memory usage after optimization
echo "=== Optimized Memory Usage ==="
/usr/bin/time -v ./bin/asthra benchmarks/large.asthra -o /tmp/large_opt.out 2>&1 | grep -E "(Maximum resident|User time|System time)"
```

#### 5.2 Calculate Performance Improvements
```bash
# Create performance comparison script
cat > benchmarks/compare_performance.py << 'EOF'
#!/usr/bin/env python3
import re
import sys

def parse_time(time_str):
    """Parse time string like '0m1.234s' to seconds"""
    match = re.match(r'(\d+)m([\d.]+)s', time_str)
    if match:
        minutes = int(match.group(1))
        seconds = float(match.group(2))
        return minutes * 60 + seconds
    return float(time_str.rstrip('s'))

def calculate_improvement(baseline, optimized):
    """Calculate percentage improvement"""
    if baseline == 0:
        return 0
    improvement = ((baseline - optimized) / baseline) * 100
    return improvement

# Example usage:
baseline_small = 1.234  # seconds
optimized_small = 0.987  # seconds

improvement = calculate_improvement(baseline_small, optimized_small)
print(f"Small file improvement: {improvement:.1f}%")
EOF

python3 benchmarks/compare_performance.py
```

#### 5.3 Verify Correctness
```bash
# Ensure optimizations don't break functionality
make test-all

# Compare output correctness
./bin/asthra benchmarks/small.asthra -o /tmp/small_baseline.out
./bin/asthra benchmarks/small.asthra -o /tmp/small_optimized.out

# Verify outputs are identical
diff /tmp/small_baseline.out /tmp/small_optimized.out
echo "Output verification: $?"

# Run comprehensive test suite
make test-curated
make test-integration
```

### Step 6: Advanced Profiling (Optional, 30-60 minutes)

#### 6.1 Micro-benchmarking
```c
// Create micro-benchmarks for specific functions
// benchmarks/micro/symbol_lookup_bench.c
#include <time.h>
#include <stdio.h>
#include "../../src/analysis/symbol_table.h"

void benchmark_symbol_lookup() {
    SymbolTable *table = symbol_table_create();
    
    // Populate with test data
    for (int i = 0; i < 1000; i++) {
        char name[32];
        snprintf(name, sizeof(name), "symbol_%d", i);
        symbol_table_insert(table, name, SYMBOL_VARIABLE);
    }
    
    // Benchmark lookups
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < 10000; i++) {
        char name[32];
        snprintf(name, sizeof(name), "symbol_%d", i % 1000);
        symbol_table_lookup(table, name);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                    (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Symbol lookup benchmark: %.6f seconds for 10000 lookups\n", elapsed);
    printf("Average lookup time: %.2f nanoseconds\n", (elapsed * 1e9) / 10000);
    
    symbol_table_free(table);
}
```

#### 6.2 Cache Performance Analysis
```bash
# Analyze cache performance with perf
perf stat -e cache-references,cache-misses,instructions,cycles \
    ./bin/asthra benchmarks/large.asthra -o /tmp/large.out

# Calculate cache miss rate
echo "Cache analysis complete. Check for high cache miss rates."
```

#### 6.3 Scalability Testing
```bash
# Test performance scaling with input size
for size in 100 1000 10000 50000; do
    echo "Testing with $size lines..."
    python3 -c "
with open('benchmarks/scale_test_${size}.asthra', 'w') as f:
    for i in range($size):
        f.write(f'fn func_{i}() -> i32 {{ return {i}; }}\\n')
    f.write('fn main() -> i32 { return func_0(); }\\n')
"
    time ./bin/asthra benchmarks/scale_test_${size}.asthra -o /tmp/scale_${size}.out
done
```

### Step 7: Documentation & Monitoring (15-30 minutes)

#### 7.1 Document Optimizations
```c
/**
 * @brief Optimized symbol table lookup using hash table
 * @param table The symbol table
 * @param name Symbol name to lookup
 * @return Symbol pointer or NULL if not found
 * 
 * Performance: O(1) average case, O(n) worst case
 * Memory: Additional O(n) space for hash table
 * 
 * Optimization notes:
 * - Replaced linear search O(n) with hash table O(1)
 * - 85% performance improvement on large symbol tables
 * - Uses FNV-1a hash function for good distribution
 * 
 * @see benchmark results in benchmarks/symbol_lookup_results.md
 */
Symbol* symbol_table_lookup(HashSymbolTable *table, const char *name);
```

#### 7.2 Update Performance Log
```bash
# Update performance tracking
cat >> benchmarks/performance_log.md << 'EOF'

## Optimization Results ($(date))

### Performance Improvements
- Small file: X.X% faster (X.XXs → X.XXs)
- Medium file: X.X% faster (X.XXs → X.XXs)
- Large file: X.X% faster (X.XXs → X.XXs)

### Memory Improvements
- Peak memory: X.X% reduction (XXX MB → XXX MB)
- Allocations: X.X% fewer allocations

### Optimizations Applied
1. **Symbol Table**: Linear search → Hash table (85% improvement)
2. **String Operations**: strcmp → Token types (60% improvement)
3. **Memory Allocation**: Individual malloc → Memory pools (40% improvement)
4. **File I/O**: Character-by-character → Buffered reading (25% improvement)

### Verification
- All tests pass: ✅
- Output correctness verified: ✅
- No performance regressions: ✅

EOF
```

#### 7.3 Set Up Performance Monitoring
```bash
# Create performance regression test
cat > tests/performance/regression_test.sh << 'EOF'
#!/bin/bash
# Performance regression test

BASELINE_TIME=5.0  # seconds for large file
THRESHOLD=1.1      # 10% regression threshold

echo "Running performance regression test..."
ACTUAL_TIME=$(time ./bin/asthra benchmarks/large.asthra -o /tmp/perf_test.out 2>&1 | grep real | awk '{print $2}' | sed 's/[ms]//g')

if (( $(echo "$ACTUAL_TIME > $BASELINE_TIME * $THRESHOLD" | bc -l) )); then
    echo "❌ Performance regression detected!"
    echo "Expected: < ${BASELINE_TIME}s * ${THRESHOLD} = $(echo "$BASELINE_TIME * $THRESHOLD" | bc -l)s"
    echo "Actual: ${ACTUAL_TIME}s"
    exit 1
else
    echo "✅ Performance regression test passed"
    echo "Time: ${ACTUAL_TIME}s (baseline: ${BASELINE_TIME}s)"
fi
EOF

chmod +x tests/performance/regression_test.sh
```

## Optimization Categories & Techniques

### CPU Optimizations

**Algorithm Improvements:**
- Replace O(n²) algorithms with O(n log n) or O(n)
- Use appropriate data structures (hash tables, trees)
- Implement caching/memoization for expensive operations
- Eliminate redundant computations

**Code-Level Optimizations:**
- Use compiler optimization flags (-O2, -O3)
- Minimize function call overhead in hot paths
- Use bit operations instead of arithmetic where appropriate
- Optimize loop structures and reduce branching

### Memory Optimizations

**Allocation Strategies:**
- Use memory pools for frequent small allocations
- Implement object recycling for temporary objects
- Reduce memory fragmentation with appropriate allocators
- Use stack allocation instead of heap when possible

**Data Structure Optimizations:**
- Improve cache locality with better data layout
- Use structure packing to reduce memory usage
- Implement copy-on-write for large data structures
- Use appropriate container types for access patterns

### I/O Optimizations

**File Operations:**
- Use buffered I/O instead of character-by-character
- Implement memory-mapped files for large inputs
- Reduce system call overhead with larger buffers
- Use asynchronous I/O where appropriate

## Common Performance Pitfalls

### Premature Optimization
**Problem**: Optimizing before identifying real bottlenecks
**Solution**: Always profile first, optimize second

### Micro-optimizations
**Problem**: Focusing on small gains while missing big opportunities
**Solution**: Focus on algorithmic improvements first

### Breaking Correctness
**Problem**: Optimizations that introduce bugs
**Solution**: Comprehensive testing after each optimization

### Optimization Tunnel Vision
**Problem**: Over-optimizing one component while ignoring others
**Solution**: Take a holistic view of system performance

## Success Criteria

Before considering optimization complete:

- [ ] **Measurable Improvement**: At least 20% improvement in target metric
- [ ] **Correctness Maintained**: All tests pass, output unchanged
- [ ] **No Regressions**: Other performance metrics not degraded
- [ ] **Scalability**: Improvements hold across different input sizes
- [ ] **Documentation**: Optimizations documented with rationale
- [ ] **Monitoring**: Performance regression tests in place

## Related Workflows

- **[Fixing Bugs](fixing-bugs.md)** - Performance bugs and memory issues
- **[Testing Workflows](testing-workflows.md)** - Performance testing strategies
- **[Adding Features](adding-features.md)** - Performance considerations for new features
- **[Code Review Process](code-review.md)** - Performance review guidelines

---

**🚀 Performance Optimized Successfully!**

*Your optimizations are now ready for review. Remember that the best optimizations improve both performance and code maintainability.* 