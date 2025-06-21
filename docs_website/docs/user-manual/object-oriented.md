# Object-Oriented Programming in Asthra

✅ **Status: FULLY IMPLEMENTED** - Complete object-oriented programming support with visibility control, implementation blocks, and instance methods.

This section provides a comprehensive guide to Asthra's object-oriented programming capabilities, implemented through three core keywords: `pub`, `impl`, and `self`.

## Overview

Asthra supports modern object-oriented programming patterns while maintaining its focus on performance, safety, and AI-friendly code generation. The object-oriented system provides:

- **Encapsulation**: Public/private visibility control for data hiding
- **Method Organization**: Clean separation of behavior through implementation blocks
- **Type Safety**: Compile-time checking of visibility rules and method calls
- **Zero Overhead**: Efficient compile-time method resolution with no runtime cost
- **Familiar Syntax**: Industry-standard method call syntax (`::` and `.`)

## Visibility Control with `pub`

### Default Visibility (Private)

By default, all declarations in Asthra are **private**:

```asthra
priv struct PrivateData {        // Private struct
    secret: i32;            // Private field
    internal_flag: bool;    // Private field
}

priv fn private_function(void) -&gt; void {  // Private function
    // Only accessible within this module
}
```

### Public Visibility

Use the `pub` keyword to make items accessible from other modules:

```asthra
pub struct PublicAPI {          // Public struct
    pub version: string;        // Public field
    pub enabled: bool;          // Public field
    internal_config: &#91;&#93;u8;      // Private field (default)
}

pub fn public_utility(void) -&gt; int {  // Public function
    return 42;
}
```

### Fine-Grained Field Visibility

Control visibility at the individual field level:

```asthra
pub struct UserAccount {
    pub username: string;       // Public: external access allowed
    pub email: string;          // Public: external access allowed
    pub created_at: i64;        // Public: readonly information
    password_hash: &#91;&#93;u8;        // Private: security-sensitive
    session_token: string;      // Private: internal state
    failed_login_count: i32;    // Private: implementation detail
}

impl UserAccount {
    pub fn new(username: string, email: string) -&gt; UserAccount {
        return UserAccount {
            username: username,
            email: email,
            created_at: get_current_timestamp(void),
            password_hash: &#91;void&#93;,
            session_token: "",
            failed_login_count: 0
        };
    }
    
    // Public method for controlled access to private data
    pub fn is_locked(self) -&gt; bool " + 
        return self.failed_login_count &gt;= 3;
     + "
    
    // Private method for internal operations
    priv fn increment_failed_logins(self) -&gt; UserAccount " + 
        return UserAccount " + 
            username: self.username,
            email: self.email,
            created_at: self.created_at,
            password_hash: self.password_hash,
            session_token: self.session_token,
            failed_login_count: self.failed_login_count + 1
         + ";
     + "
}
```

## Implementation Blocks with `impl`

### Basic Implementation Block Structure

Use `impl` to define methods for structs:

```asthra
pub struct Rectangle " + 
    width: f64;
    height: f64;
 + "

impl Rectangle " + 
    // Methods are defined here
 + "
```

### Associated Functions (Constructors)

Functions without a `self` parameter are **associated functions**:

```asthra
impl Rectangle " + 
    // Primary constructor
    pub fn new(width: f64, height: f64) -&gt; Rectangle {
        return Rectangle { width: width, height: height  + ";
    }
    
    // Alternative constructors
    pub fn square(size: f64) -&gt; Rectangle " + 
        return Rectangle::new(size, size);
     + "
    
    pub fn unit(void) -&gt; Rectangle " + 
        return Rectangle::new(1.0, 1.0);
     + "
    
    // Factory methods
    pub fn from_area_and_ratio(area: f64, ratio: f64) -&gt; Rectangle " + 
        let width: f64 = sqrt(area * ratio);
        let height: f64 = area / width;
        return Rectangle::new(width, height);
     + "
}

// Usage: Called with :: syntax
pub fn create_rectangles(void) -&gt; void " + 
    let rect1: Rectangle = Rectangle::new(10.0, 5.0);
    let rect2: Rectangle = Rectangle::square(7.0);
    let rect3: Rectangle = Rectangle::unit(void);
    let rect4: Rectangle = Rectangle::from_area_and_ratio(50.0, 2.0);
 + "
```

