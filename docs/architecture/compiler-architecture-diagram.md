# Asthra Compiler Architecture Diagrams

This document provides visual representations of the Asthra compiler architecture using Mermaid diagrams.

## Overall Compilation Pipeline

```mermaid
graph TB
    subgraph "Frontend"
        A[Source Files<br/>.asthra] --> B[Lexer<br/>Tokenization]
        B --> C[Parser<br/>AST Generation]
        C --> D[Semantic Analyzer<br/>Type Checking]
    end
    
    subgraph "Middle-end"
        D --> E[Const Evaluator<br/>Compile-time Eval]
        E --> F[Type Inference<br/>& Resolution]
        F --> G[Memory Analysis<br/>Ownership Tracking]
    end
    
    subgraph "Backend"
        G --> H[Code Generator<br/>Assembly Gen]
        H --> I[Optimizer<br/>Peephole & CFG]
        I --> J[ELF Writer<br/>Binary Generation]
    end
    
    subgraph "Runtime Integration"
        J --> K[Linker<br/>Symbol Resolution]
        K --> L[Executable<br/>Native Binary]
        M[Runtime Library<br/>libasthra] --> L
    end
    
    style A fill:#e1f5fe
    style L fill:#c8e6c9
    style M fill:#fff9c4
```

## Detailed Component Architecture

```mermaid
graph LR
    subgraph "Parser Module"
        P1[Lexer Core] --> P2[Token Stream]
        P2 --> P3[Grammar Rules]
        P3 --> P4[AST Builder]
        P4 --> P5[AST Nodes]
        
        P6[Keyword Table] --> P1
        P7[String Scanner] --> P1
        P8[Number Scanner] --> P1
    end
    
    subgraph "Semantic Module"
        S1[Symbol Tables] --> S2[Scope Manager]
        S3[Type Registry] --> S4[Type Checker]
        S5[Const Evaluator] --> S4
        S2 --> S4
        S4 --> S6[Analyzed AST]
        
        S7[FFI Validator] --> S4
        S8[Memory Analyzer] --> S4
    end
    
    subgraph "CodeGen Module"
        C1[Instruction Buffer] --> C2[Register Allocator]
        C2 --> C3[Assembly Emitter]
        C3 --> C4[ELF Sections]
        
        C5[Optimization Passes] --> C3
        C6[Generic Instantiator] --> C1
        C7[FFI Marshaler] --> C3
    end
    
    P5 --> S1
    S6 --> C1
```

## Memory Management Architecture

```mermaid
graph TD
    subgraph "AST Memory Management"
        AM1[AST Node] --> AM2{Ref Count > 0?}
        AM2 -->|Yes| AM3[Keep Alive]
        AM2 -->|No| AM4[Free Node]
        AM4 --> AM5[Free Children]
        
        AM6[Atomic RefCount] --> AM1
        AM7[Thread-Safe Ops] --> AM6
    end
    
    subgraph "Runtime Memory Zones"
        RM1[GC Zone] --> RM2[Conservative GC]
        RM3[C Zone] --> RM4[Manual Memory]
        RM5[Pinned Zone] --> RM6[FFI Safety]
        
        RM2 --> RM7[Mark & Sweep]
        RM4 --> RM8[Ref Counting]
        RM6 --> RM9[Ownership Track]
    end
    
    subgraph "Allocator Hierarchy"
        A1[Arena Allocator] --> A2[Parser Temps]
        A3[Pool Allocator] --> A4[Symbol Entries]
        A5[Slab Allocator] --> A6[Type Infos]
        
        A1 --> A7[Bulk Free]
        A3 --> A8[Fixed Size]
        A5 --> A9[Cache Friendly]
    end
```

## Type System Architecture

```mermaid
classDiagram
    class TypeInfo {
        +TypeKind kind
        +string name
        +size_t size
        +size_t alignment
        +atomic refcount
    }
    
    class PrimitiveType {
        +PrimitiveKind type
    }
    
    class SliceType {
        +TypeInfo* element_type
    }
    
    class StructType {
        +Field* fields
        +size_t field_count
        +bool is_generic
    }
    
    class EnumType {
        +Variant* variants
        +size_t variant_count
    }
    
    class ResultType {
        +TypeInfo* ok_type
        +TypeInfo* err_type
    }
    
    class GenericType {
        +TypeParam* params
        +TypeInfo* definition
    }
    
    TypeInfo <|-- PrimitiveType
    TypeInfo <|-- SliceType
    TypeInfo <|-- StructType
    TypeInfo <|-- EnumType
    TypeInfo <|-- ResultType
    TypeInfo <|-- GenericType
```

## Code Generation Flow

```mermaid
flowchart TD
    A[Analyzed AST] --> B{Node Type}
    
    B -->|Function| C[Generate Prologue]
    C --> D[Allocate Locals]
    D --> E[Generate Body]
    E --> F[Generate Epilogue]
    
    B -->|Expression| G[Evaluate Operands]
    G --> H[Allocate Registers]
    H --> I[Emit Instructions]
    
    B -->|Statement| J[Generate Control Flow]
    J --> K[Update CFG]
    K --> L[Emit Jumps/Branches]
    
    F --> M[Optimization]
    I --> M
    L --> M
    
    M --> N[Register Reallocation]
    N --> O[Peephole Optimization]
    O --> P[Dead Code Elimination]
    P --> Q[Final Assembly]
```

