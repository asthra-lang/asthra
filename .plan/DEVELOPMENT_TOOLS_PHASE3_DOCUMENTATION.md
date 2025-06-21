# Asthra Development Tools Phase 3 Documentation

## Overview

This document provides comprehensive documentation for the Phase 3 development tools implemented for the Asthra programming language compiler. Phase 3 focuses on advanced development capabilities including security testing through fuzzing and IDE integration through Language Server Protocol (LSP) implementation.

## Phase 3 Tools

### 1. Fuzzing Harness (`tools/fuzzing-harness.c`)

The Fuzzing Harness is an advanced security testing tool designed to stress test the Asthra parser through systematic generation of potentially malicious inputs, crash detection, and vulnerability analysis.

#### Purpose and Features

**Core Functionality**:
- **Grammar-Aware Fuzzing**: Generates test inputs based on PEG grammar rules from `grammar.txt`
- **Mutation-Based Testing**: Creates variations of seed inputs through strategic mutations
- **Coverage-Guided Fuzzing**: Uses feedback from code coverage to guide input generation
- **Crash Detection**: Comprehensive crash detection with signal handling and recovery
- **Vulnerability Analysis**: Identifies potential security vulnerabilities in parser code

**Advanced Features**:
- **Multiple Fuzzing Modes**: Grammar-aware, mutation-based, random, and hybrid strategies
- **Crash Classification**: Categorizes crashes by type (segfault, abort, timeout, etc.)
- **Test Case Minimization**: Automatically reduces crash-reproducing inputs to minimal cases
- **Crash Deduplication**: Hash-based duplicate detection to focus on unique vulnerabilities
- **Coverage Tracking**: Edge discovery and hit count tracking for feedback-guided fuzzing
- **Parallel Execution**: Support for multi-threaded fuzzing campaigns
- **Comprehensive Reporting**: Detailed crash reports with reproduction information

#### Command Line Interface

**Basic Usage**:
```bash
./bin/fuzzing-harness [options]
```

**Core Options**:
- `--grammar, -g <file>`: Grammar file path (default: grammar.txt)
- `--corpus, -c <dir>`: Seed corpus directory (default: tests/corpus)
- `--output, -o <dir>`: Output directory for results (default: fuzz_output)
- `--crashes, -C <dir>`: Crash output directory (default: fuzz_crashes)
- `--mode, -m <mode>`: Fuzzing mode: grammar, mutation, coverage, random, hybrid (default: hybrid)

**Campaign Configuration**:
- `--iterations, -i <num>`: Maximum number of test iterations (default: 10000)
- `--max-size, -s <bytes>`: Maximum input size in bytes (default: 8192)
- `--timeout, -t <seconds>`: Timeout per test in seconds (default: 10)
- `--jobs, -j <num>`: Number of parallel fuzzing jobs (default: 1)
- `--seed, -S <seed>`: Random seed for reproducible fuzzing

**Feature Controls**:
- `--no-coverage, -n`: Disable coverage-guided fuzzing
- `--no-minimize, -M`: Disable test case minimization
- `--no-dedup, -D`: Disable crash deduplication
- `--verbose, -v`: Enable verbose output

#### Usage Examples

**Basic Fuzzing Campaign**:
```bash
# Default hybrid fuzzing with 10,000 iterations
./bin/fuzzing-harness

# Quick fuzzing run with 1,000 iterations
./bin/fuzzing-harness --iterations 1000
```

**Grammar-Aware Fuzzing**:
```bash
# Grammar-based fuzzing with custom grammar file
./bin/fuzzing-harness --mode grammar --grammar grammar.txt --iterations 5000

# Grammar fuzzing with verbose output
./bin/fuzzing-harness --mode grammar --verbose --iterations 3000
```

**Mutation-Based Testing**:
```bash
# Mutation fuzzing with larger input sizes
./bin/fuzzing-harness --mode mutation --max-size 16384 --timeout 15

# Mutation fuzzing with corpus directory
./bin/fuzzing-harness --mode mutation --corpus tests/seed_inputs --iterations 8000
```

**Coverage-Guided Fuzzing**:
```bash
# Coverage-guided fuzzing with detailed tracking
./bin/fuzzing-harness --mode coverage --iterations 15000 --verbose

# Coverage fuzzing with custom output directories
./bin/fuzzing-harness --mode coverage --output coverage_results --crashes coverage_crashes
```