### Instance Methods

Functions with a `self` parameter are **instance methods**:

```asthra
impl Rectangle " + 
    // Getter methods
    pub fn area(self) -&gt; f64 " + 
        return self.width * self.height;
     + "
    
    pub fn perimeter(self) -&gt; f64 " + 
        return 2.0 * (self.width + self.height);
     + "
    
    pub fn diagonal(self) -&gt; f64 " + 
        return sqrt(self.width * self.width + self.height * self.height);
     + "
    
    // Predicate methods
    pub fn is_square(self) -&gt; bool " + 
        return self.width == self.height;
     + "
    
    pub fn is_portrait(self) -&gt; bool " + 
        return self.height &gt; self.width;
     + "
    
    pub fn is_landscape(self) -&gt; bool " + 
        return self.width &gt; self.height;
     + "
    
    // Transformation methods (return new instances)
    pub fn scale(self, factor: f64) -&gt; Rectangle " + 
        return Rectangle::new(self.width * factor, self.height * factor);
     + "
    
    pub fn rotate(self) -&gt; Rectangle " + 
        return Rectangle::new(self.height, self.width);
     + "
    
    pub fn resize(self, new_width: f64, new_height: f64) -&gt; Rectangle " + 
        return Rectangle::new(new_width, new_height);
     + 

// Usage: Called with . syntax on instances
pub fn use_rectangle_methods(void) -&gt; void " + 
    let rect: Rectangle = Rectangle::new(10.0, 5.0);
    
    // Simple method calls
    let area: f64 = rect.area(void);
    let perimeter: f64 = rect.perimeter(void);
    let is_square: bool = rect.is_square(void);
    
    // Method chaining
    let transformed: Rectangle = rect
        .scale(2.0)
        .rotate(void)
        .scale(0.5);
 + "
```

## Instance Methods with `self`

### The `self` Parameter

The `self` parameter represents the current instance and must be the first parameter in instance methods:

```asthra
struct Circle " + 
    radius: f64;
    center_x: f64;
    center_y: f64;
 + "

impl Circle " + 
    // ✅ Correct: self is first parameter
    pub fn area(self) -&gt; f64 " + 
        return 3.14159 * self.radius * self.radius;
     + "
    
    // ✅ Correct: self with additional parameters
    pub fn distance_to(self, other: Circle) -&gt; f64 " + 
        let dx: f64 = self.center_x - other.center_x;
        let dy: f64 = self.center_y - other.center_y;
        return sqrt(dx * dx + dy * dy);
     + "
    
    // ✅ Correct: self can access all fields
    pub fn circumference(self) -&gt; f64 " + 
        return 2.0 * 3.14159 * self.radius;
     + "
    
    // ❌ Error: self must be first parameter
    // fn invalid_method(radius: f64, self) -&gt; f64 " +  ...  + "
    
    // ❌ Error: cannot have multiple self parameters
    // fn invalid_method2(self, other_self: Circle) -&gt; f64 " +  ...  + 
```

### Accessing Fields Through `self`

Use `self` to access the instance's fields and call other methods:

```asthra
struct BankAccount " + 
    pub account_number: string;
    balance: f64;                    // Private field
    transaction_history: &#91;&#93;string;   // Private field
 + "

impl BankAccount " + 
    pub fn new(account_number: string, initial_balance: f64) -&gt; BankAccount {
        return BankAccount {
            account_number: account_number,
            balance: initial_balance,
            transaction_history: &#91;&#93;
         + ";
    }
    
    // Controlled access to private field
    pub fn get_balance(self) -&gt; f64 " + 
        return self.balance;
     + "
    
    // Method that modifies state (returns new instance)
    pub fn deposit(self, amount: f64) -&gt; Result&lt;BankAccount, string&gt; {if amount &lt;= 0.0 " + 
            return Result.Err("Deposit amount must be positive");
         + "
        
        return Result.Ok(BankAccount "DEPOSIT: " + amount)
         + ");
    }
    
    pub fn withdraw(self, amount: f64) -&gt; Result&lt;BankAccount, string&gt; {if amount &lt;= 0.0 {
            return Result.Err("Withdrawal amount must be positive");
         + "
        
        if amount &gt; self.balance "Insufficient funds");
        }
        
        return Result.Ok(BankAccount " + 
            account_number: self.account_number,
            balance: self.balance - amount,
            transaction_history: self.add_transaction("WITHDRAWAL: " + amount)
         + ");
    }
    
    // Private helper method
    pub fn add_transaction(self, transaction: string) -&gt; &#91;&#93;string " + 
        // Implementation would add transaction to history
        return self.transaction_history;
     + "
    
    // Method calling other methods
    pub fn transfer_to(self, target_account: string, amount: f64) -&gt; Result&lt;BankAccount, string&gt; "Insufficient funds for transfer");
        }
        
        return self.withdraw(amount);  // Chaining method calls
    }
}
```

