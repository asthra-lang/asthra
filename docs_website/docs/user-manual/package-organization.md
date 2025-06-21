# Package Organization

**Last Updated**: 2025-01-07  
**Target Audience**: Developers working with multi-file Asthra projects

## Overview

Asthra uses a **Go-style package organization system** that provides clear project structure, excellent AI-assisted development support, and straightforward dependency management. This guide covers everything you need to know about organizing your Asthra projects effectively.

### What are Packages in Asthra?

A **package** in Asthra is a collection of related source files that:
- All reside in the same directory
- Share the same package declaration
- Can access each other's private functions and types
- Present a unified public interface to other packages

### Why Go-Style Organization?

Asthra adopted Go's proven package model because it:
- **Eliminates ambiguity** for AI code generation (no guessing about module boundaries)
- **Simplifies mental models** (one directory = one package, always)
- **Scales naturally** from single-file projects to large codebases
- **Provides clear visibility rules** (public/private at package level)
- **Enables gradual refactoring** (split large files without changing public APIs)

### Benefits for AI-Assisted Development

The package system specifically optimizes for AI code generation by:
- **Deterministic imports**: No complex module resolution algorithms
- **Clear boundaries**: AI knows exactly what's accessible where
- **Consistent patterns**: Same organizational rules across all projects
- **Predictable visibility**: Simple pub/priv rules at package level

---

## Directory-Based Packages

### The Golden Rule

**One Directory = One Package**

Every `.asthra` file in a directory **must** declare the same package name. This rule eliminates ambiguity and makes project structure immediately clear.

### Basic Project Structure

#### Single Package Project
```
calculator/
├── Asthra.toml
└── src/
    ├── main.asthra        # package main
    ├── operations.asthra  # package main
    └── utils.asthra       # package main
```

#### Multi-Package Project
```
web-service/
├── Asthra.toml
├── src/
│   └── main.asthra      # package main
├── handlers/
│   ├── user.asthra      # package handlers
│   ├── auth.asthra      # package handlers
│   └── api.asthra       # package handlers
├── models/
│   ├── user.asthra      # package models
│   └── session.asthra   # package models
├── database/
│   ├── connection.asthra # package database
│   └── queries.asthra   # package database
└── utils/
    ├── logging.asthra   # package utils
    └── config.asthra    # package utils
```

### Package Declaration Syntax

Every `.asthra` file **must** start with a package declaration:

```asthra
// Required: First line of every .asthra file
package package_name;

// Optional: Import declarations
import "other_package";
import "stdlib/collections";

// Rest of file: Types, functions, etc.
pub fn example_function() -&gt; void {
    // Implementation
}
```

### Multi-File Package Implementation

Files in the same package work together seamlessly:

```asthra
// models/user.asthra
package models;

pub struct User {
    pub id: i32,
    pub name: string,
    email: string,        // Private field
}

pub fn create_user(name: string, email: string) -&gt; User {
    return User {
        id: generate_id(),  // Can call private functions from same package
        name: name,
        email: email,
    };
}

// Private function - accessible within models package only
fn generate_id() -&gt; i32 {
    // Implementation
    return 42;
}
```

```asthra
// models/session.asthra
package models;  // Must match other files in models/ directory

pub struct Session {
    pub user_id: i32,
    pub token: string,
    expires_at: i64,     // Private field
}

pub fn create_session(user: User) -&gt; Session {
    // Can access private fields and functions from same package
    let user_id: i32 = user.id;
    let session_id: i32 = generate_id();  // ✅ Can call private function
    
    return Session {
        user_id: user_id,
        token: "session_" + string(session_id),
        expires_at: current_timestamp() + 3600,
    };
}
```

---

## Project Structure Patterns

### Small Projects (1-3 packages)

**Single Package Pattern**:
```
simple-tool/
├── Asthra.toml
└── src/
    ├── main.asthra      # Entry point
    ├── logic.asthra     # Core logic
    └── utils.asthra     # Helper functions
```

Example package declaration:
```asthra
// All files in src/ use: package main;
package main;

pub fn main() -&gt; i32 {
    let result: bool = process_input();
    return if result { 0 } else { 1 };
}
```

### Medium Projects (3-8 packages)

**Domain-Driven Pattern**:
```
inventory-system/
├── Asthra.toml
├── src/
│   └── main.asthra      # package main
├── models/              # package models - Data structures
│   ├── product.asthra
│   ├── order.asthra
│   └── inventory.asthra
├── services/            # package services - Business logic
│   ├── inventory.asthra
│   ├── ordering.asthra
│   └── reporting.asthra
├── storage/             # package storage - Data persistence
│   ├── database.asthra
│   └── cache.asthra
└── utils/               # package utils - Shared utilities
    ├── logging.asthra
    └── config.asthra
```