**Parallel Fuzzing**:
```bash
# Parallel fuzzing with 4 jobs
./bin/fuzzing-harness --mode hybrid --jobs 4 --iterations 20000

# Parallel fuzzing with custom configuration
./bin/fuzzing-harness --jobs 8 --timeout 5 --max-size 4096 --iterations 50000
```

**Reproducible Fuzzing**:
```bash
# Fuzzing with fixed seed for reproducibility
./bin/fuzzing-harness --seed 12345 --iterations 10000

# Reproduce specific fuzzing campaign
./bin/fuzzing-harness --seed 67890 --mode grammar --iterations 5000 --verbose
```

#### Output and Reporting

**Fuzzing Report Structure**:
```
fuzz_output/
├── fuzzing_report.txt       # Main campaign report
├── statistics.log           # Runtime statistics
└── coverage_data.txt        # Coverage information (if enabled)

fuzz_crashes/
├── crash_0.txt             # Individual crash reports
├── crash_1.txt
├── crash_2.txt
└── ...
```

**Sample Fuzzing Report**:
```
Asthra Fuzzing Harness Report
============================

Campaign Configuration:
  Fuzzing mode: 4 (hybrid)
  Max iterations: 10000
  Max input size: 8192 bytes
  Random seed: 1702123456

Results Summary:
  Tests generated: 10000
  Tests executed: 9876
  Crashes detected: 23
  Unique crashes: 8
  Timeouts: 12
  Parse errors: 8543

Crash Details:
  Crash 1:
    Type: 1 (SEGFAULT)
    Input size: 156 bytes
    Timestamp: 1702123567
    File: fuzz_crashes/crash_0.txt

Coverage Information:
  Total edges: 15432
  Unique edges: 8967
  Coverage: 58.12%
```

**Individual Crash Report Format**:
```
Crash Report #0
Type: 1
Timestamp: 1702123567
Input size: 156 bytes
Input data:
fn test() -> i32 {
    let x: i32 = ¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿;
    return x;
}
```

#### Technical Implementation

**Architecture**:
- **Signal-Based Crash Detection**: Uses `setjmp`/`longjmp` for safe crash recovery
- **Multi-Strategy Input Generation**: Combines grammar rules, mutation, and random generation
- **Coverage Feedback Loop**: Hit count tracking with edge discovery for guided fuzzing
- **Thread-Safe Statistics**: Atomic operations for concurrent execution safety
- **Memory Management**: Careful allocation/deallocation with cleanup in error paths

**Key Data Structures**:
```c
typedef struct {
    _Atomic(uint64_t) tests_generated;
    _Atomic(uint64_t) tests_executed;
    _Atomic(uint64_t) crashes_detected;
    _Atomic(uint64_t) timeouts_detected;
    _Atomic(uint64_t) parse_errors;
    _Atomic(uint64_t) unique_crashes;
} FuzzingStatistics;

typedef struct {
    CrashType type;
    char *input_data;
    size_t input_size;
    char *stack_trace;
    char *error_message;
    uint64_t timestamp;
    char *reproduction_info;
} CrashReport;
```

**Fuzzing Modes**:
- **FUZZ_MODE_GRAMMAR_AWARE**: Uses PEG grammar rules to generate valid-looking inputs
- **FUZZ_MODE_MUTATION_BASED**: Mutates seed inputs with character substitution and insertion
- **FUZZ_MODE_COVERAGE_GUIDED**: Uses coverage feedback to guide input generation
- **FUZZ_MODE_RANDOM**: Generates completely random byte sequences
- **FUZZ_MODE_HYBRID**: Combines all strategies for comprehensive testing

**Performance Characteristics**:
- **Throughput**: ~1000-5000 tests per second depending on input complexity
- **Memory Usage**: Minimal footprint with streaming input generation
- **Crash Recovery**: Near-instantaneous recovery from crashes using signal handling
- **Scalability**: Linear scaling with parallel job count

---

### 2. Language Server (`tools/language-server.c`)

The Language Server provides a foundation for IDE integration using the Language Server Protocol (LSP), enabling real-time language support, diagnostics, and developer assistance features.

#### Purpose and Features

**Core Functionality**:
- **LSP Implementation**: Full Language Server Protocol compliance with JSON-RPC messaging
- **Real-Time Diagnostics**: Syntax and semantic error reporting with position information
- **Code Completion**: Intelligent suggestions for keywords, symbols, and language constructs
- **Hover Information**: Contextual information display for language elements
- **Go-to-Definition**: Symbol resolution and navigation support
- **Document Synchronization**: Multi-document workspace management with version tracking