## Method Call Syntax

### Associated Function Calls (`::`)

Use `::` syntax to call associated functions (no instance required):

```asthra
// Constructor calls
let point: Point = Point::new(3.0, 4.0);
let origin: Point = Point::origin();

// Factory methods
let config: Config = Config::from_file("settings.conf");
let default_config: Config = Config::default();

// Utility functions
let version: string = AppInfo::get_version();
let build_info: BuildInfo = AppInfo::get_build_info();
```

### Instance Method Calls (`.`)

Use `.` syntax to call instance methods (requires an instance):

```asthra
let circle: Circle = Circle::new(5.0, 0.0, 0.0);

// Simple method calls
let area: f64 = circle.area();
let circumference: f64 = circle.circumference();

// Method calls with parameters
let other_circle: Circle = Circle::new(3.0, 10.0, 0.0);
let distance: f64 = circle.distance_to(other_circle);

// Boolean methods
let is_unit_circle: bool = circle.is_unit_circle();
```

### Method Chaining

Chain method calls when methods return the same or compatible types:

```asthra
struct StringBuilder " + 
    content: string;
 + "

impl StringBuilder " + 
    pub fn new() -&gt; StringBuilder {
        return StringBuilder { content: ";
    }
    
    pub fn append(self, text: string) -&gt; StringBuilder " + 
        return StringBuilder " +  content: self.content + text  + ";
     + "
    
    pub fn append_line(self, text: string) -&gt; StringBuilder "\n" };
    }
    
    pub fn to_string(self) -&gt; string " + 
        return self.content;
     + "
}

// Method chaining example
pub fn build_message() -&gt; string {return StringBuilder::new()
        .append("Hello, ")
        .append("world!")
        .append_line("")
        .append("This is a message.")
        .to_string();
 + "
```

## Advanced Object-Oriented Patterns

### Builder Pattern

Implement the builder pattern for flexible object construction:

```asthra
pub struct HttpRequest " + 
    pub method: string;
    pub url: string;
    pub headers: &#91;&#93;string;
    pub body: string;
    timeout: i32;
    retries: i32;
 + "

impl HttpRequest "",
            timeout: 30,
            retries: 3
        };
    }
    
    pub fn with_header(self, header: string) -&gt; HttpRequest " + 
        return HttpRequest " + 
            method: self.method,
            url: self.url,
            headers: self.add_header(header),
            body: self.body,
            timeout: self.timeout,
            retries: self.retries
         + ";
     + "
    
    pub fn with_body(self, body: string) -&gt; HttpRequest " + 
        return HttpRequest {
            method: self.method,
            url: self.url,
            headers: self.headers,
            body: body,
            timeout: self.timeout,
            retries: self.retries
         + ";
    }
    
    pub fn with_timeout(self, timeout: i32) -&gt; HttpRequest " + 
        return HttpRequest " + 
            method: self.method,
            url: self.url,
            headers: self.headers,
            body: self.body,
            timeout: timeout,
            retries: self.retries
         + ";
     + "
    
    pub fn with_retries(self, retries: i32) -&gt; HttpRequest " + 
        return HttpRequest {
            method: self.method,
            url: self.url,
            headers: self.headers,
            body: self.body,
            timeout: self.timeout,
            retries: retries
         + ";
    }
    
    pub fn add_header(self, header: string) -&gt; &#91;&#93;string " + 
        // Implementation would add header to array
        return self.headers;
     + "
}

// Usage with fluent interface
pub fn create_request() -&gt; HttpRequest {return HttpRequest::new("POST", "https://api.example.com/users")
        .with_header("Content-Type: application/json")
        .with_header("Authorization: Bearer token123")
        .with_body(\"name\": \"John Doe\")
        .with_timeout(60)
        .with_retries(5);
 + "
```

