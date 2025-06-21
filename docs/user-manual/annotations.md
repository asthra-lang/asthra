# Annotations and Semantic Tags

**Last Updated**: 2024-12-28  
**Implementation Status**: ✅ Human Review Tags Implemented, 🔧 Context Parameters Partially Implemented  
**Target Audience**: AI code generators and human developers

## Overview

Asthra's annotation system provides semantic metadata that guides AI code generation, enables human review workflows, and provides compiler hints for optimization and safety. This system is a core feature of Asthra's AI-first design philosophy, allowing precise communication between AI generators and human reviewers.

### Purpose in AI-First Language Design

Annotations serve multiple critical functions in Asthra:

- **AI Guidance**: Provide semantic hints to AI code generators about complexity, safety requirements, and review needs
- **Human Review Coordination**: Enable systematic human oversight of AI-generated code through priority-based review workflows
- **Compiler Optimization**: Guide compiler optimizations for security, performance, and memory management
- **Tooling Integration**: Enable build systems, static analyzers, and IDEs to process code with enhanced context

### Integration with Compiler and Tooling

The Asthra compiler natively understands and processes annotations:

- **Parse-time Processing**: Annotations are parsed as first-class language constructs
- **Semantic Analysis**: Annotation semantics are validated during compilation
- **Code Generation**: Annotations influence compiler output for security and performance
- **Metadata Export**: Annotation information is available to external tooling via compiler APIs

### Role in AI-Human Collaboration

Annotations enable a structured workflow where AI generates code with explicit guidance for human reviewers:

```asthra
#[human_review(high, context = "crypto")]
#[constant_time]
pub fn generate_cryptographic_key(entropy: []u8) -> Result<[]u8, string> {// AI-generated cryptographic code with clear human review requirements
    // High priority due to security implications
    // Constant-time execution prevents timing attacks
    return derive_key_from_entropy(entropy);
 + "
```

## Human Review Tags (`#[human_review]`)

The human review annotation system is the cornerstone of Asthra's AI-human collaboration model, providing a structured approach to code review prioritization.

### Philosophy and Design

Human review tags embody Asthra's philosophy that AI-generated code should be transparent about its complexity and review requirements. Rather than requiring human review of all code, the system enables targeted review based on:

- **Complexity Assessment**: AI generators self-assess the complexity of generated code
- **Risk Evaluation**: Higher-risk operations receive higher review priorities
- **Context Awareness**: Domain-specific contexts (crypto, concurrency, FFI) receive specialized review
- **Efficiency Optimization**: Low-risk, well-established patterns require minimal human oversight

### Priority-Based Review System

The review system uses four priority levels to balance thoroughness with efficiency:

#### `low` Priority - Standard Operations, Well-Established Patterns

Use for routine operations with well-understood behavior and minimal risk:

```asthra
#[human_review(low)]
pub fn calculate_checksum(data: []u8) -> u32 " + 
    // Standard checksum calculation using well-established algorithm
    let checksum: u32 = 0;
    for byte in data {
        checksum = checksum ^ (byte as u32);
     + "
    return checksum;
}

#[human_review(low)]
pub fn format_user_message(username: string, message: string) -> string " + 
    // Simple string formatting with no security implications
    return username + ": " + message;
 + "
```

#### `medium` Priority - Complex Logic, Benefits from Human Review

Use for operations involving complex logic, multiple error paths, or non-trivial algorithms:

```asthra
#[human_review(medium)]
pub fn parse_configuration_file(content: string) -> Result<Config, string> {// Complex parsing logic with multiple validation steps
    let lines: []string = content.split('\n');
    let config: Config = Config.new();
    
    for line in lines " + 
        let trimmed: string = line.trim();
        if trimmed.starts_with('#') || trimmed.is_empty() {
            continue; // Skip comments and empty lines
         + "
        
        let parts: []string = trimmed.split('=');
        if parts.len != 2 "Invalid configuration line: " + line);
         + "
        
        let key: string = parts[0].trim();
        let value: string = parts[1].trim();
        
        match key timeout" => " + 
                let timeout_value: Result<i32, string> = value.parse_int();
                match timeout_value {
                    Result.Ok(val) => config.timeout = val,
                    Result.Err(_) => return Result.Err("Invalid timeout value: " + value)
                 + ,
            "host" => config.host = value,
            "port" => " + 
                let port_value: Result<u16, string> = value.parse_uint();
                match port_value " + 
                    Result.Ok(val) => config.port = val,
                    Result.Err(_) => return Result.Err("Invalid port value: " + value)
                 + ,
            _ => return Result.Err("Unknown configuration key: " + key)
        }
    }
    
    return Result.Ok(config);
}
```

