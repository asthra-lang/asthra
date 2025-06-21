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