## Build System Architecture

```mermaid
graph TD
    subgraph "Makefile Modules"
        M1[platform.mk] --> M2[Detect OS/Arch]
        M3[compiler.mk] --> M4[Compiler Flags]
        M5[paths.mk] --> M6[Directory Setup]
        M7[rules.mk] --> M8[Build Rules]
        M9[targets.mk] --> M10[Main Targets]
    end
    
    subgraph "Test System"
        T1[test-framework] --> T2[Category Tests]
        T2 --> T3[Parser Tests]
        T2 --> T4[Semantic Tests]
        T2 --> T5[CodeGen Tests]
        T2 --> T6[FFI Tests]
        T2 --> T7[Integration Tests]
    end
    
    subgraph "Ampu Build Tool"
        A1[Project Config] --> A2[Dependency Resolution]
        A2 --> A3[Import Validation]
        A3 --> A4[Compilation Order]
        A4 --> A5[Invoke Compiler]
        A5 --> A6[Link Binary]
    end
```

## Concurrency Architecture

```mermaid
graph LR
    subgraph "Compilation Parallelism"
        CP1[File Parser] --> CP2[Work Queue]
        CP2 --> CP3[Thread Pool]
        CP3 --> CP4[Parallel Analysis]
        CP4 --> CP5[Merge Results]
    end
    
    subgraph "Runtime Concurrency"
        RC1[Task Spawner] --> RC2[Task Queue]
        RC2 --> RC3[Worker Threads]
        RC3 --> RC4[Task Execution]
        
        RC5[Channels] --> RC6[Message Passing]
        RC7[Atomics] --> RC8[Lock-Free Ops]
        RC9[Mutexes] --> RC10[Critical Sections]
    end
    
    subgraph "Thread Safety"
        TS1[Atomic RefCount] --> TS2[AST Nodes]
        TS3[Thread-Local] --> TS4[GC Roots]
        TS5[Lock-Free] --> TS6[Work Stealing]
    end
```

## Error Handling Flow

```mermaid
stateDiagram-v2
    [*] --> Lexing
    Lexing --> Parsing : Tokens
    Lexing --> LexError : Invalid Token
    
    Parsing --> SemanticAnalysis : AST
    Parsing --> ParseError : Syntax Error
    
    SemanticAnalysis --> CodeGeneration : Validated AST
    SemanticAnalysis --> SemanticError : Type Error
    
    CodeGeneration --> Optimization : Assembly
    CodeGeneration --> CodeGenError : Generation Failed
    
    Optimization --> ELFGeneration : Optimized Code
    
    ELFGeneration --> Success : Binary
    ELFGeneration --> LinkError : Symbol Error
    
    LexError --> ErrorRecovery
    ParseError --> ErrorRecovery
    SemanticError --> ErrorReport
    CodeGenError --> ErrorReport
    LinkError --> ErrorReport
    
    ErrorRecovery --> Parsing : Synchronized
    ErrorReport --> [*]
    Success --> [*]
```

## Extension Points

```mermaid
mindmap
  root((Asthra Compiler))
    Language Features
      Grammar Extensions
        New Keywords
        New Syntax
        New Operators
      Type System
        New Types
        Type Constraints
        Generic Extensions
      Semantic Rules
        New Validations
        Custom Analyses
        
    Optimization
      New Passes
        Peephole Patterns
        CFG Optimizations
        Data Flow Analysis
      Target-Specific
        Architecture Opts
        ABI Optimizations
        
    Platform Support
      New Architectures
        ISA Support
        Register Sets
        Calling Conventions
      New OS
        Binary Formats
        System Calls
        
    Tools Integration
      IDE Support
        Language Server
        Debugger Interface
      Build Systems
        Package Managers
        CI/CD Integration
```

## Data Flow Through Compilation

```mermaid
sequenceDiagram
    participant Source
    participant Lexer
    participant Parser
    participant Semantic
    participant CodeGen
    participant Optimizer
    participant ELF
    participant Binary
    
    Source->>Lexer: Character Stream
    Lexer->>Parser: Token Stream
    Parser->>Semantic: AST
    
    Semantic->>Semantic: Symbol Resolution
    Semantic->>Semantic: Type Checking
    Semantic->>Semantic: Memory Analysis
    
    Semantic->>CodeGen: Annotated AST
    CodeGen->>CodeGen: Instruction Selection
    CodeGen->>CodeGen: Register Allocation
    
    CodeGen->>Optimizer: Raw Assembly
    Optimizer->>Optimizer: Peephole Opts
    Optimizer->>Optimizer: Dead Code Elim
    
    Optimizer->>ELF: Optimized Assembly
    ELF->>ELF: Section Generation
    ELF->>ELF: Symbol Table
    ELF->>ELF: Relocations
    
    ELF->>Binary: Executable File
```

## Summary

These diagrams illustrate:
- **Pipeline Flow**: How source code transforms into executables
- **Component Architecture**: Internal structure of major modules
- **Memory Management**: Reference counting and allocation strategies
- **Type System**: Hierarchical type representation
- **Build System**: Modular Makefile organization
- **Concurrency**: Parallel compilation and runtime support
- **Error Handling**: Recovery and reporting mechanisms
- **Extension Points**: Where to add new features
- **Data Flow**: Step-by-step compilation process

The visual representations help understand the relationships between components and the flow of data through the compilation pipeline.