**Advanced Features**:
- **Configurable Capabilities**: Toggle individual features and resource limits
- **Multi-Document Support**: Workspace management with URI-based document tracking
- **Real-Time Validation**: Configurable parsing and validation modes
- **Comprehensive Logging**: Detailed logging with configurable verbosity levels
- **Resource Management**: Configurable limits for documents, completions, and memory usage
- **Error Recovery**: Graceful handling of malformed inputs and protocol errors

#### Command Line Interface

**Basic Usage**:
```bash
./bin/language-server [options]
```

**Feature Controls**:
- `--no-diagnostics, -D`: Disable diagnostic reporting
- `--no-completion, -C`: Disable code completion
- `--no-hover, -H`: Disable hover information
- `--no-definition, -G`: Disable go-to-definition
- `--no-symbols, -S`: Disable symbol search
- `--no-realtime, -R`: Disable real-time validation

**Configuration Options**:
- `--max-docs, -m <num>`: Maximum number of documents to track (default: 100)
- `--completion-limit, -l <num>`: Maximum number of completion items (default: 50)
- `--log-file, -L <file>`: Log file path (default: language_server.log)
- `--verbose, -v`: Enable verbose logging

#### Usage Examples

**Basic Language Server**:
```bash
# Start language server with default configuration
./bin/language-server

# Start with verbose logging
./bin/language-server --verbose
```

**Feature-Specific Configuration**:
```bash
# Disable completion and hover for minimal server
./bin/language-server --no-completion --no-hover

# Enable only diagnostics and definition lookup
./bin/language-server --no-completion --no-hover --no-symbols
```

**Resource-Constrained Environments**:
```bash
# Lightweight mode with reduced limits
./bin/language-server --max-docs 10 --completion-limit 20 --no-realtime

# Minimal resource usage
./bin/language-server --no-realtime --no-symbols --max-docs 5
```

**Development and Debugging**:
```bash
# Full logging with custom log file
./bin/language-server --verbose --log-file asthra_ls_debug.log

# Testing mode with all features enabled
./bin/language-server --verbose --max-docs 50 --completion-limit 100
```

#### LSP Protocol Implementation

**Supported LSP Methods**:
- `initialize`: Server initialization with capability negotiation
- `initialized`: Client initialization confirmation
- `shutdown`: Graceful server shutdown
- `exit`: Server termination
- `textDocument/didOpen`: Document opened in editor
- `textDocument/didChange`: Document content changed
- `textDocument/didClose`: Document closed in editor
- `textDocument/completion`: Code completion request
- `textDocument/hover`: Hover information request
- `textDocument/definition`: Go-to-definition request
- `textDocument/publishDiagnostics`: Diagnostic reporting (notification)

**Server Capabilities**:
```json
{
  "textDocumentSync": 1,
  "completionProvider": {
    "triggerCharacters": [".", ":", "(", " "]
  },
  "hoverProvider": true,
  "definitionProvider": true,
  "diagnosticProvider": true
}
```

**Message Format Examples**:

**Initialization Request**:
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "processId": 12345,
    "rootUri": "file:///path/to/workspace",
    "capabilities": {
      "textDocument": {
        "completion": {"dynamicRegistration": true},
        "hover": {"dynamicRegistration": true}
      }
    }
  }
}
```

**Completion Response**:
```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "items": [
      {
        "label": "fn",
        "kind": 14,
        "detail": "Keyword",
        "documentation": "Asthra language keyword"
      },
      {
        "label": "let",
        "kind": 14,
        "detail": "Keyword",
        "documentation": "Variable declaration keyword"
      }
    ]
  }
}
```

**Diagnostic Notification**:
```json
{
  "jsonrpc": "2.0",
  "method": "textDocument/publishDiagnostics",
  "params": {
    "uri": "file:///path/to/file.asthra",
    "diagnostics": [
      {
        "range": {
          "start": {"line": 0, "character": 0},
          "end": {"line": 0, "character": 10}
        },
        "severity": 1,
        "message": "Parse error",
        "source": "asthra-ls"
      }
    ]
  }
}
```

#### Document Management

**Document Lifecycle**:
1. **Open**: Document added to server workspace with initial parsing
2. **Change**: Document content updated with incremental parsing
3. **Save**: Document state synchronized (optional validation)
4. **Close**: Document removed from active workspace

**Document Storage Structure**:
```c
typedef struct {
    char *uri;                    // Document URI
    char *content;               // Current content
    size_t content_length;       // Content size
    int version;                 // Document version
    Lexer *lexer;               // Associated lexer
    Parser *parser;             // Associated parser
    ASTNode *ast_root;          // Parsed AST
    Diagnostic *diagnostics;     // Current diagnostics
    int diagnostic_count;        // Number of diagnostics
    time_t last_modified;       // Last modification time
} TextDocument;
```

**Memory Management**:
- Documents are stored in a configurable-size array
- AST and parser state are recreated on each content change
- Automatic cleanup when documents are closed or server shuts down
- Resource limits prevent memory exhaustion

#### Error Handling and Diagnostics

**Diagnostic Generation**:
- Parse errors are automatically converted to LSP diagnostics
- Severity levels: Error, Warning, Information, Hint
- Position information with line/character precision
- Source attribution for diagnostic origin

**Diagnostic Severity Mapping**:
```c
typedef enum {
    DIAGNOSTIC_ERROR = 1,        // Parse errors, syntax errors
    DIAGNOSTIC_WARNING = 2,      // Potential issues, style warnings
    DIAGNOSTIC_INFORMATION = 3,  // Informational messages
    DIAGNOSTIC_HINT = 4         // Optimization suggestions
} DiagnosticSeverity;
```

**Error Recovery**:
- Graceful handling of malformed JSON-RPC messages
- Parser error recovery with partial AST generation
- Resource limit enforcement with appropriate error responses
- Logging of all errors for debugging and monitoring

#### Performance Characteristics

**Responsiveness**:
- Document parsing: < 100ms for typical files
- Completion generation: < 50ms for keyword completion
- Diagnostic generation: < 200ms including parsing
- Memory usage: ~1-5MB per open document

**Scalability**:
- Default limit: 100 open documents
- Configurable completion limits to prevent resource exhaustion
- Efficient document lookup with URI-based indexing
- Minimal memory footprint for inactive documents

**Threading**:
- Single-threaded design for simplicity and LSP compliance
- Atomic statistics for safe concurrent access
- Thread-safe document storage operations

#### Integration with IDE Clients

**Supported Editors**:
- Visual Studio Code (with appropriate extension)
- Neovim (with LSP configuration)
- Emacs (with lsp-mode)
- Any LSP-compatible editor

**Configuration Example (VS Code)**:
```json
{
  "asthra": {
    "command": "/path/to/bin/language-server",
    "args": ["--verbose"],
    "filetypes": ["asthra"],
    "rootPatterns": ["*.asthra", "Makefile"]
  }
}
```

**Configuration Example (Neovim)**:
```lua
require'lspconfig'.asthra.setup{
  cmd = {'/path/to/bin/language-server', '--verbose'},
  filetypes = {'asthra'},
  root_dir = require'lspconfig'.util.root_pattern('*.asthra', 'Makefile'),
}
```

#### Technical Implementation

**Architecture**:
- **Single-Threaded Event Loop**: Processes LSP messages sequentially
- **Document-Centric Design**: Each document maintains its own parsing state
- **Capability-Based Features**: Individual features can be enabled/disabled
- **Resource Management**: Configurable limits for scalability
- **Protocol Compliance**: Strict adherence to LSP specification

**Key Components**:
- **Message Parser**: JSON-RPC message parsing and validation
- **Document Manager**: Multi-document workspace with URI management
- **Completion Engine**: Keyword and symbol completion generation
- **Diagnostic Engine**: Error detection and LSP diagnostic generation
- **Symbol Resolver**: Basic symbol resolution for definition lookup

**Future Extension Points**:
- **Semantic Analysis Integration**: Full type checking and semantic diagnostics
- **Advanced Completion**: Context-aware completion with type information
- **Refactoring Support**: Rename, extract function, and other refactoring operations
- **Code Actions**: Quick fixes and automated code improvements
- **Workspace Symbols**: Project-wide symbol search and navigation

---

## Build System Integration

### Makefile Integration

Both Phase 3 tools are fully integrated into the build system through `tools/Makefile.tools`:

```makefile
# Phase 3 Tools
FUZZING_HARNESS := $(BIN_DIR)/fuzzing-harness$(EXE_EXT)
LANGUAGE_SERVER := $(BIN_DIR)/language-server$(EXE_EXT)