### State Machine Pattern

Model state changes through method calls:

```asthra
pub struct ConnectionState " + 
    pub status: string;
    pub address: string;
    pub port: i32;
    last_error: string;
    retry_count: i32;
 + "

impl ConnectionState " + 
    pub fn new(address: string, port: i32) -&gt; ConnectionState {
        return ConnectionState {
            status: "disconnected",
            address: address,
            port: port,
            last_error: "",
            retry_count: 0
         + ";
    }
    
    pub fn connect(self) -&gt; Result&lt;ConnectionState, string&gt; {if self.status == "connected" " + 
            return Result.Err("Already connected");
         + "
        
        // Simulate connection logic
        if self.address == "" " + 
            return Result.Err("Invalid address");
         + "
        
        return Result.Ok(ConnectionState "connected",
            address: self.address,
            port: self.port,
            last_error: "",
            retry_count: 0
         + ");
    }
    
    pub fn disconnect(self) -&gt; ConnectionState " + 
        return ConnectionState {
            status: "disconnected",
            address: self.address,
            port: self.port,
            last_error: "",
            retry_count: self.retry_count
         + ";
    }
    
    pub fn retry_connection(self) -&gt; Result&lt;ConnectionState, string&gt; "Maximum retries exceeded");
        }
        
        let new_state: ConnectionState = ConnectionState " + 
            status: self.status,
            address: self.address,
            port: self.port,
            last_error: self.last_error,
            retry_count: self.retry_count + 1
         + ";
        
        return new_state.connect();
    }
    
    pub fn is_connected(self) -&gt; bool " + 
        return self.status == "connected";
     + "
}
```

### Factory Pattern

Create families of related objects:

```asthra
pub struct DatabaseConnection " + 
    pub driver: string;
    pub host: string;
    pub port: i32;
    pub database: string;
 + "

impl DatabaseConnection " + 
    // Generic constructor
    pub fn new(driver: string, host: string, port: i32, database: string) -&gt; DatabaseConnection {
        return DatabaseConnection {
            driver: driver,
            host: host,
            port: port,
            database: database
         + ";
    }
    
    // Specific factory methods
    pub fn postgres(host: string, database: string) -&gt; DatabaseConnection "postgresql", host, 5432, database);
    }
    
    pub fn mysql(host: string, database: string) -&gt; DatabaseConnection " + 
        return DatabaseConnection::new("mysql", host, 3306, database);
     + "
    
    pub fn sqlite(file_path: string) -&gt; DatabaseConnection "sqlite", file_path, 0, "");
    }
    
    pub fn get_connection_string(self) -&gt; string " + 
        if self.driver == "sqlite" " + 
            return "sqlite://" + self.host;
         + "
        
        return self.driver + "://" + self.host + ":" + self.port + "/" + self.database;
     + "
}

// Usage
pub fn create_database_connections() -&gt; void {let pg_conn: DatabaseConnection = DatabaseConnection::postgres("localhost", "myapp");
    let mysql_conn: DatabaseConnection = DatabaseConnection::mysql("db.example.com", "production");
    let sqlite_conn: DatabaseConnection = DatabaseConnection::sqlite("/path/to/database.db");
 + "
```

## Error Handling in Object-Oriented Code

Combine object-oriented patterns with Asthra's Result type for robust error handling:

```asthra
pub struct FileManager " + 
    pub current_path: string;
    is_initialized: bool;
 + "

impl FileManager "" " + 
            return Result.Err("Base path cannot be empty");
         + "
        
        return Result.Ok(FileManager " + 
            current_path: base_path,
            is_initialized: true
         + ");
    }
    
    pub fn read_file(self, filename: string) -&gt; Result&lt;string, string&gt; "FileManager not initialized");
        }
        
        if filename == "" " + 
            return Result.Err("Filename cannot be empty");
         + "
        
        let full_path: string = self.current_path + "/" + filename;
        
        // Simulate file reading
        return Result.Ok("File contents from " + full_path);
    }
    
    pub fn write_file(self, filename: string, content: string) -&gt; Result&lt;FileManager, string&gt; {if !self.is_initialized " + 
            return Result.Err("FileManager not initialized");
         + "
        
        if filename == "" " + 
            return Result.Err("Filename cannot be empty");
         + "
        
        // Simulate file writing
        log("Writing to file: " + filename);
        
        return Result.Ok(self);  // Return same instance for chaining
     + "
    
    pub fn change_directory(self, new_path: string) -&gt; Result&lt;FileManager, string&gt; {if new_path == "" " + 
            return Result.Err("Path cannot be empty");
         + "
        
        return Result.Ok(FileManager " + 
            current_path: new_path,
            is_initialized: self.is_initialized
         + ");
     + "
}

// Usage with comprehensive error handling
pub fn file_operations() -&gt; Result&lt;void, string&gt; "/home/user") " + 
        Result.Ok(fm) =&gt; fm,
        Result.Err(error) =&gt; return Result.Err("Failed to create FileManager: " + error)
     + ";
    
    let content: string = match file_manager.read_file("config.txt") " + 
        Result.Ok(data) =&gt; data,
        Result.Err(error) =&gt; return Result.Err("Failed to read file: " + error)
     + ";
    
    let updated_manager: FileManager = match file_manager.write_file("output.txt", "New content") " + 
        Result.Ok(fm) =&gt; fm,
        Result.Err(error) =&gt; return Result.Err("Failed to write file: " + error)
     + ";
    
    return Result.Ok(void);
}
```

## Compilation and Type Safety

### Visibility Enforcement

Asthra enforces visibility rules at compile time:

```asthra
pub struct SecureData " + 
    pub id: i32;
    secret_key: &#91;&#93;u8;    // Private field
 + "

impl SecureData " + 
    pub fn new(id: i32) -&gt; SecureData {
        return SecureData { id: id, secret_key: &#91;&#93;  + ";
    }
    
    pub fn get_id(self) -&gt; i32 " + 
        return self.id;  // ✅ Can access all fields within impl
     + "
    
    pub fn get_secret_key(self) -&gt; &#91;&#93;u8 " +   // Private method
        return self.secret_key;
     + "
}

pub fn test_visibility() -&gt; void {let data: SecureData = SecureData::new(123);
    
    let id: i32 = data.id;           // ✅ Public field access allowed
    let id2: i32 = data.get_id();    // ✅ Public method call allowed
    
    // let secret: &#91;&#93;u8 = data.secret_key;     // ❌ Compile error: private field
    // let secret2: &#91;&#93;u8 = data.get_secret_key(); // ❌ Compile error: private method
 + "
```

### Method Resolution

The compiler resolves method calls based on the call syntax:

```asthra
struct Calculator " + 
    value: f64;
 + "

impl Calculator " + 
    pub fn new(initial_value: f64) -&gt; Calculator {  // Associated function
        return Calculator { value: initial_value  + ";
    }
    
    pub fn add(self, other: f64) -&gt; Calculator " +     // Instance method
        return Calculator " +  value: self.value + other  + ";
     + "
}

pub fn method_resolution_examples() -&gt; void {// ✅ Correct: Associated function called with ::
    let calc: Calculator = Calculator::new(10.0);
    
    // ✅ Correct: Instance method called with .
    let result: Calculator = calc.add(5.0);
    
    // ❌ Compile error: Cannot call associated function on instance
    // let calc2: Calculator = calc::new(20.0);
    
    // ❌ Compile error: Cannot call instance method on type
    // let result2: Calculator = Calculator.add(5.0);
 + "
```

### Self Parameter Validation

The compiler validates self parameter usage:

```asthra
impl Calculator " + 
    // ✅ Valid: Associated function without self
    pub fn create_default() -&gt; Calculator {...  + "
    
    // ✅ Valid: Instance method with self first
    pub fn multiply(self, factor: f64) -&gt; Calculator " +  ...  + "
    
    // ✅ Valid: Instance method with self and additional parameters
    pub fn divide_by(self, divisor: f64) -&gt; Result&lt;Calculator, string&gt; {...  + "
    
    // ❌ Compile error: Associated function cannot have self
    // fn invalid_constructor(self) -&gt; Calculator " +  ...  + "
    
    // ❌ Compile error: self must be first parameter
    // fn invalid_method(factor: f64, self) -&gt; Calculator " +  ...  + "
    
    // ❌ Compile error: multiple self parameters not allowed
    // fn invalid_method2(self, other_self: Calculator) -&gt; Calculator " +  ...  + 
```

