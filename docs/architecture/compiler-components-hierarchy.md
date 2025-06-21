# Asthra Compiler Components by Abstraction Level

This document lists the components of the Asthra compiler ordered by their level of abstraction, from the lowest level (lexer) to the highest level (integration). This hierarchy represents the typical flow of compilation from source text through to executable generation.

## Core Compilation Pipeline Components

### 1. **Lexer** (Lowest Level)
- **Directory**: `tests/lexer/`
- **Purpose**: Tokenizes source code into fundamental units
- **Responsibilities**:
  - Breaking raw text into tokens
  - Keyword recognition
  - Operator identification
  - Literal parsing (strings, numbers)
  - Comment handling
  - Error reporting for invalid tokens

### 2. **Parser**
- **Directory**: `tests/parser/`
- **Purpose**: Constructs Abstract Syntax Trees (AST) from tokens
- **Responsibilities**:
  - Grammar compliance validation
  - AST structure construction
  - Syntax validation
  - Operator precedence handling
  - Annotation parsing
  - Error recovery and reporting

### 3. **Semantic Analysis**
- **Directory**: `tests/semantic/`
- **Purpose**: Type checking and semantic validation
- **Responsibilities**:
  - Type inference and checking
  - Symbol resolution
  - Scope analysis
  - Semantic constraint validation
  - Name binding
  - Control flow analysis

### 4. **Code Generation**
- **Directory**: `tests/codegen/`
- **Purpose**: Generates assembly/machine code from AST
- **Responsibilities**:
  - Instruction generation
  - Register allocation
  - Stack frame management
  - Expression evaluation
  - Control flow implementation
  - ELF/object file writing

### 5. **Optimization**
- **Directory**: `tests/optimization/`
- **Purpose**: Code optimization passes
- **Responsibilities**:
  - Constant folding
  - Dead code elimination
  - Common subexpression elimination
  - Peephole optimization
  - Instruction scheduling
  - Performance improvements

### 6. **Linker**
- **Directory**: `tests/linker/`
- **Purpose**: Links object files into executables
- **Responsibilities**:
  - Symbol resolution
  - Object file handling
  - Library linking
  - Relocation processing
  - Executable generation

### 7. **Platform**
- **Directory**: `tests/platform/`
- **Purpose**: Platform-specific executable generation
- **Responsibilities**:
  - ELF format support (Linux)
  - Mach-O format support (macOS)
  - PE format support (Windows)
  - Cross-platform compatibility
  - Architecture-specific code

### 8. **Runtime System**
- **Directory**: `tests/runtime/`
- **Purpose**: Runtime library functionality
- **Responsibilities**:
  - Memory management
  - Task execution
  - Runtime APIs
  - String and slice operations
  - Garbage collection
  - Runtime initialization

### 9. **Standard Library**
- **Directory**: `tests/stdlib/`
- **Purpose**: Standard library packages and utilities
- **Responsibilities**:
  - Built-in packages
  - Channel implementation
  - Coordination primitives
  - Hash/JSON/TOML support
  - Common utilities

### 10. **Integration** (Highest Level)
- **Directory**: `tests/integration/`
- **Purpose**: End-to-end compilation pipeline testing
- **Responsibilities**:
  - Complete program compilation
  - Full pipeline validation
  - Cross-component integration
  - Real-world program testing
  - Performance validation

## Cross-Cutting Components

These components provide functionality across multiple abstraction levels:

### **FFI (Foreign Function Interface)**
- **Directory**: `tests/ffi/`
- **Purpose**: C interoperability
- **Integrates with**: Parser, Semantic, Codegen, Runtime

### **Concurrency**
- **Directory**: `tests/concurrency/`
- **Purpose**: Concurrent programming features
- **Integrates with**: Semantic, Codegen, Runtime

### **Memory Management**
- **Directory**: `tests/memory/`
- **Purpose**: Memory safety and management
- **Integrates with**: Codegen, Runtime

### **Security**
- **Directory**: `tests/security/`
- **Purpose**: Security and safety features
- **Integrates with**: All compilation stages

### **Performance**
- **Directory**: `tests/performance/`
- **Purpose**: Performance benchmarks and validation
- **Integrates with**: All components

### **Patterns**
- **Directory**: `tests/patterns/`
- **Purpose**: Pattern matching support
- **Integrates with**: Parser, Semantic, Codegen

### **Pipeline**
- **Directory**: `tests/pipeline/`
- **Purpose**: Compilation pipeline orchestration
- **Integrates with**: All compilation stages

## Supporting Infrastructure

### **Test Framework**
- **Directory**: `tests/framework/`
- **Purpose**: Testing infrastructure and utilities
- **Supports**: All test categories

### **AI Integration**
- **Directories**: `tests/ai_annotations/`, `tests/ai_api/`, `tests/ai_linter/`
- **Purpose**: AI-assisted development features
- **Supports**: Development workflow

### **Diagnostics**
- **Directory**: `tests/diagnostics/`
- **Purpose**: Error reporting and diagnostics
- **Supports**: All compilation stages

## Notes

- Each component builds upon the abstractions provided by lower-level components
- The compilation flow typically proceeds from Lexer → Parser → Semantic → Codegen → Optimization → Linker → Platform
- The Runtime and Standard Library provide the execution environment for compiled programs
- Cross-cutting components interact with multiple stages of the compilation pipeline
- Integration tests validate the complete compilation workflow from source to executable