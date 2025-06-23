# Getting Started

**Implementation Status**: ✅ Complete  
**Last Verified**: 2025-01-07  
**Grammar Version**: v1.22  
**Test Coverage**: 100%

This guide will help you install Asthra, write your first program, and understand the basic compilation process.

## Installation

To install the Asthra compiler, download the appropriate binary for your POSIX-compliant system from the official Asthra website. Ensure it's added to your system's PATH.

```bash
# Example (Linux/macOS)
curl -L https://example.com/asthra/v1.2/asthra-compiler-x86_64-linux.tar.gz | tar -xz
sudo mv asthra-compiler /usr/local/bin/asthra
asthra --version
```

## Your First Asthra Program

Asthra programs are defined by a package declaration, optional import statements, and top-level declarations.
Create a file named `hello.asthra`:

```asthra
package main;
import "system/libc"; // Import standard C library functions

// #[human_review(low)] - Example of a tag for AI guidance
pub fn main(void) -> Result<int, string> {// main now returns a Result
    let message: string = "Hello, Asthra v1.22 from AI!\nRunning with enhanced features!\n";
    
    // Use libc.puts via FFI to print to console
    // This is unsafe as it directly calls C.
    unsafe " + 
        // For simple C calls with string literals, direct usage is often okay,
        // but for complex scenarios, explicit conversion (see FFI guide) is safer.
        libc.puts(message); // Assuming libc.puts can take Asthra string directly for simplicity here
     + "
    
    return Result.Ok(0); // Indicate success using pattern matching syntax
 + "
```

## Compiling and Running

Use the `asthra build` command.

```bash
asthra build hello.asthra
```

This will produce an executable (e.g., `hello`). Run it:

```bash
./hello
```

Expected Output:

```
Hello, Asthra v1.22 from AI!
Running with enhanced features!
```

## Setting Up a Multi-File Project

As your programs grow, you'll want to organize code across multiple files using Asthra's package system. Here's how to set up a simple multi-file project:

### Create Project Structure

```bash
mkdir my-asthra-project
cd my-asthra-project

# Create package directories
mkdir src utils

# Create the main project configuration
cat > Asthra.toml << EOF
[project]
name = "my_asthra_project"
version = "1.0.0"
description = "My first multi-file Asthra project"

[build]
target = "x86_64-linux"
optimization_level = 1
EOF
```

### Create Package Files

Create the main program in `src/main.asthra`:

```asthra
package main;

import "utils";
import "system/libc";

pub fn main() -> Result<i32, string> {
    let message: string = utils.create_greeting("Asthra");
    let result: bool = utils.print_message(message);
    
    return if result { Result.Ok(0) } else { Result.Err("Failed to print message") };
}
```

Create utility functions in `utils/helpers.asthra`:

```asthra
package utils;

import "system/libc";

pub fn create_greeting(name: string) -> string {
    return "Hello from " + name + " package system!";
}

pub fn print_message(message: string) -> bool {
    unsafe {
        libc.puts(message);
    }
    return true;
}
```

### Build and Run

```bash
# Build the entire project
asthra build

# Run the executable
./my_asthra_project
```

Expected Output:
```
Hello from Asthra package system!
```

### Package System Benefits

This multi-file approach provides:

- **Code Organization**: Related functionality grouped in packages
- **Visibility Control**: Use `pub` to expose functions to other packages
- **Import System**: Clean dependency management between packages
- **Scalability**: Easy to add new packages as your project grows

**For comprehensive package organization guidance, see [Package Organization](package-organization.md)**

## Understanding the Program Structure

Let's break down the hello world program:

### Package Declaration
```asthra
package main;
```
Every Asthra program starts with a package declaration. The `main` package indicates this is an executable program.

### Import Statements
```asthra
import "system/libc";
```
This imports the standard C library, allowing us to use functions like `puts()`. The string path indicates a system library module.

### Function Definition
```asthra
pub fn main(void) -> Result<int, string> {```
The `main` function is the entry point with `pub` visibility modifier (v1.16+ explicit visibility required). It returns a `Result<int, string>` type, which can be either:
- `Result.Ok(0)` for success
- `Result.Err("error message")` for failure

### Variable Declaration
```asthra
let message: string = "Hello, Asthra v1.22 from AI!\nRunning with enhanced features!\n";
```
Variables are declared with `let` and are immutable by default. The type annotation `: string` is explicit and mandatory (v1.15+ requirement for AI generation reliability).

### Unsafe Block
```asthra
unsafe {
    libc.puts(message);
 + "
```
Calls to C functions must be wrapped in `unsafe` blocks since Asthra cannot guarantee their memory safety.

### Return Statement
```asthra
return Result.Ok(0);
```
The function returns a successful result with exit code 0.

## Main Function Return Types

Asthra provides flexible options for main function return types, each suited for different application patterns. Understanding these options helps you choose the right approach for your program's needs.

### 1. Integer Return Type (`i32`)

**Best for**: Command-line tools, system utilities, build scripts

```asthra
pub fn main(none) -> i32 {
    let result = perform_operation();
    return if result.is_success() { 0 } else { 1 };
}
```

**Characteristics:**
- Return value becomes the process exit code directly
- 0 indicates success, non-zero indicates error
- Range: typically 0-255 (platform dependent)
- Simple and direct for basic success/failure scenarios