### Large Projects (10+ packages)

**Layered Architecture Pattern**:
```
enterprise-app/
├── Asthra.toml
├── src/
│   └── main.asthra           # package main
├── api/                      # package api
│   ├── handlers/
│   │   ├── user.asthra       # package handlers
│   │   └── product.asthra    # package handlers
│   └── middleware/
│       ├── auth.asthra       # package middleware
│       └── logging.asthra    # package middleware
├── domain/
│   ├── user/
│   │   ├── model.asthra      # package user
│   │   └── service.asthra    # package user
│   └── product/
│       ├── model.asthra      # package product
│       └── service.asthra    # package product
├── infrastructure/
│   ├── database/
│   │   ├── connection.asthra # package database
│   │   └── migration.asthra  # package database
│   └── cache/
│       └── redis.asthra      # package cache
└── shared/
    ├── types/
    │   └── common.asthra     # package types
    └── utils/
        └── helpers.asthra    # package utils
```

---

## Cross-Package Communication

### Import Declarations

Access other packages using import statements:

```asthra
// src/main.asthra
package main;

import "models";     // Import local package
import "services";   // Import local package
import "stdlib/io";  // Import standard library

pub fn main() -&gt; i32 {
    // Use imported packages
    let user: models.User = models.create_user("Alice", "alice@example.com");
    let result: bool = services.process_user(user);
    
    io.println("Processing complete");
    return if result { 0 } else { 1 };
}
```

### Import Aliases (v1.22+)

Resolve naming conflicts with import aliases:

```asthra
// When you have multiple packages with similar names
package main;

import "graphics/math" as GraphicsMath;
import "physics/math" as PhysicsMath;
import "collections/vector" as Vec;

pub fn calculate_physics() -&gt; void {
    let distance: f64 = GraphicsMath.calculate_distance(1.0, 2.0);
    let velocity: f64 = PhysicsMath.calculate_velocity(10.0, 5.0);
    let my_vector: Vec.Vector<i32> = Vec.create_vector<i32>();
}
```

### Public/Private Visibility

Asthra uses simple visibility rules:

```asthra
// models/user.asthra
package models;

// Public struct - accessible from other packages
pub struct User {
    pub id: i32,        // Public field
    pub name: string,   // Public field
    email: string,      // Private field (package-only)
}

// Public function - accessible from other packages
pub fn get_user_id(user: User) -&gt; i32 {
    return user.id;
}

// Private function - accessible within models package only
fn validate_email(email: string) -&gt; bool {
    // Implementation
    return true;
}
```

```asthra
// services/user.asthra
package services;

import "models";

pub fn process_user(user: models.User) -&gt; bool {
    let user_id: i32 = models.get_user_id(user);  // ✅ Can access public function
    // user.email;                                 // ❌ Cannot access private field
    // models.validate_email("test");              // ❌ Cannot access private function
    
    return user_id &gt; 0;
}
```

### Function and Type Access

Access patterns across packages:

```asthra
// database/connection.asthra
package database;

pub struct Connection {
    pub host: string,
    port: i32,           // Private field
}

pub fn create_connection(host: string, port: i32) -&gt; Connection {
    return Connection { host: host, port: port };
}

pub fn close_connection(conn: Connection) -&gt; void {
    // Implementation
}
```

```asthra
// services/data.asthra
package services;

import "database";

pub fn setup_database() -&gt; database.Connection {
    // Using public constructor function
    let conn: database.Connection = database.create_connection("localhost", 5432);
    
    // Can access public fields
    let host: string = conn.host;
    
    // Cannot access private fields
    // let port: i32 = conn.port;  // ❌ Compilation error
    
    return conn;
}
```

---

## Testing Organization

### Test File Convention

Asthra follows Go's `_test.asthra` naming convention with two testing approaches:

#### Same-Package Testing (White-Box)
Test files in the same package can access private functions:

```asthra
// models/user_test.asthra
package models;  // Same package as models/user.asthra

import "stdlib/testing";

pub fn test_create_user() -&gt; void {
    let user: User = create_user("Alice", "alice@example.com");
    
    // Can access public fields
    testing.assert_eq(user.name, "Alice");
    
    // Can access private functions (same package)
    let id: i32 = generate_id();  // ✅ Accessible
    testing.assert(id &gt; 0);
}

pub fn test_validate_email() -&gt; void {
    // Can test private functions
    let valid: bool = validate_email("test@example.com");  // ✅ Accessible
    testing.assert(valid);
}
```

