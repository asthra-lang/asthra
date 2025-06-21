# Package Organization

**Version:** 1.3  
**Last Updated:** 2024-12-28

Asthra adopts Go's package organization model to improve AI editor compatibility and code maintainability. This system allows large packages to be split across multiple files while maintaining clear organizational boundaries.

## Directory-Based Packages

All source files in a given directory must belong to the same package:

```
my-project/
├── asthra.toml
├── src/
│   ├── main.asthra          # package main
│   ├── types.asthra         # package main  
│   ├── utils.asthra         # package main
│   └── handlers.asthra      # package main
├── crypto/
│   ├── hash.asthra          # package crypto
│   ├── encrypt.asthra       # package crypto
│   ├── decrypt.asthra       # package crypto
│   └── keys.asthra          # package crypto
├── database/
│   ├── connection.asthra    # package database
│   ├── queries.asthra       # package database
│   └── models.asthra        # package database
└── tests/
    ├── main_test.asthra     # package main (same package testing)
    ├── crypto_test.asthra   # package crypto_test (black-box testing)
    └── database_test.asthra # package database_test (black-box testing)
```

## Package Declaration Rules

Every `.asthra` file must start with a package declaration:

```asthra
// src/main.asthra
package main;

pub fn main(none) -&gt; i32 {
    let result = process_data();
    return result;
}

// src/types.asthra  
package main;  // Must match other files in src/ directory

pub struct UserData {
    id: i32;
    name: string;
    email: string;
}

// src/utils.asthra
package main;  // Must match other files in src/ directory

pub fn process_data(none) -&gt; i32 {
    let user = UserData {
        id: 1,
        name: "John",
        email: "john@example.com"
    };
    return validate_user(user);
}

pub fn validate_user(user: UserData) -&gt; i32 {
    if user.id &gt; 0 {
        return 0;
    }
    return 1;
}
```

## Cross-Package Access

Functions and types can be accessed across packages using import statements:

```asthra
// crypto/hash.asthra
package crypto;

pub fn sha256(data: &#91;&#93;u8) -&gt; &#91;&#93;u8 {
    // Implementation
}

pub fn internal_helper(none) -&gt; void {
    // Private function, not accessible outside package
}

// crypto/encrypt.asthra  
package crypto;  // Same package, can access internal_helper

pub fn encrypt_data(data: &#91;&#93;u8, key: &#91;&#93;u8) -&gt; &#91;&#93;u8 {
    internal_helper();  // Can call private functions within same package
    let hash = sha256(data);  // Can call public functions within same package
    // Implementation
}

// src/main.asthra
package main;

import "crypto";  // Import the crypto package

pub fn main(none) -&gt; i32 {
    let data: &#91;&#93;u8 = &#91;1, 2, 3, 4&#93;;
    let key: &#91;&#93;u8 = &#91;5, 6, 7, 8&#93;;
    
    let hash = crypto.sha256(data);        // ✅ Can access public functions
    let encrypted = crypto.encrypt_data(data, key);  // ✅ Can access public functions
    // crypto.internal_helper();           // ❌ Cannot access private functions
    
    return 0;
}
```

## Visibility Rules

Asthra uses a simple public/private visibility system:

```asthra
// database/models.asthra
package database;

// Public struct - can be imported by other packages
pub struct User {
    pub id: i32;           // Public field
    pub name: string;      // Public field
    email: string;         // Private field (package-only access)
}

// Private struct - only accessible within database package
pub struct ConnectionConfig {
    host: string;
    port: i32;
    credentials: string;
}

// Public function
pub fn create_user(name: string, email: string) -&gt; User {
    return User {
        id: generate_id(),  // Can call private functions
        name: name,
        email: email
    };
}

// Private function - only accessible within database package  
pub fn generate_id(none) -&gt; i32 {
    // Implementation
    return 42;
}

// database/connection.asthra
package database;  // Same package

pub fn connect(none) -&gt; Result&lt;Connection, string&gt; {
    // Can access private types and functions from same package
    let config = ConnectionConfig {
        host: "localhost",
        port: 5432,
        credentials: load_credentials()
    };
    
    let user_id = generate_id();  // ✅ Can call private functions
    // Implementation
}
```