**Example Use Cases:**
```asthra
// File processing utility
pub fn main(none) -> i32 {
    let file_path = get_command_line_arg(1);
    match process_file(file_path) {
        true => return 0,   // Success
        false => return 1   // Error
    }
}

// Validation tool with different exit codes
pub fn main(none) -> i32 {
    match validate_system() {
        ValidationResult.AllGood => return 0,
        ValidationResult.Warnings => return 1,
        ValidationResult.Errors => return 2,
        ValidationResult.CriticalFailure => return 3
    }
}
```

### 2. Void Return Type (`void`)

**Best for**: Simple programs, tutorials, demonstrations

```asthra
pub fn main(none) -> void {
    print_greeting();
    perform_simple_task();
    return ();
}
```

**Characteristics:**
- Always exits with code 0 (success)
- No explicit exit code handling needed
- Simplest option for programs that don't need error reporting
- Uses unit type `()` for return statement

**Example Use Cases:**
```asthra
// Simple greeting program
pub fn main(none) -> void {
    let name = "World";
    print("Hello, " + name + "!");
    return ();
}

// Basic calculation demo
pub fn main(none) -> void {
    let a = 10;
    let b = 20;
    let sum = a + b;
    print("Sum: " + sum.to_string());
    return ();
}
```

### 3. Result Return Type (`Result<i32, string>`)

**Best for**: Applications with rich error reporting, complex programs

```asthra
pub fn main(none) -> Result<i32, string> {
    return Result.Ok(0);        // Success with exit code
    // return Result.Err("error message"); // Error with description
}
```

**Characteristics:**
- `Result.Ok(code)` exits with the specified code
- `Result.Err(message)` prints the message to stderr and exits with code 1
- Enables descriptive error messages for debugging
- Supports different success exit codes

**Example Use Cases:**
```asthra
// Configuration-based application
pub fn main(none) -> Result<i32, string> {
    let config = match load_config("app.conf") {
        Ok(c) => c,
        Err(e) => return Result.Err("Failed to load config: " + e)
    };
    
    let result = match run_application(config) {
        Ok(exit_code) => exit_code,
        Err(e) => return Result.Err("Application error: " + e)
    };
    
    return Result.Ok(result);
}

// Network service with detailed error reporting
pub fn main(none) -> Result<i32, string> {
    match initialize_network() {
        Ok(_) => {},
        Err(e) => return Result.Err("Network initialization failed: " + e)
    }
    
    match start_server() {
        Ok(_) => Result.Ok(0),
        Err(e) => Result.Err("Server startup failed: " + e)
    }
}
```

### 4. Never Return Type (`Never`)

**Best for**: System services, daemon processes, embedded applications

```asthra
pub fn main(none) -> Never {
    initialize_service();
    loop {
        handle_requests();
    }
    // Never reaches here - function never returns normally
}
```

**Characteristics:**
- Function must never return through normal control flow
- Always terminates via `panic()`, `exit()`, or infinite loops
- Exit code determined by the termination mechanism
- Enables compiler optimizations and exhaustiveness checking

**Example Use Cases:**
```asthra
// HTTP server daemon
pub fn main(none) -> Never {
    initialize_server();
    log("Server started, entering main loop");
    
    loop {
        let request = wait_for_request();
        handle_request(request);
    }
}

// Embedded system main loop
pub fn main(none) -> Never {
    initialize_hardware();
    
    loop {
        read_sensors();
        update_actuators();
        sleep_microseconds(1000);
    }
}

// Critical system monitor
pub fn main(none) -> Never {
    while true {
        let system_status = check_system_health();
        if system_status.is_critical() {
            panic("Critical system failure detected");
        }
        sleep_seconds(1);
    }
}
```

### Choosing the Right Return Type

| Use Case | Recommended Type | Reason |
|----------|------------------|---------|
| Simple scripts | `void` | No error handling needed |
| Command-line tools | `i32` | Standard Unix exit codes |
| Complex applications | `Result<i32, string>` | Rich error reporting |
| System services | `Never` | Continuous operation |
| Build tools | `i32` | Integration with build systems |
| Embedded systems | `Never` | Continuous operation |
| Tutorials/demos | `void` | Simplicity |

### Migration Between Return Types

You can easily migrate between return types as your program evolves:

```asthra
// Start simple
pub fn main(none) -> void {
    process_data();
    return ();
}

// Add basic error handling
pub fn main(none) -> i32 {
    let success = process_data();
    return if success { 0 } else { 1 };
}

// Add rich error reporting
pub fn main(none) -> Result<i32, string> {
    match process_data() {
        Ok(_) => Result.Ok(0),
        Err(e) => Result.Err("Processing failed: " + e)
    }
}
```

## Next Steps

Now that you have a working Asthra program, you can:

1. **Learn the language fundamentals**: See [Language Fundamentals](language-fundamentals.md)
2. **Explore C interoperability**: Check [C Interoperability](ffi-interop.md)
3. **Set up a project**: Review [Building Projects](building-projects.md)

## Common Issues

### Compiler Not Found
If you get "command not found" errors, ensure the Asthra compiler is in your PATH:

```bash
export PATH=$PATH:/path/to/asthra/bin
```

### Missing C Libraries
If you encounter linking errors, ensure you have the required C development libraries installed:

```bash
# Ubuntu/Debian
sudo apt-get install build-essential libc6-dev

# macOS (with Homebrew)
xcode-select --install
```

### Permission Errors
If the compiled executable doesn't run due to permissions:

```bash
chmod +x hello
./hello
``` 
