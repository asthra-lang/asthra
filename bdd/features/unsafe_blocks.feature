Feature: Unsafe blocks
  As a developer
  I want to use unsafe blocks for FFI and raw pointer operations
  So that I can perform low-level operations while maintaining safety boundaries

  Background:
    Given the Asthra compiler is available

  Scenario: Basic unsafe block
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          unsafe {
              // Perform unsafe operations here
              let value: i32 = 42;
              return value;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Unsafe block with pointer operations
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 100;
          let ptr: *const i32 = &value;
          
          unsafe {
              let dereferenced = *ptr;
              return dereferenced;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 100

  Scenario: Unsafe block with mutable pointer
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut value: i32 = 50;
          let ptr: *mut i32 = &value;
          
          unsafe {
              *ptr = 75;
          }
          
          return value;
      }
      """
    When I compile and run the program
    Then the exit code should be 75

  Scenario: Nested unsafe blocks
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 10;
          let ptr: *const i32 = &value;
          
          unsafe {
              let intermediate = *ptr;
              unsafe {
                  let final_value = intermediate * 2;
                  return final_value;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 20

  Scenario: Unsafe block with FFI call
    Given I have the following Asthra code:
      """
      package test;

      extern fn get_unsafe_value(none) -> i32;

      pub fn main(none) -> i32 {
          unsafe {
              let result = get_unsafe_value();
              return result;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Unsafe block with multiple statements
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let x: i32 = 5;
          let y: i32 = 10;
          let ptr_x: *const i32 = &x;
          let ptr_y: *const i32 = &y;
          
          unsafe {
              let val_x = *ptr_x;
              let val_y = *ptr_y;
              let sum = val_x + val_y;
              let result = sum * 2;
              return result;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 30

  Scenario: Unsafe block in control flow
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let should_use_unsafe: bool = true;
          let value: i32 = 25;
          let ptr: *const i32 = &value;
          
          if should_use_unsafe {
              unsafe {
                  let result = *ptr;
                  return result;
              }
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 25

  Scenario: Unsafe block in loop
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let values: [3]i32 = [10, 20, 30];
          let mut total: i32 = 0;
          
          for i in range(3) {
              unsafe {
                  let ptr: *const i32 = &values[i];
                  let value = *ptr;
                  total = total + value;
              }
          }
          
          return total;
      }
      """
    When I compile and run the program
    Then the exit code should be 60

  Scenario: Unsafe block with function call
    Given I have the following Asthra code:
      """
      package test;

      pub fn unsafe_operation(ptr: *const i32) -> i32 {
          unsafe {
              return *ptr;
          }
      }

      pub fn main(none) -> i32 {
          let value: i32 = 123;
          let result = unsafe_operation(&value);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 123

  Scenario: Unsafe block with variable declaration
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let source: i32 = 456;
          let source_ptr: *const i32 = &source;
          
          unsafe {
              let copied_value: i32 = *source_ptr;
              let multiplied = copied_value * 2;
              return multiplied;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 912

  Scenario: Mixed safe and unsafe code
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let safe_value: i32 = 15;
          let safe_result = safe_value + 5;
          
          let ptr: *const i32 = &safe_result;
          let unsafe_result: i32 = unsafe {
              *ptr
          };
          
          return unsafe_result + 10;
      }
      """
    When I compile and run the program
    Then the exit code should be 30

  Scenario: Unsafe block with array pointer arithmetic
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [5]i32 = [1, 2, 3, 4, 5];
          let base_ptr: *const i32 = &array[0];
          
          unsafe {
              // Access third element (index 2)
              let third_ptr = base_ptr + 2;  // Pointer arithmetic
              let third_value = *third_ptr;
              return third_value;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Error - Pointer dereference outside unsafe block
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let ptr: *const i32 = &value;
          let result = *ptr;  // Error: dereferencing outside unsafe block
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "unsafe operation" or "requires unsafe block"

  Scenario: Error - FFI call outside unsafe block
    Given I have the following Asthra code:
      """
      package test;

      extern fn unsafe_function(none) -> i32;

      pub fn main(none) -> i32 {
          let result = unsafe_function();  // Error: FFI call outside unsafe
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "unsafe operation" or "requires unsafe block"

  Scenario: Error - Missing braces in unsafe block
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let ptr: *const i32 = &value;
          
          unsafe  // Error: missing braces
              let result = *ptr;
              return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected '{'" or "syntax error"

  Scenario: Unsafe block with struct field access
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      pub fn main(none) -> i32 {
          let point: Point = Point { x: 10, y: 20 };
          let ptr: *const Point = &point;
          
          unsafe {
              let accessed_point = *ptr;
              return accessed_point.x + accessed_point.y;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 30

  Scenario: Unsafe block scope and variable visibility
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let outer_value: i32 = 100;
          
          unsafe {
              let inner_value: i32 = 200;
              let total = outer_value + inner_value;  // Access outer scope
              return total;
          }
          
          // inner_value not accessible here
      }
      """
    When I compile and run the program
    Then the exit code should be 300