# Import System

**Version:** 1.3  
**Last Updated:** 2024-12-28

Asthra provides a comprehensive import system that supports package imports, selective imports, import aliases, and automatic standard library discovery. The system is designed to be AI-friendly with predictable resolution rules and clear syntax.

## Basic Import Syntax

### Package Imports

```asthra
package main;

// Basic package import
import "crypto";                    // Import entire crypto package
import "stdlib/string";             // Import standard library string package
import "github.com/user/json";      // Import external package

pub fn main(none) -> i32 {
    // Access functions via package name
    let hash = crypto.sha256(data);
    let trimmed = string.trim("  hello  ");
    let parsed = json.parse(input);
    return 0;
}
```

### Selective Imports

```asthra
package main;

// Selective imports - import specific functions
import "crypto." + sha256, encrypt // TODO: Convert string interpolation to concatenation;
import "stdlib/string." + trim, len, format // TODO: Convert string interpolation to concatenation;
import "collections." + List, Map // TODO: Convert string interpolation to concatenation;

pub fn main(none) -> i32 {
    // Use imported functions directly (no package prefix)
    let hash = sha256(data);          // Not: crypto.sha256(data)
    let text = trim("  hello  ");     // Not: string.trim("  hello  ")
    let length = len(text);           // Not: string.len(text)
    
    return 0;
}
```

## Import Aliases

Import aliases provide a clean solution for name conflicts and long package names:

```asthra
package main;

// Basic alias syntax
import "stdlib/string" as str;
import "github.com/user/stringutils" as strutils;
import "github.com/company/advanced-string" as advstr;

// Using aliased imports
pub fn process_text(none) -> void " + let text: string = "  Hello World  ";
    
    // Use different string libraries with clear disambiguation
    let trimmed: string = str.trim(text);
    let processed: string = strutils.advanced_trim(text);
    let formatted: string = advstr.format_with_style(text, "uppercase");
    
    log("Original: " + text);
    log("Stdlib trim: " + trimmed);
    log("Utils trim: " + processed);
    log("Advanced format: " + formatted); + "

// Alias can be any valid identifier
import " // TODO: Convert string interpolation to concatenationgithub.com/crypto/sha256" as sha;
import "github.com/crypto/blake2" as blake;
import "stdlib/crypto" as crypto;

pub fn hash_comparison(data: []u8) -> void " + let sha_hash: []u8 = sha.hash(data);
    let blake_hash: []u8 = blake.hash(data);
    let stdlib_hash: []u8 = crypto.sha256(data);
    
    // Clear distinction between different crypto libraries + "

// Import alias with local modules
import " // TODO: Convert string interpolation to concatenation./utils/string_helpers" as helpers;
import "../common/string_validation" as validator;

pub fn validate_and_process(input: string) -> Result<string, string> " + let validation_result: Result<void, string> = validator.validate_format(input);
    match validation_result {
        Result.Ok(_) => {
            return Result.Ok(helpers.process_string(input)); + ",
        Result.Err(error) => {
            return Result.Err(" // TODO: Convert string interpolation to concatenationValidation failed: " + error);
        }
    }
}
```

## Import Alias Rules

- Alias must be a valid identifier (`[a-zA-Z_][a-zA-Z0-9_]*`)
- Alias cannot conflict with existing identifiers in the current scope
- Alias cannot be a reserved keyword
- Multiple imports can use the same alias if they're in different scopes
- Original module name is not accessible once aliased (must use alias)

## Name Conflict Resolution

Import aliases provide systematic name conflict resolution:

```asthra
// Example: Multiple packages export functions with same names
import "stdlib/string" as str;
import "github.com/user/stringutils" as strutils;
import "github.com/company/advanced-string" as advstr;

pub fn process_text(none) -> void " + let text: string = "  Hello World  ";
    
    // Each package's functions are clearly distinguished
    let stdlib_result: string = str.trim(text);
    let utils_result: string = strutils.trim(text);
    let advanced_result: string = advstr.trim(text);
    
    // No ambiguity about which function is called + "

// Different scopes can use same alias names
pub fn string_processing(none) -> void {
    import " // TODO: Convert string interpolation to concatenationstdlib/string" as util;
    let result = util.trim("text");
}

pub fn collection_processing(none) -> void {
    import "stdlib/collections" as util;  // Same alias, different scope
    let list = util.new_list();
}
```