## Best Practices

### 1. Use Clear Naming Conventions

```asthra
impl HttpClient " + 
    // ✅ Good: Descriptive method names
    pub fn create_connection() -&gt; HttpClient { ...  + "
    pub fn send_request(self, request: HttpRequest) -&gt; Result&lt;HttpResponse, string&gt; {...  + "
    pub fn close_connection(self) -&gt; void " +  ...  + "
    
    // ❌ Avoid: Unclear or abbreviated names
    // fn conn() -&gt; HttpClient {...  + "
    // fn send(self, req: HttpRequest) -&gt; Result&lt;HttpResponse, string&gt; {...  + "
}
```

### 2. Group Related Functionality

```asthra
impl BankAccount " + 
    // Constructors first
    pub fn new(account_number: string) -&gt; BankAccount " +  ...  + "
    pub fn from_existing_data(data: AccountData) -&gt; BankAccount " +  ...  + "
    
    // Basic getters
    pub fn get_balance(self) -&gt; f64 " +  ...  + "
    pub fn get_account_number(self) -&gt; string " +  ...  + "
    
    // Core operations
    pub fn deposit(self, amount: f64) -&gt; Result&lt;BankAccount, string&gt; {...  + "
    pub fn withdraw(self, amount: f64) -&gt; Result&lt;BankAccount, string&gt; {...  + "
    
    // Utility methods
    pub fn is_active(self) -&gt; bool " +  ...  + "
    pub fn can_withdraw(self, amount: f64) -&gt; bool " +  ...  + 
```

### 3. Use Appropriate Visibility Levels

```asthra
pub struct ApiClient " + 
    pub base_url: string;        // Public: users may need to inspect
    timeout: i32;                // Private: implementation detail
    auth_token: string;          // Private: security-sensitive
 + "

impl ApiClient " + 
    // Public API
    pub fn new(base_url: string) -&gt; ApiClient " +  ...  + "
    pub fn get_timeout(self) -&gt; i32 " +  ...  + "
    pub fn set_timeout(self, timeout: i32) -&gt; ApiClient " +  ...  + "
    
    // Private helpers
    pub fn build_headers(self) -&gt; &#91;&#93;string " +  ...  + "
    pub fn handle_auth(self) -&gt; bool " +  ...  + 
```

### 4. Provide Comprehensive Error Handling

```asthra
impl DatabaseConnection "" " + 
            return Result.Err("Host cannot be empty");
         + "
        
        if self.port &lt;= 0 "Port must be positive");
        }
        
        // Connection logic...
        return Result.Ok(connected_state);
    }
    
    pub fn execute_query(self, query: string) -&gt; Result&lt;QueryResult, string&gt; {if !self.is_connected() {
            return Result.Err("Not connected to database");
         + "
        
        if query == "" {
            return Result.Err("Query cannot be empty");
        }
        
        // Execute logic...
        return Result.Ok(result);
    }
}
```

## Summary

Asthra's object-oriented programming system provides:

- **Complete Encapsulation**: Public/private visibility at struct, field, and method levels
- **Clean Method Organization**: Implementation blocks separate behavior from data structure
- **Type-Safe Method Dispatch**: Compile-time resolution with zero runtime overhead
- **Familiar Syntax**: Industry-standard `::` and `.` method call syntax
- **Error Integration**: Seamless combination with Asthra's Result type system
- **AI-Friendly Design**: Unambiguous grammar and predictable behavior for code generation

These features enable developers to write maintainable, secure, and efficient object-oriented code while maintaining Asthra's core principles of performance and safety.

For practical examples and patterns, see the comprehensive test suite in &#91;test_pub_impl_self_comprehensive.asthra&#93;(../../test_pub_impl_self_comprehensive.asthra) and the detailed implementation guide in &#91;docs/examples/pub_impl_self_guide.md&#93;(../examples/pub_impl_self_guide.md). 
