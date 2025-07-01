Feature: Pointer operations
  As a developer
  I want to perform pointer dereferencing and addressing operations
  So that I can work with memory addresses and perform low-level operations

  Background:
    Given the Asthra compiler is available

  Scenario: Basic address-of operator
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let ptr: *const i32 = &value;
          
          unsafe {
              let result = *ptr;
              return result;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Mutable pointer operations
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut value: i32 = 10;
          let ptr: *mut i32 = &value;
          
          unsafe {
              *ptr = 25;
          }
          
          return value;
      }
      """
    When I compile and run the program
    Then the exit code should be 25

  Scenario: Const pointer operations
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 100;
          let const_ptr: *const i32 = &value;
          
          unsafe {
              let read_value = *const_ptr;
              return read_value;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 100

  Scenario: Pointer to different types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let int_val: i32 = 42;
          let float_val: f32 = 3.14;
          let bool_val: bool = true;
          
          let int_ptr: *const i32 = &int_val;
          let float_ptr: *const f32 = &float_val;
          let bool_ptr: *const bool = &bool_val;
          
          unsafe {
              let int_result = *int_ptr;
              let bool_result = *bool_ptr;
              
              if bool_result {
                  return int_result;
              } else {
                  return 0;
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Pointer to struct
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      pub fn main(none) -> i32 {
          let point: Point = Point { x: 5, y: 10 };
          let ptr: *const Point = &point;
          
          unsafe {
              let accessed_point = *ptr;
              return accessed_point.x + accessed_point.y;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 15

  Scenario: Pointer to array element
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [3]i32 = [10, 20, 30];
          let ptr: *const i32 = &array[1];
          
          unsafe {
              let element = *ptr;
              return element;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 20

  Scenario: Pointer arithmetic
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [4]i32 = [1, 2, 3, 4];
          let base_ptr: *const i32 = &array[0];
          
          unsafe {
              let first = *base_ptr;
              let second = *(base_ptr + 1);
              let third = *(base_ptr + 2);
              
              return first + second + third;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 6

  Scenario: Double pointer indirection
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 99;
          let ptr: *const i32 = &value;
          let ptr_to_ptr: *const *const i32 = &ptr;
          
          unsafe {
              let retrieved_ptr = *ptr_to_ptr;
              let final_value = *retrieved_ptr;
              return final_value;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 99

  Scenario: Pointer comparison
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value1: i32 = 10;
          let value2: i32 = 20;
          let ptr1: *const i32 = &value1;
          let ptr2: *const i32 = &value2;
          let ptr3: *const i32 = &value1;
          
          if ptr1 == ptr3 {
              return 1;  // Same address
          } else if ptr1 == ptr2 {
              return 2;  // Different addresses
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Null pointer handling
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let null_ptr: *const i32 = 0 as *const i32;
          let value: i32 = 42;
          let valid_ptr: *const i32 = &value;
          
          if null_ptr == 0 as *const i32 {
              return 1;  // Null pointer detected
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Pointer assignment and mutation
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut first: i32 = 10;
          let mut second: i32 = 20;
          let mut ptr: *mut i32 = &first;
          
          unsafe {
              *ptr = 15;  // Modify first through pointer
              ptr = &second;  // Point to second
              *ptr = 25;  // Modify second through pointer
          }
          
          return first + second;
      }
      """
    When I compile and run the program
    Then the exit code should be 40

  Scenario: Pointer in function parameters
    Given I have the following Asthra code:
      """
      package test;

      pub fn modify_through_pointer(ptr: *mut i32, new_value: i32) -> void {
          unsafe {
              *ptr = new_value;
          }
          return ();
      }

      pub fn main(none) -> i32 {
          let mut value: i32 = 5;
          modify_through_pointer(&value, 50);
          return value;
      }
      """
    When I compile and run the program
    Then the exit code should be 50

  Scenario: Pointer return from function
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_pointer(value: *const i32) -> *const i32 {
          return value;
      }

      pub fn main(none) -> i32 {
          let number: i32 = 77;
          let ptr = get_pointer(&number);
          
          unsafe {
              let result = *ptr;
              return result;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 77

  Scenario: Error - Dereferencing without unsafe block
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let ptr: *const i32 = &value;
          let result = *ptr;  // Error: unsafe operation
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "unsafe operation" or "requires unsafe block"

  Scenario: Error - Invalid pointer arithmetic
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let ptr: *const i32 = &value;
          let invalid_ptr = ptr + 1000000;  // Potentially invalid
          
          unsafe {
              let result = *invalid_ptr;  // May cause runtime error
              return result;
          }
      }
      """
    When I compile and run the program
    Then the program may crash or behave unpredictably

  Scenario: Error - Modifying through const pointer
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let const_ptr: *const i32 = &value;
          
          unsafe {
              *const_ptr = 50;  // Error: cannot modify through const pointer
          }
          
          return value;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "const pointer" or "cannot modify"

  Scenario: Pointer with different sized types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let byte_val: u8 = 255;
          let int_val: i32 = 1000;
          let long_val: i64 = 99999;
          
          let byte_ptr: *const u8 = &byte_val;
          let int_ptr: *const i32 = &int_val;
          let long_ptr: *const i64 = &long_val;
          
          unsafe {
              let b = *byte_ptr as i32;
              let i = *int_ptr;
              let l = *long_ptr as i32;
              
              return b + i + l;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 101254

  Scenario: Pointer to function parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn process_by_reference(value: *const i32) -> i32 {
          unsafe {
              return (*value) * 2;
          }
      }

      pub fn main(none) -> i32 {
          let number: i32 = 15;
          let result = process_by_reference(&number);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 30