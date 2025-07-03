@edge_cases
Feature: Type checking and validation
  As a language designer
  I want to ensure type inference and checking work correctly
  So that programs have strong type safety and clear error messages

  Background:
    Given the Asthra compiler is available

  # Test 1: Type inference in variable declarations
  Scenario: All variable declarations require explicit type annotations
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x = 42;  // Missing type annotation - should fail
          return x;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type annotation"

  Scenario: Correct variable declaration with explicit type
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 42;           // Correct: explicit type
          let y: string = "hello";   // Correct: explicit type
          let z: bool = true;        // Correct: explicit type
          
          if z {
              return x;
          }
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Type annotations required even with complex initializers
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let nums: []i32 = [1, 2, 3, 4, 5];
          let tup: (i32, string) = (42, "test");
          let arr: [5]i32 = [0; 5];
          
          return tup.0;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  # Test 2: Type checking in expressions and operations
  Scenario: Binary operations require compatible types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 42;
          let y: string = "hello";
          let z: i32 = x + y;  // Type error: can't add i32 and string
          return z;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Arithmetic operations with mixed numeric types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 42;
          let y: f64 = 3.14;
          let z: f64 = x + y;  // Type error: need explicit conversion
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Logical operations require boolean operands
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 42;
          let y: bool = true;
          let z: bool = x && y;  // Type error: i32 is not bool
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Test 3: Generic type parameter inference and constraints
  Scenario: Generic struct instantiation with type inference
    Given I have the following Asthra code:
      """
      package test;

      struct Box<T> {
          value: T
      }

      pub fn main(none) -> i32 {
          let int_box: Box<i32> = Box<i32> { value: 42 };
          let str_box: Box<string> = Box<string> { value: "hello" };
          
          return int_box.value;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Generic function type parameter inference
    Given I have the following Asthra code:
      """
      package test;

      fn identity<T>(value: T) -> T {
          return value;
      }

      pub fn main(none) -> i32 {
          let x: i32 = identity<i32>(42);
          let y: string = identity<string>("hello");
          let z: bool = identity<bool>(true);
          
          if z {
              return x;
          }
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Mismatched generic type arguments
    Given I have the following Asthra code:
      """
      package test;

      struct Pair<T, U> {
          first: T,
          second: U
      }

      pub fn main(none) -> i32 {
          let p: Pair<i32, string> = Pair<string, i32> { 
              first: "wrong", 
              second: 42 
          };
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Test 4: Type compatibility in assignments
  Scenario: Assignment type mismatch
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut x: i32 = 42;
          let y: string = "hello";
          x = y;  // Type error: can't assign string to i32
          return x;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Assignment to immutable variable
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 42;  // Immutable by default
          x = 100;  // Error: can't assign to immutable variable
          return x;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "immutable" or "mutable"

  Scenario: Correct mutable assignment
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut x: i32 = 42;
          x = 100;  // OK: x is mutable
          return x;
      }
      """
    When I compile and run the program
    Then the exit code should be 100

  # Test 5: Type checking for function returns
  Scenario: Function return type mismatch
    Given I have the following Asthra code:
      """
      package test;

      fn get_string(none) -> string {
          return 42;  // Type error: returning i32 instead of string
      }

      pub fn main(none) -> i32 {
          let s: string = get_string();
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Void function must return unit
    Given I have the following Asthra code:
      """
      package test;

      fn do_nothing(none) -> void {
          return 42;  // Type error: void functions must return ()
      }

      pub fn main(none) -> i32 {
          do_nothing();
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type" or "void"

  Scenario: Correct void function return
    Given I have the following Asthra code:
      """
      package test;

      fn print_message(msg: string) -> void {
          log(msg);
          return ();  // Correct: explicit unit return
      }

      pub fn main(none) -> i32 {
          print_message("Hello, Asthra!");
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  # Test 6: Array and slice type validation
  Scenario: Array size must be constant expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size: i32 = 5;
          let arr: [size]i32 = [0; 5];  // Error: size must be const
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "const"

  Scenario: Correct fixed array with const size
    Given I have the following Asthra code:
      """
      package test;

      const SIZE: i32 = 5;

      pub fn main(none) -> i32 {
          let arr: [SIZE]i32 = [0; SIZE];
          return arr[0];
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Slice type compatibility
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let nums: []i32 = [1, 2, 3, 4, 5];
          let strs: []string = nums;  // Type error: []i32 != []string
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Test 7: Tuple type checking (minimum 2 elements)
  Scenario: Single element tuple is invalid
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let t: (i32) = (42);  // Error: tuples need at least 2 elements
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail

  Scenario: Valid tuple types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let pair: (i32, string) = (42, "hello");
          let triple: (bool, f64, i32) = (true, 3.14, 100);
          
          return pair.0 + triple.2;  // 42 + 100 = 142
      }
      """
    When I compile and run the program
    Then the exit code should be 142

  Scenario: Tuple element type mismatch
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let pair: (i32, string) = ("wrong", 42);  // Types swapped
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Test 8: Result and Option type validation
  Scenario: Result type construction and matching
    Given I have the following Asthra code:
      """
      package test;

      fn divide(a: i32, b: i32) -> Result<i32, string> {
          if b == 0 {
              return Result<i32, string>::Err("division by zero");
          }
          return Result<i32, string>::Ok(a / b);
      }

      pub fn main(none) -> i32 {
          let result: Result<i32, string> = divide(10, 2);
          match result {
              Result.Ok(value) => return value;,
              Result.Err(msg) => return -1;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 5

  Scenario: Option type construction and matching
    Given I have the following Asthra code:
      """
      package test;

      fn find_value(search: i32) -> Option<i32> {
          if search > 0 {
              return Option<i32>::Some(search * 2);
          }
          return Option<i32>::None;
      }

      pub fn main(none) -> i32 {
          let opt: Option<i32> = find_value(21);
          match opt {
              Option.Some(value) => return value;,
              Option.None => return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Result type parameter mismatch
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let result: Result<i32, string> = Result<string, i32>::Ok("wrong");
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Edge case: Complex nested types
  Scenario: Nested generic type validation
    Given I have the following Asthra code:
      """
      package test;

      struct Container<T> {
          items: []T
      }

      pub fn main(none) -> i32 {
          let nested: Container<Option<i32>> = Container<Option<i32>> {
              items: [Option<i32>::Some(10), Option<i32>::None, Option<i32>::Some(32)]
          };
          
          match nested.items[2] {
              Option.Some(value) => return value;,
              Option.None => return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 32

  Scenario: Type inference with method calls
    Given I have the following Asthra code:
      """
      package test;

      struct Counter {
          value: i32
      }

      impl Counter {
          fn increment(self) -> i32 {
              return self.value + 1;
          }
      }

      pub fn main(none) -> i32 {
          let c: Counter = Counter { value: 41 };
          let result: string = c.increment();  // Type error: increment returns i32
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Test pointer type checking
  Scenario: Pointer type compatibility
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 42;
          let y: string = "hello";
          
          unsafe {
              let p1: *const i32 = &x;
              let p2: *const string = p1;  // Type error: incompatible pointer types
          }
          
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Mutable vs const pointer types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut x: i32 = 42;
          
          unsafe {
              let p1: *mut i32 = &mut x;
              let p2: *const i32 = p1;  // OK: mut to const is allowed
              let p3: *mut i32 = p2;    // Error: const to mut not allowed
          }
          
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type" or "const"

  # Test type checking with literal expressions
  Scenario: Array literal type inference
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let nums: []i32 = [1, 2, "three", 4];  // Type error: string in i32 array
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Struct literal field type checking
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: f64,
          y: f64
      }

      pub fn main(none) -> i32 {
          let p: Point = Point { x: 3.14, y: "wrong" };  // Type error
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  # Test Never type
  Scenario: Never type for non-returning functions
    Given I have the following Asthra code:
      """
      package test;

      fn always_panics(none) -> Never {
          panic("This function never returns");
      }

      pub fn main(none) -> i32 {
          let x: i32 = always_panics();  // OK: Never type can be assigned to any type
          return x;  // This line is unreachable
      }
      """
    When I compile and run the program
    Then the program should panic with message "This function never returns"

  Scenario: Never type cannot be constructed
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: Never = 42;  // Error: Cannot create value of Never type
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "Never"

  # Test TaskHandle type checking
  Scenario: TaskHandle type parameter validation
    Given I have the following Asthra code:
      """
      package test;

      fn compute(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          let handle: TaskHandle<i32> = spawn_with_handle compute();
          let result: string = await handle;  // Type error: handle returns i32
          return 0;
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "type"

  Scenario: Correct TaskHandle usage
    Given I have the following Asthra code:
      """
      package test;

      fn compute(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          let handle: TaskHandle<i32> = spawn_with_handle compute();
          let result: i32 = await handle;
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 42