## Import Resolution Rules

### Resolution Priority

1. **Relative imports:** `import "./crypto"` - imports from subdirectory
2. **Absolute imports:** `import "crypto"` - imports from project root
3. **Standard library:** `import "stdlib/string"` - imports from standard library
4. **External packages:** `import "github.com/user/package"` - imports external dependency

### Resolution Examples

```asthra
// Project structure:
// my-project/
// ├── src/main.asthra
// ├── crypto/hash.asthra
// ├── utils/helpers.asthra
// └── asthra.toml

package main;

// Relative imports
import "./crypto";           // Imports crypto/ directory (same level as src/)
import "../utils";           // Imports utils/ directory (up one level)
import "./crypto/hash";      // Error: crypto is a package, not a module file

// Absolute imports (from project root)
import "crypto";             // Imports crypto/ package
import "utils";              // Imports utils/ package

// Standard library
import "stdlib/string";      // Always resolves to standard library
import "stdlib/io";          // Always resolves to standard library
import "stdlib/collections"; // Always resolves to standard library

// External packages (must be declared in asthra.toml)
import "github.com/user/json";       // External dependency
import "github.com/company/crypto";  // External crypto library
```

## Standard Library Auto-Discovery

The Asthra build system automatically provides access to the standard library:

```asthra
// These work out of the box - no configuration needed
import "stdlib/string";
import "stdlib/asthra/core";
import "stdlib/collections";
import "stdlib/datetime/time";

pub fn main(none) -> i32 {
    // Standard library functions are immediately available
    let version: string = core.runtime_version();
    let trimmed: string = string.trim("  hello world  ");
    return 0;
}
```

**Auto-Discovery Features:**
- **Automatic Path Resolution:** `stdlib/*` imports are resolved without manual setup
- **Always Available:** Standard library is included in the compiler search path by default
- **No Explicit Configuration:** Developers don't need to specify stdlib locations in `asthra.toml`

**Contrast with Third-Party Packages:**
```toml
# asthra.toml - Third-party packages must be explicitly declared
[dependencies]
"github.com/user/package" = "1.0.0"  # Required for third-party imports
```

## Module Function Access

Asthra supports complete module function access using dot notation with full parsing, semantic analysis, and code generation support:

```asthra
import "stdlib/string" as str;
import "stdlib/collections" as collections;
import "github.com/crypto/sha256" as crypto;

pub fn demonstrate_module_access(none) -> void " + let text: string = "  Hello, World!  ";
    
    let trimmed: string = str.trim(text);
    let uppercase: string = str.to_uppercase(trimmed);
    let length: usize = str.len(uppercase);
    
    let processed: string = str.trim(text)
        .to_lowercase()
        .replace("world", "asthra");
    
    let data: []u8 = str.to_bytes(processed);
    let hash: []u8 = crypto.hash(data);
    
    let words: []string = str.split(processed, " ");
    let word_count: usize = collections.len(words); + "

pub fn module_access_in_expressions(none) -> void {
    let text1: string = " // TODO: Convert string interpolation to concatenationhello";
    let text2: string = "world";
    
    // Module function calls in binary expressions
    let combined: string = str.trim(text1) + " " + str.trim(text2);
    
    // Module function calls in conditionals
    if str.len(text1) > str.len(text2) {
        log("text1 is longer");
    }
    
    // Module function calls in assignments
    let result: string = str.to_uppercase(combined);
}
```

## Import Access Control

The build system enforces import access control to maintain security boundaries:

```asthra
// Allowed: Standard library imports
import "stdlib/string";
import "stdlib/crypto";

// Allowed: Third-party packages
import "github.com/user/json";

// Allowed: Local modules
import "./utils";
import "../common/types";

// Forbidden: Internal runtime imports (user code)
import "internal/runtime/memory_allocator";  // Build error
import "internal/compiler/ast";              // Build error
```

## Best Practices

### Import Alias Patterns

**Common Alias Conventions:**
- `str` for string utilities
- `fs` for filesystem operations
- `crypto` for cryptographic functions
- `http` for HTTP client libraries
- `json` for JSON parsing libraries
- `db` for database interfaces