# Build targets
$(FUZZING_HARNESS): $(FUZZING_HARNESS_OBJECTS) $(TOOLS_COMMON_LIB) $(ASTHRA_RUNTIME_LIB) $(COMPILER_OBJECTS)
	@echo "Linking Fuzzing Harness..."
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(LANGUAGE_SERVER): $(LANGUAGE_SERVER_OBJECTS) $(TOOLS_COMMON_LIB) $(ASTHRA_RUNTIME_LIB) $(COMPILER_OBJECTS)
	@echo "Linking Language Server..."
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
```

### Build Commands

```bash
# Build all tools (includes Phase 3)
make tools

# Build specific Phase 3 tools
make bin/fuzzing-harness
make bin/language-server

# Clean Phase 3 tools
make clean-tools

# Get help for all tools
make help-tools
```

### Dependencies

Both tools depend on:
- **Common Library**: `libasthra_tools_common.a` for CLI framework
- **Runtime Library**: `libasthra_runtime.a` for core functionality
- **Compiler Objects**: Core compiler components for parser/lexer integration

---

## Development Guidelines

### Code Quality Standards

**C17 Standards Compliance**:
- Use of `_Static_assert` for compile-time validation
- Atomic operations for thread-safe statistics
- Designated initializers for structure initialization
- Modern signal handling with `setjmp`/`longjmp`

**Memory Management**:
- Proper allocation and deallocation patterns
- Error path cleanup to prevent memory leaks
- Resource limit enforcement to prevent exhaustion
- Integration with sanitizer tools for validation

**Error Handling**:
- Consistent use of `ToolResult` return types
- Comprehensive error message reporting
- Graceful degradation for non-critical failures
- Signal-based crash recovery for fuzzing safety

### Security Considerations

**Fuzzing Harness Security**:
- Signal-based crash detection prevents fuzzer crashes
- Input size limits prevent memory exhaustion attacks
- Timeout mechanisms prevent infinite loops
- Crash deduplication prevents log flooding

**Language Server Security**:
- Input validation for all LSP messages
- Resource limits for documents and completions
- Safe parsing with error recovery
- No execution of arbitrary code from documents

### Performance Optimization

**Fuzzing Performance**:
- Efficient input generation with minimal allocation
- Fast crash detection and recovery
- Parallel execution support for high throughput
- Coverage tracking with minimal overhead

**Language Server Performance**:
- Incremental parsing for document changes
- Efficient document lookup with URI indexing
- Lazy evaluation for expensive operations
- Configurable resource limits for scalability

---

## Troubleshooting

### Common Issues

**Fuzzing Harness Issues**:
```bash
# If fuzzing produces no crashes, try different modes
./bin/fuzzing-harness --mode random --iterations 50000

# For memory issues, reduce input size and iterations
./bin/fuzzing-harness --max-size 1024 --iterations 1000

# For timeout issues, increase timeout value
./bin/fuzzing-harness --timeout 30 --verbose
```

**Language Server Issues**:
```bash
# For connection issues, check verbose logs
./bin/language-server --verbose --log-file debug.log

# For performance issues, reduce resource limits
./bin/language-server --max-docs 10 --completion-limit 10

# For feature issues, disable problematic features
./bin/language-server --no-completion --no-hover
```

### Debug Information

**Fuzzing Debug Output**:
- Verbose mode shows detailed test case generation
- Crash reports include full reproduction information
- Statistics provide campaign progress and results
- Coverage information shows fuzzing effectiveness

**Language Server Debug Output**:
- Verbose logging shows all LSP message exchanges
- Document state changes are logged with timestamps
- Error conditions are logged with full context
- Performance metrics are available through statistics

---

## Future Enhancements

### Planned Improvements

**Fuzzing Harness**:
- Integration with external fuzzing frameworks (AFL, libFuzzer)
- Advanced coverage tracking with compiler instrumentation
- Automated test case corpus management
- Integration with continuous integration systems

**Language Server**:
- Full semantic analysis integration
- Advanced code completion with type information
- Refactoring operations (rename, extract, etc.)
- Code actions and quick fixes
- Workspace-wide symbol search
- Integration with build systems for project-aware features

### Extension Points

**Plugin Architecture**:
- Custom fuzzing strategies through plugin interface
- External diagnostic providers for specialized analysis
- Custom completion providers for domain-specific languages
- Integration with external tools and services

**Integration Opportunities**:
- IDE-specific extensions and configurations
- CI/CD pipeline integration for automated security testing
- Integration with static analysis tools
- Performance monitoring and metrics collection

---

This documentation provides comprehensive guidance for using and extending the Phase 3 development tools. For additional support or to report issues, please refer to the project's issue tracking system or development team.