#### `high` Priority - Critical Systems, Security-Sensitive Code

Use for operations that could impact system security, data integrity, or system stability:

```asthra
#[human_review(high)]
pub fn authenticate_user(username: string, password: string) -> Result<AuthToken, string> {// Authentication logic requires careful review for security
    let user_record: Result<User, string> = database.find_user(username);
    
    match user_record " + 
        Result.Err(_) => {
            // Constant-time response to prevent username enumeration
            let dummy_hash: []u8 = generate_dummy_hash();
            verify_password_hash(password, dummy_hash);
            return Result.Err("Authentication failed");
         + ",
        Result.Ok(user) => " + 
            let password_valid: bool = verify_password_hash(password, user.password_hash);
            if password_valid {
                let token: AuthToken = generate_auth_token(user.id);
                return Result.Ok(token);
             + " else "Authentication failed");
             + "
        }
    }
}

#[human_review(high)]
pub fn process_financial_transaction(transaction: Transaction) -> Result<Receipt, string> {// Financial operations require high-priority review
    let validation: Result<void, string> = validate_transaction_limits(transaction);
    match validation {
        Result.Err(error) => return Result.Err("Transaction validation failed: " + error),
        Result.Ok(_) => { + "
    }
    
    let authorization: Result<void, string> = authorize_transaction(transaction);
    match authorization "Transaction authorization failed: " + error),
        Result.Ok(_) => {}
    }
    
    // Execute transaction with audit logging
    let receipt: Result<Receipt, string> = execute_transaction(transaction);
    audit_log("TRANSACTION", "Executed transaction: " + transaction.id);
    
    return receipt;
}
```

#### `critical` Priority - Mandatory Human Review (Future Extension)

Reserved for future use in systems requiring mandatory human review before deployment:

```asthra
#[human_review(critical)]
pub fn nuclear_reactor_control(control_rods: []ControlRod) -> Result<void, string> {// Critical systems require mandatory human review before deployment
    // This priority level may trigger build failures without human sign-off
    for rod in control_rods " + 
        let safety_check: Result<void, string> = verify_rod_safety(rod);
        match safety_check {
            Result.Err(error) => {
                emergency_shutdown();
                return Result.Err("Safety check failed: " + error);
             + ",
            Result.Ok(_) => continue
         + "
    }
    return Result.Ok(void);
}
```

### Context-Aware Review Guidelines

Context specifications provide domain-specific guidance for human reviewers:

#### `context = "crypto"` - Cryptographic Operations

Cryptographic code requires specialized knowledge and careful review for security vulnerabilities:

```asthra
#[human_review(high, context = "crypto")]
#[constant_time]
pub fn derive_encryption_key(password: string, salt: []u8) -> Result<[]u8, string> {// Cryptographic key derivation requires expert review
    // Constant-time execution prevents timing attacks
    let iterations: u32 = 100000; // PBKDF2 iteration count
    let key_length: usize = 32;   // 256-bit key
    
    let derived_key: Result<[]u8, string> = pbkdf2_derive(
        password.as_bytes(),
        salt,
        iterations,
        key_length
    );
    
    return derived_key;
 + "

#[human_review(medium, context = "crypto")]
pub fn generate_random_salt() -> Result<[]u8, string> {// Salt generation is lower risk but still crypto-related
    let salt: []u8 = allocate_slice(16); // 128-bit salt
    let result: Result<void, string> = fill_random_bytes(salt);
    
    match result " + 
        Result.Ok(_) => return Result.Ok(salt),
        Result.Err(error) => return Result.Err("Failed to generate salt: " + error)
     + 
```

#### `context = "concurrency"` - Parallel/Async Code

Concurrent code requires review for race conditions, deadlocks, and synchronization issues:

```asthra
#[human_review(high, context = "concurrency")]
pub fn parallel_data_processing(data: []DataChunk) -> Result<[]ProcessedData, string> {// Complex concurrency patterns need careful review
    let results: []Result<ProcessedData, string> = allocate_slice(data.len);
    let tasks: []Task = allocate_slice(data.len);
    
    // Spawn parallel processing tasks
    for i in range(0, data.len) {
        tasks[i] = spawn_task(|| {
            return process_data_chunk(data[i]);
         + ");
    }
    
    // Collect results with timeout handling
    for i in range(0, tasks.len) " + 
        let result: Result<ProcessedData, string> = await_task_timeout(tasks[i], 30000); // 30 second timeout
        results[i] = result;
     + "
    
    // Check for any failures
    for result in results " + 
        match result " + 
            Result.Err(error) => return Result.Err("Processing failed: " + error),
            Result.Ok(_) => continue
         + 
    
    // Extract successful results
    let processed: []ProcessedData = allocate_slice(results.len);
    for i in range(0, results.len) " + 
        match results[i] {
            Result.Ok(data) => processed[i] = data,
            Result.Err(_) => { + " // Already handled above
        }
    }
    
    return Result.Ok(processed);
}
```

#### `context = "ffi"` - Foreign Function Interfaces

FFI code requires review for memory safety, ownership transfer, and C library integration:

```asthra
#[human_review(medium, context = "ffi")]
extern "libc" fn malloc(size: usize) -> *mut u8;

#[human_review(medium, context = "ffi")]
extern "libc" fn free(ptr: *mut u8) -> void;

#[human_review(high, context = "ffi")]
pub fn safe_c_memory_wrapper(size: usize) -> Result<[]u8, string> {// High-risk FFI operations require careful review
    if size == 0 " + 
        return Result.Err("Cannot allocate zero bytes");
     + "
    
    let ptr: *mut u8 = unsafe " +  malloc(size)  + ";
    if ptr.is_null() "Memory allocation failed");
     + "
    
    // Create safe slice wrapper
    let slice: []u8 = unsafe " +  slice_from_raw_parts(ptr, size)  + ";
    return Result.Ok(slice);
}

#[human_review(low, context = "ffi")]
extern "libc" fn strlen(s: *const u8) -> usize;

#[human_review(low, context = "ffi")]
pub fn get_c_string_length(c_str: *const u8) -> Result<usize, string> {// Simple FFI wrapper with minimal risk
    if c_str.is_null() " + 
        return Result.Err("Null pointer provided");
     + "
    
    let length: usize = unsafe " +  strlen(c_str)  + ";
    return Result.Ok(length);
 + "
```

#### `context = "security"` - Security-Critical Operations

Security-sensitive operations that don't fall into crypto or FFI categories:

```asthra
#[human_review(high, context = "security")]
pub fn validate_user_input(input: string) -> Result<string, string> {// Input validation is security-critical
    if input.len > 1000 " + 
        return Result.Err("Input too long");
     + "
    
    // Check for potential injection attacks
    let dangerous_patterns: []string = ["<script", "javascript:", "data:", "vbscript:"];
    for pattern in dangerous_patterns " + 
        if input.to_lowercase().contains(pattern) {
            return Result.Err("Potentially dangerous input detected");
         + 
    
    // Sanitize HTML entities
    let sanitized: string = input
        .replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace("\"", "&quot;")
        .replace("'", "&#x27;");
    
    return Result.Ok(sanitized);
}
```

## Security Annotations

Security annotations provide compiler-enforced guarantees for security-critical code.

### `#[constant_time]` - Timing Attack Prevention

Prevents timing-based side-channel attacks by ensuring constant execution time:

```asthra
#[constant_time]
#[human_review(high, context = "crypto")]
pub fn compare_password_hashes(hash1: []u8, hash2: []u8) -> bool " + 
    // Constant-time comparison prevents timing attacks
    if hash1.len != hash2.len " + 
        return false;
     + "
    
    let result: u8 = 0;
    for i in range(0, hash1.len) " + 
        result = result | (hash1[i] ^ hash2[i]);
     + "
    
    return result == 0;
 + "

#[constant_time]
#[human_review(high, context = "crypto")]
pub fn secure_memory_compare(a: []u8, b: []u8) -> bool " + 
    // Compiler ensures this function executes in constant time
    // regardless of input values
    if a.len != b.len " + 
        return false;
     + "
    
    let diff: u8 = 0;
    for i in range(0, a.len) " + 
        diff = diff | (a[i] ^ b[i]);
     + "
    
    return diff == 0;
 + "
```