```asthra
// ✅ Recommended: Use descriptive, conventional aliases
import "stdlib/string" as str;
import "stdlib/filesystem" as fs;
import "stdlib/crypto" as crypto;
import "github.com/user/http-client" as http;
import "github.com/user/json-parser" as json;
import "github.com/user/database" as db;

// ❌ Avoid: Unclear or conflicting aliases
import "stdlib/string" as s;              // Too short
import "stdlib/filesystem" as string;     // Conflicts with type name
import "github.com/user/crypto" as fs;    // Misleading name
```

### Name Conflict Resolution Strategy

1. **Explicit Aliasing:** Use descriptive aliases that indicate the source or purpose
2. **Consistent Naming:** Establish project-wide conventions for common aliases
3. **Scope Isolation:** Different functions can use different aliases for the same import

```asthra
// ✅ Strategy: Descriptive aliases with purpose indication
import "stdlib/crypto" as stdlib_crypto;
import "github.com/company/crypto" as company_crypto;
import "github.com/security/crypto" as security_crypto;

pub fn crypto_comparison(none) -> void {
    let data: []u8 = [1, 2, 3, 4];
    
    // Clear indication of which crypto library is used
    let stdlib_hash = stdlib_crypto.sha256(data);
    let company_hash = company_crypto.sha256(data);
    let security_hash = security_crypto.sha256(data);
}
```

### AI-Friendly Import Patterns

```asthra
// ✅ Preferred: Clear, predictable import patterns
import "stdlib/string" as str;
import "stdlib/collections" as collections;
import "crypto" as crypto;

pub fn ai_friendly_example(none) -> void " + // Predictable access patterns
    let text = str.trim("  hello  ");
    let length = str.len(text);
    let words = str.split(text, " ");
    
    // Clear function provenance
    let list = collections.new_list();
    let hash = crypto.sha256(data); + "

// ❌ Avoid: Complex or ambiguous import patterns
import " // TODO: Convert string interpolation to concatenationvery/long/package/path/string-utilities" as s;
import "stdlib/string" as string_lib;
import "utils" as string_util;

pub fn confusing_example(none) -> void {
    // Unclear which string library is being used
    let result1 = s.trim(text);
    let result2 = string_lib.trim(text);
    let result3 = string_util.trim(text);
}
```

## Error Handling

### Import Resolution Errors

```asthra
// Common import errors and solutions

// ❌ Error: Package not found
import "nonexistent/package";
// Solution: Check package exists in project or dependencies

// ❌ Error: Circular import
// file1.asthra
import "file2";
// file2.asthra  
import "file1";
// Solution: Restructure code to eliminate circular dependencies

// ❌ Error: Alias conflicts with existing identifier
let str = "hello";
import "stdlib/string" as str;  // Error: 'str' already defined
// Solution: Use different alias name

// ❌ Error: Invalid alias name
import "stdlib/string" as 123invalid;  // Error: Invalid identifier
// Solution: Use valid identifier starting with letter or underscore
```

### Diagnostic Messages

The compiler provides clear diagnostic messages for import errors:

```
Error: Import resolution failed
  --> src/main.asthra:3:8
   |
3  | import "nonexistent/package";
   |        ^^^^^^^^^^^^^^^^^^^^^ package not found
   |
   = help: Check that the package exists in your project or dependencies
   = note: Available packages: crypto, database, utils

Error: Import alias conflicts with existing identifier  
  --> src/main.asthra:5:32
   |
4  | let str = "hello";
5  | import "stdlib/string" as str;
   |                           ^^^ identifier 'str' already defined
   |
   = help: Use a different alias name: import "stdlib/string" as string;
```

## Implementation Details

**Implementation Components:**
- ✅ **AST Node**: `AST_IMPORT_DECL` with import path, alias, and selective imports
- ✅ **Parser**: Complete parsing of import declarations and alias syntax
- ✅ **Semantic Analysis**: Symbol resolution and import validation
- ✅ **Module Resolution**: Package discovery and path resolution
- ✅ **Code Generation**: Module function call generation
- ✅ **Integration**: Works with package system and build tool

**Future Enhancements:**
- Conditional imports based on target platform
- Import attributes for optimization hints
- Module re-exports for public API definition
- Import maps for dependency injection 