#### Separate-Package Testing (Black-Box)
Test files in different packages test only public interfaces:

```asthra
// tests/models_test.asthra
package models_test;  // Different package

import "models";
import "stdlib/testing";

pub fn test_user_public_api() -&gt; void {
    let user: models.User = models.create_user("Bob", "bob@example.com");
    
    // Can only access public interface
    let id: i32 = models.get_user_id(user);  // ✅ Public function
    testing.assert(id &gt; 0);
    
    // Cannot access private functions
    // models.validate_email("test");        // ❌ Not accessible
    // let id2: i32 = models.generate_id();  // ❌ Not accessible
}
```

### Test Structure Best Practices

Organize tests to match your package structure:

```
project/
├── models/
│   ├── user.asthra
│   ├── product.asthra
│   ├── user_test.asthra      # Same-package tests
│   └── product_test.asthra   # Same-package tests
├── services/
│   ├── inventory.asthra
│   └── inventory_test.asthra # Same-package tests
└── tests/
    ├── integration_test.asthra  # Cross-package integration tests
    ├── models_test.asthra       # Black-box model tests
    └── services_test.asthra     # Black-box service tests
```

---

## Best Practices

### Package Naming Conventions

Follow these naming guidelines:

```asthra
// ✅ GOOD: Clear, descriptive names
package user_management;
package database_connection;
package crypto_utils;
package web_handlers;

// ✅ GOOD: Short names for well-known concepts
package models;
package services;
package utils;
package api;

// ❌ AVOID: Vague or overly generic names
package stuff;
package helpers;
package common;
package misc;

// ❌ AVOID: Names that don't match directory
// In directory: user_auth/
package authentication;  // Should be: package user_auth;
```

### Dependency Management

Organize packages to minimize circular dependencies:

```asthra
// ✅ GOOD: Clear dependency hierarchy
// main -&gt; services -&gt; models -&gt; utils
// api -&gt; handlers -&gt; services -&gt; models

// models/user.asthra (no dependencies on services)
package models;

pub struct User {
    pub id: i32,
    pub name: string,
}

// services/user.asthra (depends on models)
package services;

import "models";

pub fn process_user(user: models.User) -&gt; bool {
    return user.id &gt; 0;
}

// api/handlers.asthra (depends on services)
package handlers;

import "services";
import "models";

pub fn handle_user_request() -&gt; models.User {
    // Implementation
}
```

### Circular Dependency Avoidance

Prevent circular dependencies through careful design:

```asthra
// ❌ AVOID: Circular dependency
// Package A imports B, and B imports A

// Better approach: Extract shared types to a separate package
// shared_types -&gt; models -&gt; services -&gt; handlers
```

### Performance Considerations

Package organization affects compilation and runtime performance:

```asthra
// ✅ GOOD: Group related functionality
package crypto_primitives;  // hash.asthra, encrypt.asthra, keys.asthra

// ✅ GOOD: Separate concerns for selective compilation
package crypto_hash;        // Only hash functions
package crypto_encrypt;     // Only encryption functions

// ❌ AVOID: Monolithic packages that force unnecessary compilation
package everything;         // crypto, database, web, utils all in one
```

---

## Migration from Modules

### Converting Existing Code

If you have old module-based code, follow this migration pattern:

#### Before (Module System)
```
old-project/
├── src/
│   ├── main.asthra
│   └── modules/
│       ├── user_module.asthra
│       └── auth_module.asthra
```

#### After (Package System)
```
new-project/
├── src/
│   └── main.asthra      # package main
├── user/
│   └── user.asthra      # package user
└── auth/
    └── auth.asthra      # package auth
```

### Common Migration Patterns

1\. **Module to Package**: Convert each module directory to a package directory
2\. **Update Declarations**: Change `module` declarations to `package` declarations  
3\. **Fix Imports**: Update import paths to use package names
4\. **Visibility Updates**: Review and update pub/priv declarations

### Migration Example

```asthra
// OLD: user_module.asthra
module user_management;

export fn create_user() -&gt; User {
    // Implementation
}

// NEW: user/user.asthra
package user;

pub fn create_user() -&gt; User {
    // Implementation
}
```