### `#[volatile_memory]` - Memory Optimization Control

Prevents compiler optimizations that could leave sensitive data in memory:

```asthra
#[volatile_memory]
#[human_review(high, context = "security")]
pub fn clear_sensitive_data(data: []u8) -> void " + 
    // Volatile memory annotation prevents compiler from optimizing away the clear
    for i in range(0, data.len) {
        data[i] = 0;
     + "
    // Compiler cannot optimize this loop away due to volatile annotation
}

#[volatile_memory]
#[human_review(medium, context = "crypto")]
pub fn secure_key_processing(key: []u8) -> Result<[]u8, string> {// Process cryptographic key with guaranteed memory clearing
    let processed_key: []u8 = allocate_slice(key.len);
    
    // Perform key processing
    for i in range(0, key.len) " + 
        processed_key[i] = key[i] ^ 0xAA; // Example transformation
     + "
    
    // Clear original key from memory (guaranteed by volatile annotation)
    for i in range(0, key.len) " + 
        key[i] = 0;
     + "
    
    return Result.Ok(processed_key);
 + "
```

### Integration with Human Review Priorities

Security annotations often combine with human review tags for comprehensive protection:

```asthra
#[human_review(high, context = "crypto")]
#[constant_time]
#[volatile_memory]
pub fn critical_crypto_operation(secret: []u8, data: []u8) -> Result<[]u8, string> {// Triple protection: human review + timing safety + memory safety
    let result: []u8 = allocate_slice(32); // 256-bit result
    
    // Perform cryptographic operation
    let operation_result: Result<void, string> = perform_crypto_operation(secret, data, result);
    
    // Clear sensitive inputs (volatile ensures this isn't optimized away)
    for i in range(0, secret.len) " + 
        secret[i] = 0;
     + "
    
    match operation_result " + 
        Result.Ok(_) => return Result.Ok(result),
        Result.Err(error) => {
            // Clear result on error
            for i in range(0, result.len) {
                result[i] = 0;
             + "
            return Result.Err(error);
         + "
    }
}
```

## Memory Management Annotations

**Design Philosophy Clarification**: Asthra's memory management annotation system is **complete by design** for AI-first development. Our two-level system (ownership strategy + transfer semantics) intentionally provides comprehensive safety without Rust-style complexity. This section demonstrates our current **fully implemented** capabilities that already address all practical memory safety scenarios.

### Comprehensive Current Capabilities

Asthra **already supports** memory management annotations across all essential contexts:

**✅ Variable Declarations:**
```asthra
let file: #[ownership(c)] FileHandle = open_file("data.bin");
let buffer: #[ownership(pinned)] []u8 = allocate_secure_buffer();
let gc_data: #[ownership(gc)] DataStruct = create_managed_data();
```

**✅ Function Parameters and Returns:**
```asthra
pub fn process_data(#[borrowed] input: *const u8, #[transfer_full] output: *mut u8) -> #[transfer_full] *mut Result;
extern "C" fn malloc(size: usize) -> #[transfer_full] *mut void;
extern "C" fn free(#[transfer_full] ptr: *mut void);
```

**Why This Design Is AI-Optimized:**
- **Predictable Patterns**: Clear, local semantics that AI can reliably generate
- **Orthogonal Concerns**: Ownership strategy (how memory is managed) separate from transfer semantics (how ownership moves)
- **Fail-Safe Defaults**: Missing annotations default to safe garbage collection
- **Conflict Prevention**: Grammar prevents dangerous combinations like `#[transfer_full] #[transfer_none]`

Memory management annotations provide explicit control over ownership, lifetime, and memory allocation strategies.

### `#[ownership(gc|c|pinned)]` - Memory Ownership Control

Specifies memory ownership and management strategy:

```asthra
#[ownership(gc)]
pub fn create_managed_data() -> []u8 " + 
    // Garbage collector manages this memory
    let data: []u8 = allocate_slice(1024);
    return data; // GC will clean up when no longer referenced
 + "

#[ownership(c)]
#[human_review(medium, context = "ffi")]
pub fn create_c_compatible_data() -> *mut u8 " + 
    // C-style manual memory management
    let ptr: *mut u8 = unsafe { malloc(1024)  + ";
    return ptr; // Caller responsible for calling free()
}

#[ownership(pinned)]
#[human_review(high, context = "security")]
pub fn create_pinned_secure_buffer() -> []u8 " + 
    // Memory pinned in physical RAM, not swapped to disk
    let buffer: []u8 = allocate_pinned_slice(4096);
    return buffer; // Suitable for cryptographic keys
 + "
```

### FFI Transfer Annotations

Control ownership transfer across FFI boundaries:

#### `#[transfer_full]` - Full Ownership Transfer

```asthra
#[human_review(medium, context = "ffi")]
extern "crypto_lib" fn encrypt_data(
    input: *const u8,
    input_len: usize,
    #[transfer_full] output: *mut u8  // Caller owns the output buffer
) -> i32;

#[human_review(medium, context = "ffi")]
pub fn safe_encrypt_wrapper(data: []u8) -> Result<[]u8, string> {let output_size: usize = data.len + 16; // Add space for padding
    let output_buffer: *mut u8 = unsafe " +  malloc(output_size)  + ";
    
    if output_buffer.is_null() " + 
        return Result.Err("Memory allocation failed");
     + "
    
    let result: i32 = unsafe " + 
        encrypt_data(data.ptr, data.len, output_buffer)
     + ";
    
    if result != 0 " + 
        unsafe { free(output_buffer)  + "; // Clean up on error
        return Result.Err("Encryption failed");
     + "
    
    // Transfer ownership to Asthra slice
    let encrypted: []u8 = unsafe " +  slice_from_raw_parts(output_buffer, output_size)  + ";
    return Result.Ok(encrypted);
}
```

#### `#[transfer_none]` - No Ownership Transfer

```asthra
#[human_review(low, context = "ffi")]
extern "libc" fn strlen(
    #[transfer_none] s: *const u8  // Caller retains ownership
) -> usize;

#[human_review(low, context = "ffi")]
pub fn get_string_length(s: string) -> usize " + 
    // String remains owned by caller
    let c_str: *const u8 = s.as_ptr();
    let length: usize = unsafe { strlen(c_str)  + ";
    return length;
    // s is still valid here and will be cleaned up by caller
}
```

### `#[borrowed]` - Lifetime Annotations

Indicates borrowed references with limited lifetime:

```asthra
#[human_review(medium, context = "ffi")]
extern "crypto_lib" fn hash_data(
    #[borrowed] input: *const u8,  // Borrowed for duration of call
    input_len: usize,
    #[transfer_full] output: *mut u8
) -> i32;

#[human_review(medium, context = "ffi")]
pub fn compute_hash(data: []u8) -> Result<[]u8, string> {let hash_buffer: *mut u8 = unsafe " +  malloc(32)  + "; // SHA-256 output
    
    if hash_buffer.is_null() " + 
        return Result.Err("Memory allocation failed");
     + "
    
    // data is borrowed for the duration of the hash_data call
    let result: i32 = unsafe " + 
        hash_data(data.ptr, data.len, hash_buffer)
     + ";
    // data is still valid here
    
    if result != 0 " + 
        unsafe { free(hash_buffer)  + ";
        return Result.Err("Hash computation failed");
     + "
    
    let hash: []u8 = unsafe " +  slice_from_raw_parts(hash_buffer, 32)  + ";
    return Result.Ok(hash);
}
```

## Best Practices

### Annotation Combinations

Effective use of annotations often involves combining multiple annotation types:

```asthra
// Comprehensive security-critical function
#[human_review(high, context = "crypto")]
#[constant_time]
#[volatile_memory]
#[ownership(pinned)]
pub fn process_master_key(key_material: []u8) -> Result<MasterKey, string> {// All security measures applied:
    // - High-priority human review with crypto context
    // - Constant-time execution prevents timing attacks
    // - Volatile memory prevents optimization of sensitive data
    // - Pinned memory prevents swapping to disk
    
    let processed_key: MasterKey = derive_master_key(key_material);
    
    // Clear input material (volatile ensures this happens)
    for i in range(0, key_material.len) " + 
        key_material[i] = 0;
     + "
    
    return Result.Ok(processed_key);
 + "

// FFI function with comprehensive annotations
#[human_review(medium, context = "ffi")]
extern "secure_lib" fn secure_operation(
    #[borrowed] input: *const u8,
    input_len: usize,
    #[transfer_full] output: *mut u8,
    output_len: *mut usize
) -> i32;
```

### AI Code Generation Guidelines

When generating code with annotations, AI systems should follow these guidelines:

#### Priority Assignment Rules

1. **Start with `low`** for simple, well-understood operations
2. **Use `medium`** for complex logic, multiple error paths, or non-trivial algorithms
3. **Use `high`** for security-sensitive, financial, or system-critical operations
4. **Reserve `critical`** for life-safety or national security applications

#### Context Selection

1. **`crypto`** - Any cryptographic operations, key management, or security primitives
2. **concurrency** - Parallel processing, async operations, or synchronization
3. **ffi** - Foreign function interfaces, C library integration, or unsafe operations
4. **security** - Input validation, authentication, authorization, or data sanitization

#### Annotation Placement

```asthra
// Correct: Annotations before function declaration
#[human_review(medium)]
#[constant_time]
pub fn secure_function() -> Result<void, string> {// Function implementation
 + "

// Correct: Parameter-level annotations
extern "lib" fn c_function(
    #[transfer_none] input: *const u8,
    #[transfer_full] output: *mut u8
) -> i32;

// Correct: Multiple annotations on same declaration
#[human_review(high, context = "crypto")]
#[constant_time]
#[volatile_memory]
pub fn crypto_function() -> Result<[]u8, string> {// Implementation
 + "
```

### Human Review Workflows

Human reviewers should follow structured workflows based on annotation priorities:

#### High Priority Review Checklist

- [ ] **Security Analysis**: Check for potential vulnerabilities
- [ ] **Error Handling**: Verify all error paths are handled correctly
- [ ] **Resource Management**: Ensure proper cleanup of resources
- [ ] **Input Validation**: Verify all inputs are properly validated
- [ ] **Cryptographic Correctness**: For crypto context, verify algorithm usage
- [ ] **Concurrency Safety**: For concurrency context, check for race conditions
- [ ] **Memory Safety**: For FFI context, verify memory management

#### Medium Priority Review Checklist

- [ ] **Logic Correctness**: Verify algorithm implementation
- [ ] **Error Handling**: Check major error paths
- [ ] **Performance**: Identify potential performance issues
- [ ] **Maintainability**: Assess code clarity and structure

#### Low Priority Review Checklist

- [ ] **Basic Correctness**: Spot-check for obvious errors
- [ ] **Style Consistency**: Verify adherence to coding standards

### Performance Considerations

Annotations have minimal runtime overhead but can affect compilation:

- **`#[constant_time]`** may prevent certain compiler optimizations
- **`#[volatile_memory]`** prevents memory access optimizations
- **`#[ownership]`** affects memory allocation strategy
- **`#[human_review]`** has no runtime impact (compile-time only)

## Tooling Integration

### Build System Processing

The Asthra build system can process annotations for various purposes:

```toml
# Asthra.toml
[build]
process_annotations = true
generate_review_reports = true
fail_on_high_priority_review = false

[annotations]
human_review_output = "review-report.json"
security_analysis = true
generate_documentation = true

[review_workflow]
high_priority_blocking = true
medium_priority_warning = true
context_specific_routing = true
```

### Static Analysis Integration

Annotations enable enhanced static analysis:

```bash
# Extract all high-priority review items
asthra analyze --annotations --priority high --output high-priority.json

# Generate security-focused analysis
asthra analyze --annotations --context crypto,security --output security-review.json

# Check for annotation compliance
asthra lint --check-annotations --require-review-tags
```

### IDE Support and Tooling

Modern IDEs can leverage annotation metadata:

- **Syntax Highlighting**: Different colors for different review priorities
- **Code Completion**: Suggest appropriate annotation combinations
- **Inline Warnings**: Highlight high-priority items requiring review
- **Navigation**: Jump to all functions with specific annotation patterns