## Test File Convention

Test files follow Go's `_test.asthra` convention with special package rules:

```asthra
// crypto/hash_test.asthra - Same package testing (white-box)
package crypto;

import "stdlib/testing";

pub fn test_sha256(none) -&gt; void " + let data: &#91;&#93;u8 = &#91;1, 2, 3&#93;;
    let result = sha256(data);  // Can access public functions
    internal_helper();          // ✅ Can access private functions (same package)
    
    testing.assert_eq(result.len, 32); + "

// tests/crypto_test.asthra - Separate package testing (black-box)  
package crypto_test;

import " // TODO: Convert string interpolation to concatenationcrypto";
import "stdlib/testing";

pub fn test_crypto_public_api(none) -&gt; void {
    let data: &#91;&#93;u8 = &#91;1, 2, 3&#93;;
    let result = crypto.sha256(data);  // ✅ Can access public functions
    // crypto.internal_helper();       // ❌ Cannot access private functions
    
    testing.assert_eq(result.len, 32);
}
```

## AI Editor Benefits

This organization provides significant benefits for AI editors:

### Smaller File Contexts

```asthra
// Instead of one large 2000-line file:
// src/monolithic.asthra (2000 lines) 

// Split into focused, smaller files:
// src/main.asthra (50 lines)
// src/types.asthra (200 lines)  
// src/handlers.asthra (300 lines)
// src/utils.asthra (150 lines)
// src/validation.asthra (100 lines)
```

### Clear Organizational Boundaries

- Each directory represents a clear functional boundary
- AI can focus on one package at a time
- Reduced cognitive load when working with specific functionality
- Easier to understand dependencies and relationships

### Predictable File Structure

```asthra
// AI can predict file organization:
project/
├── main.asthra           # Entry point
├── types.asthra          # Data structures
├── handlers.asthra       # Request handlers  
├── utils.asthra          # Utility functions
├── validation.asthra     # Input validation
└── config.asthra         # Configuration
```



## Package Import Resolution

The Ampu build system automatically discovers packages by directory structure:

```toml
# asthra.toml
&#91;package&#93;
name = "my-app"
version = "1.0.0"

# No need to explicitly list internal packages
# ampu automatically discovers:
# - src/ -&gt; package main
# - crypto/ -&gt; package crypto  
# - database/ -&gt; package database
```

### Import Resolution Rules

1\. **Relative imports:** `import "./crypto"` imports from subdirectory
2\. **Absolute imports:** `import "crypto"` imports from project root
3\. **Standard library:** `import "stdlib/string"` imports from standard library
4\. **External packages:** `import "github.com/user/package"` imports external dependency

## Package Function Access

Asthra supports package function access using dot notation:

```asthra
import "stdlib/string" as str;
import "stdlib/collections" as collections;
import "github.com/crypto/sha256" as crypto;

pub fn demonstrate_module_access(none) -&gt; void {
    let text: string = "  Hello, World!  ";
    
    let trimmed: string = str.trim(text);
    let uppercase: string = str.to_uppercase(trimmed);
    let length: usize = str.len(uppercase);
    
    let processed: string = str.trim(text)
        .to_lowercase()
        .replace("world", "asthra");
    
    let data: &#91;&#93;u8 = str.to_bytes(processed);
    let hash: &#91;&#93;u8 = crypto.hash(data);
    
    let words: &#91;&#93;string = str.split(processed, " ");
    let word_count: usize = collections.len(words);
}
```

## Standard Library Package Organization

**Yes, the Go-style package organization rules apply to the standard library as well.** The Asthra standard library follows the same directory-based package structure:

```
stdlib/
├── string/              # package string
│   ├── string.asthra   # Core string functions
│   ├── format.asthra   # String formatting
│   └── unicode.asthra  # Unicode operations
├── io/                  # package io
│   ├── io.asthra       # Core I/O functions
│   ├── reader.asthra   # Reader interface
│   └── writer.asthra   # Writer interface
├── collections/         # package collections
│   ├── list.asthra     # List implementation
│   ├── map.asthra      # Map implementation
│   └── set.asthra      # Set implementation
├── crypto/              # package crypto
│   ├── hash.asthra     # Hashing functions
│   ├── cipher.asthra   # Encryption functions
│   └── random.asthra   # Random number generation
└── datetime/            # package datetime
    ├── time.asthra     # Time operations
    ├── format.asthra   # Time formatting
    └── zone.asthra     # Timezone handling
```

### Standard Library Package Rules

1\. **Same Directory Rule**: All `.asthra` files in `stdlib/string/` must declare `package string;`
2\. **Public API**: Standard library functions marked with `pub` are available to user code
3\. **Internal Functions**: Private functions in stdlib packages are only accessible within the same package
4\. **Multi-file Implementation**: Large stdlib packages can be split across multiple files

### Example Standard Library Implementation

```asthra
// stdlib/string/string.asthra
package string;

pub fn trim(s: string) -&gt; string {
    return internal_trim_whitespace(s);
}

pub fn len(s: string) -&gt; usize {
    return internal_utf8_len(s);
}

pub fn internal_trim_whitespace(s: string) -&gt; string {
    // Private implementation - only accessible within string package
}

// stdlib/string/format.asthra
package string;  // Same package as string.asthra

pub fn format(template: string, args: &#91;&#93;Variant) -&gt; string {
    return internal_format_impl(template, args);
}

pub fn internal_format_impl(template: string, args: &#91;&#93;Variant) -&gt; string {
    // Can access internal_trim_whitespace from string.asthra
    let trimmed = internal_trim_whitespace(template);
    // Implementation details...
}

// stdlib/string/unicode.asthra
package string;  // Same package as other string files

pub fn to_uppercase(s: string) -&gt; string {
    // Can access all internal functions from the string package
    return internal_unicode_transform(s, UPPERCASE);
}

pub fn internal_unicode_transform(s: string, transform: TransformType) -&gt; string {
    // Private implementation
}
```

### Import Examples

```asthra
// Import entire standard library packages
import "stdlib/string";
import "stdlib/collections";
import "stdlib/crypto";

pub fn example(none) -&gt; void " + // Access public functions from stdlib packages
    let text = string.trim("  hello  ");
    let formatted = string.format("Hello { // TODO: Convert string interpolation to concatenation, &#91;text&#93;);
    let uppercase = string.to_uppercase(text);
    
    // Cannot access private functions
    // string.internal_trim_whitespace(text);  // ❌ Compile error
}

// Selective imports also work with stdlib
import "stdlib/string." + trim, len // TODO: Convert string interpolation to concatenation;
import "stdlib/crypto." + sha256, encrypt // TODO: Convert string interpolation to concatenation;

pub fn selective_example(none) -&gt; void {
    let text = "hello";
    let length = len(text);      // Direct access via selective import
    let trimmed = trim(text);    // Direct access via selective import
}
```

This ensures that the standard library follows the same organizational principles as user code, making it predictable and consistent for both human developers and AI code generation tools.

## Benefits

### Code Organization
- Related functionality grouped in same package across multiple files
- Maintainability: Easier to maintain and extend library modules
- AI Editor Friendly: Source can be split into smaller, focused files
- Consistent Rules: Same package organization rules apply everywhere

### Development Experience
- **Zero Configuration:** New projects can immediately use standard library features
- **Consistent Environment:** All Asthra installations provide the same interface
- **AI-Friendly:** Code generation tools can assume stdlib availability without setup
- **Predictable Structure:** Standard organization patterns for all packages 