```asthra
// OLD: main.asthra
import user_management;

fn main() -&gt; i32 {
    let user = user_management.create_user();
}

// NEW: main.asthra
package main;

import "user";

pub fn main() -&gt; i32 {
    let user: user.User = user.create_user();
}
```

### Automated Migration Tools

Use these commands to help with migration:

```bash
# Find all module declarations
grep -r "module " src/

# Replace module with package (review before applying)
find src/ -name "*.asthra" -exec sed -i 's/module /package /g' {} \;

# Update import statements (manual review recommended)
find src/ -name "*.asthra" -exec sed -i 's/import \(&#91;^"&#93;*\);/import "\1";/g' {} \;
```

---

## Real-World Examples

### Example 1: CLI Tool
```
asthra-deploy/
├── Asthra.toml
├── src/
│   └── main.asthra      # Entry point and CLI parsing
├── config/
│   ├── loader.asthra    # Configuration loading
│   └── validator.asthra # Configuration validation
├── deploy/
│   ├── kubernetes.asthra # Kubernetes deployment
│   ├── docker.asthra    # Docker deployment
│   └── aws.asthra       # AWS deployment
└── utils/
    ├── logging.asthra   # Logging utilities
    └── filesystem.asthra # File operations
```

```asthra
// src/main.asthra
package main;

import "config";
import "deploy";
import "utils";

pub fn main() -&gt; i32 {
    let cfg: config.Config = config.load_from_file("deploy.toml");
    
    match deploy.execute_deployment(cfg) {
        Result.Ok(_) =&gt; {
            utils.log_success("Deployment completed");
            return 0;
        },
        Result.Err(error) =&gt; {
            utils.log_error("Deployment failed: " + error);
            return 1;
        },
    }
}
```

### Example 2: Web Service
```
api-server/
├── Asthra.toml
├── src/
│   └── main.asthra           # Server setup and routing
├── handlers/
│   ├── user.asthra           # User API endpoints
│   ├── product.asthra        # Product API endpoints
│   └── auth.asthra           # Authentication endpoints
├── models/
│   ├── user.asthra           # User data structures
│   ├── product.asthra        # Product data structures
│   └── response.asthra       # API response types
├── services/
│   ├── user_service.asthra   # User business logic
│   ├── product_service.asthra # Product business logic
│   └── auth_service.asthra   # Authentication logic
├── database/
│   ├── connection.asthra     # Database connection management
│   ├── user_queries.asthra   # User database operations
│   └── product_queries.asthra # Product database operations
└── middleware/
    ├── cors.asthra           # CORS handling
    ├── logging.asthra        # Request logging
    └── validation.asthra     # Input validation
```

```asthra
// handlers/user.asthra
package handlers;

import "models";
import "services";

pub fn handle_get_user(user_id: i32) -&gt; models.ApiResponse {
    match services.get_user_by_id(user_id) {
        Result.Ok(user) =&gt; {
            return models.ApiResponse {
                status: 200,
                data: user,
                error: none,
            };
        },
        Result.Err(error) =&gt; {
            return models.ApiResponse {
                status: 404,
                data: none,
                error: error,
            };
        },
    }
}
```

### Example 3: Library Package Design
```
asthra-crypto/
├── Asthra.toml
├── src/
│   └── lib.asthra           # Public library interface
├── hash/
│   ├── sha256.asthra        # SHA-256 implementation
│   ├── sha512.asthra        # SHA-512 implementation
│   └── blake3.asthra        # BLAKE3 implementation
├── encrypt/
│   ├── aes.asthra           # AES encryption
│   ├── chacha20.asthra      # ChaCha20 encryption
│   └── stream.asthra        # Stream cipher interface
├── keys/
│   ├── generation.asthra    # Key generation
│   ├── derivation.asthra    # Key derivation
│   └── exchange.asthra      # Key exchange
└── utils/
    ├── random.asthra        # Secure random number generation
    └── timing.asthra        # Constant-time operations
```

```asthra
// src/lib.asthra - Public library interface
package crypto;

// Re-export commonly used types and functions
pub use hash.sha256 as sha256;
pub use hash.sha512 as sha512;
pub use encrypt.aes as aes;
pub use keys.generation as keygen;

// Convenience functions
pub fn hash_password(password: string, salt: &#91;&#93;u8) -&gt; &#91;&#93;u8 {
    return hash.sha256(password + string(salt));
}

pub fn encrypt_message(message: &#91;&#93;u8, key: &#91;&#93;u8) -&gt; Result&lt;&#91;&#93;u8, string&gt; {
    return encrypt.aes.encrypt(message, key);
}
```