### CI/CD Pipeline Integration

Integrate annotation processing into continuous integration:

```yaml
# .github/workflows/review.yml
name: Code Review Analysis

on: [push, pull_request]

jobs:
  annotation-analysis:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      
      - name: Install Asthra
        run: |
          curl -L https://releases.asthra-lang.org/latest/asthra-linux.tar.gz | tar -xz
          sudo mv asthra /usr/local/bin/
      
      - name: Extract Review Annotations
        run: |
          asthra analyze --annotations --output review-report.json
          
      - name: Check High Priority Items
        run: |
          HIGH_COUNT=$(asthra analyze --annotations --priority high --count)
          if [ "$HIGH_COUNT" -gt 0 ]; then
            echo "Found $HIGH_COUNT high priority review items"
            echo "Manual review required before merge"
            exit 1
          fi
          
      - name: Generate Review Summary
        run: |
          asthra review-report --format markdown --output review-summary.md
          
      - name: Upload Review Report
        uses: actions/upload-artifact@v3
        with:
          name: review-report
          path: review-summary.md
```

## Package-Scope Annotations

With Asthra's **Go-style package system**, annotations can be applied at different scopes to provide comprehensive guidance for multi-package projects.

### Package-Level Organization

Annotations work seamlessly across package boundaries:

```asthra
// models/user.asthra
package models;

#[human_review(high, context = "security")]
pub fn create_user(username: string, email: string) -> User {
    // Public API requires high review
    return User {
        id: generate_id(),  // Private function, inherits context
        username: username,
        email: email,
    };
}

fn generate_id() -> i32 {
    // Private function - lower review priority
    return 42;
}
```

```asthra
// handlers/api.asthra
package handlers;

import "models";

#[human_review(medium, context = "security")]
pub fn handle_create_user(username: string, email: string) -> ApiResponse {
    // API boundary functions need security review
    let user: models.User = models.create_user(username, email);
    return create_success_response(user);
}
```

### Cross-Package Annotation Guidelines

When organizing annotations across packages:

1. **Package APIs**: Functions marked `pub` that cross package boundaries should have explicit review annotations
2. **Internal Functions**: Private functions can have lower priority unless they handle sensitive operations
3. **Data Flow**: Trace annotation requirements through import chains
4. **Context Inheritance**: Related packages can share annotation contexts

### Build System Integration

The build system can analyze annotations across package boundaries:

```bash
# Analyze annotations across all packages
asthra analyze --annotations --all-packages

# Check package-specific annotation coverage
asthra analyze --annotations --package models --coverage

# Generate cross-package security review
asthra analyze --context security --cross-package-deps
```

**For comprehensive package organization guidance, see [Package Organization](package-organization.md)**

## Implementation Status

### Currently Implemented ✅

- **Basic Human Review Tags**: `#[human_review(low|medium|high)]` syntax
- **Priority Level Processing**: Compiler recognizes and validates priority levels
- **Grammar Integration**: Full parser support for annotation syntax
- **Basic Tooling**: Command-line tools for annotation extraction

### Partially Implemented 🔧

- **Context Parameters**: `context = "crypto"` syntax parsed but not fully processed
- **Security Annotations**: `#[constant_time]` and `#[volatile_memory]` parsed but enforcement incomplete
- **FFI Annotations**: Transfer and lifetime annotations parsed but not enforced

### Planned for Future Releases 📋

- **Critical Priority Level**: `#[human_review(critical)]` with build-blocking behavior
- **Advanced Context Routing**: Automatic assignment of reviewers based on context
- **IDE Integration**: Full IDE plugin support for annotation visualization
- **Advanced Static Analysis**: Deep semantic analysis of annotated code

## Conclusion

Asthra's annotation system provides a powerful foundation for AI-human collaboration in code development. By combining human review tags with security and memory management annotations, developers can create robust, secure, and maintainable code with clear guidance for both AI generators and human reviewers.

The system's design prioritizes practicality and efficiency, enabling targeted human review where it matters most while allowing AI systems to handle routine operations with minimal oversight. As the annotation system continues to evolve, it will become an even more powerful tool for building reliable, secure software in an AI-first development environment.