---

## Advanced Package Patterns

### Package Composition

Build complex functionality by composing packages:

```asthra
// High-level package that coordinates others
package orchestrator;

import "database";
import "cache";
import "notifications";

pub fn process_user_action(user_id: i32, action: string) -&gt; Result&lt;void, string&gt; {
    // Coordinate multiple packages
    let user_data: Result&lt;database.User, string&gt; = database.get_user(user_id);
    let cache_result: Result&lt;void, string&gt; = cache.invalidate_user(user_id);
    let notify_result: Result&lt;void, string&gt; = notifications.send_update(user_id, action);
    
    // Handle coordination logic
    return Result.Ok(void);
}
```

### Plugin Architecture

Design packages for extensibility:

```asthra
// Core plugin interface
package plugins;

pub trait DataProcessor {
    fn process(data: &#91;&#93;u8) -&gt; Result&lt;&#91;&#93;u8, string&gt;;
    fn get_name() -&gt; string;
}

// JSON processor plugin
package json_processor;

import "plugins";

pub struct JsonProcessor {}

impl plugins.DataProcessor for JsonProcessor {
    fn process(data: &#91;&#93;u8) -&gt; Result&lt;&#91;&#93;u8, string&gt; {
        // JSON processing logic
        return Result.Ok(data);
    }
    
    fn get_name() -&gt; string {
        return "JSON Processor";
    }
}
```

### Package-Based Configuration

Organize configuration by package boundaries:

```asthra
// config/database.asthra
package database_config;

pub struct DatabaseConfig {
    pub host: string,
    pub port: i32,
    pub database_name: string,
    username: string,        // Private - loaded from environment
    password: string,        // Private - loaded from environment
}

pub fn load_database_config() -&gt; DatabaseConfig {
    // Load configuration with private credentials
}

// config/api.asthra  
package api_config;

pub struct ApiConfig {
    pub port: i32,
    pub max_connections: i32,
    pub timeout_seconds: i32,
}

pub fn load_api_config() -&gt; ApiConfig {
    // Load API configuration
}
```

---

## Troubleshooting Common Issues

### Package Declaration Mismatches

**Problem**: Files in the same directory have different package declarations
```asthra
// models/user.asthra
package models;      // ✅ Correct

// models/product.asthra  
package products;    // ❌ Should be 'models'
```

**Solution**: Ensure all files in a directory use the same package name
```asthra
// models/product.asthra
package models;      // ✅ Fixed
```

### Import Path Errors

**Problem**: Incorrect import paths
```asthra
import models;           // ❌ Missing quotes
import "models/user";    // ❌ Too specific (imports entire package)
import "../models";      // ❌ Relative paths not allowed
```

**Solution**: Use correct import syntax
```asthra
import "models";         // ✅ Correct package import
```

### Circular Dependencies

**Problem**: Packages importing each other
```asthra
// Package A
import "B";

// Package B  
import "A";              // ❌ Circular dependency
```

**Solution**: Extract shared types or redesign package boundaries
```asthra
// Create shared package for common types
// shared -&gt; A, B (both import shared, not each other)
```

### Private Access Violations

**Problem**: Trying to access private members from other packages
```asthra
// other_package.asthra
let user: models.User = models.User { 
    id: 1,
    name: "Alice",
    email: "private_field"   // ❌ Cannot access private field
};
```

**Solution**: Use public constructor functions
```asthra
// Use public constructor
let user: models.User = models.create_user("Alice", "alice@example.com");
```

---

## See Also

- **&#91;Language Fundamentals&#93;(language-fundamentals.md)** - Basic package declaration syntax
- **&#91;Building Projects&#93;(building-projects.md)** - Build system integration with packages  
- **&#91;Getting Started&#93;(getting-started.md)** - Setting up your first package-based project
- **&#91;Annotations&#93;(annotations.md)** - Package-scope annotations and semantic tags
- **&#91;FFI Interop&#93;(ffi-interop.md)** - Organizing C interoperability across packages

---

**Next Steps**: Now that you understand package organization, learn about &#91;Building Projects&#93;(building-projects.md) to see how the build system integrates with your package structure, or explore &#91;Language Fundamentals&#93;(language-fundamentals.md) for detailed syntax reference.

---

**Next Steps**: Now that you understand package organization, learn about &#91;Building Projects&#93;(building-projects.md) to see how the build system integrates with your package structure, or explore &#91;Language Fundamentals&#93;(language-fundamentals.md) for detailed syntax reference. 
