Feature: Sizeof operator
  As a developer
  I want to get compile-time size calculations of types
  So that I can perform memory calculations and type-aware operations

  Background:
    Given the Asthra compiler is available

  Scenario: Sizeof primitive types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_i32 = sizeof(i32);
          let size_bool = sizeof(bool);
          let size_u8 = sizeof(u8);
          
          return size_i32 + size_bool + size_u8;
      }
      """
    When I compile and run the program
    Then the exit code should be 6

  Scenario: Sizeof integer types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_i8 = sizeof(i8);
          let size_i16 = sizeof(i16);
          let size_i32 = sizeof(i32);
          let size_i64 = sizeof(i64);
          
          if size_i8 == 1 && size_i16 == 2 && size_i32 == 4 && size_i64 == 8 {
              return 1;  // Sizes are as expected
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof unsigned integer types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_u8 = sizeof(u8);
          let size_u16 = sizeof(u16);
          let size_u32 = sizeof(u32);
          let size_u64 = sizeof(u64);
          
          if size_u8 == 1 && size_u16 == 2 && size_u32 == 4 && size_u64 == 8 {
              return 1;  // Sizes are as expected
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof floating-point types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_f32 = sizeof(f32);
          let size_f64 = sizeof(f64);
          
          if size_f32 == 4 && size_f64 == 8 {
              return 1;  // Float sizes are correct
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof pointer types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_const_ptr = sizeof(*const i32);
          let size_mut_ptr = sizeof(*mut i32);
          let size_ptr_to_ptr = sizeof(*const *const i32);
          
          // All pointers should be the same size (8 bytes on 64-bit)
          if size_const_ptr == size_mut_ptr && size_mut_ptr == size_ptr_to_ptr {
              return size_const_ptr as i32;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 8

  Scenario: Sizeof array types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_array_3_i32 = sizeof([3]i32);
          let size_array_5_u8 = sizeof([5]u8);
          let size_array_2_f64 = sizeof([2]f64);
          
          // [3]i32 = 3 * 4 = 12, [5]u8 = 5 * 1 = 5, [2]f64 = 2 * 8 = 16
          let expected_total = 12 + 5 + 16;
          let actual_total = size_array_3_i32 + size_array_5_u8 + size_array_2_f64;
          
          if actual_total == expected_total {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof struct types
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      struct Person {
          age: i32,
          active: bool
      }

      pub fn main(none) -> i32 {
          let size_point = sizeof(Point);
          let size_person = sizeof(Person);
          
          // Point has 2 i32s = 8 bytes, Person has i32 + bool with padding
          if size_point >= 8 && size_person >= 5 {
              return 1;  // Reasonable sizes
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof with const expressions
    Given I have the following Asthra code:
      """
      package test;

      const ARRAY_SIZE: usize = sizeof([10]i32);
      const POINTER_SIZE: usize = sizeof(*const void);

      pub fn main(none) -> i32 {
          if ARRAY_SIZE == 40 && POINTER_SIZE == 8 {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof in array size expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let buffer: [sizeof(i64)]u8 = [0; sizeof(i64)];
          let buffer_size = sizeof([sizeof(i64)]u8);
          
          if buffer_size == 8 {
              return 1;  // Buffer is 8 bytes
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof tuple types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_tuple_2 = sizeof((i32, bool));
          let size_tuple_3 = sizeof((u8, u16, u32));
          
          // Tuples may have padding for alignment
          if size_tuple_2 >= 5 && size_tuple_3 >= 7 {
              return 1;  // Reasonable sizes considering padding
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof slice type
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_slice = sizeof([]i32);
          
          // Slice is a fat pointer (pointer + length)
          if size_slice == 16 {  // 8 bytes ptr + 8 bytes len on 64-bit
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof Option and Result types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size_option = sizeof(Option<i32>);
          let size_result = sizeof(Result<i32, bool>);
          
          // Option and Result are enums with data
          if size_option >= sizeof(i32) && size_result >= sizeof(i32) + sizeof(bool) {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof in memory allocation context
    Given I have the following Asthra code:
      """
      package test;

      pub fn calculate_buffer_size(count: usize) -> usize {
          return count * sizeof(f64);
      }

      pub fn main(none) -> i32 {
          let buffer_size = calculate_buffer_size(10);
          let expected_size = 10 * 8;  // 10 f64s = 80 bytes
          
          if buffer_size == expected_size {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof with generic types
    Given I have the following Asthra code:
      """
      package test;

      struct Container<T> {
          value: T,
          count: i32
      }

      pub fn main(none) -> i32 {
          let size_i32_container = sizeof(Container<i32>);
          let size_u8_container = sizeof(Container<u8>);
          
          // Both should have i32 for count plus the type T with potential padding
          if size_i32_container >= 8 && size_u8_container >= 5 {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof comparison operations
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          if sizeof(i64) > sizeof(i32) && 
             sizeof(i32) > sizeof(i16) && 
             sizeof(i16) > sizeof(i8) {
              return 1;  // Size ordering is correct
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Error - Sizeof with invalid type
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size = sizeof(UndefinedType);  // Error: type not defined
          return size as i32;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "undefined type" or "not found"

  Scenario: Error - Sizeof with expression instead of type
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let size = sizeof(value + 1);  // Error: expression not allowed
          return size as i32;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected type" or "invalid sizeof argument"

  Scenario: Error - Missing parentheses in sizeof
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let size = sizeof i32;  // Error: missing parentheses
          return size as i32;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected '('" or "syntax error"

  Scenario: Sizeof zero-sized types
    Given I have the following Asthra code:
      """
      package test;

      struct Empty {
          none
      }

      pub fn main(none) -> i32 {
          let size_empty = sizeof(Empty);
          let size_unit = sizeof(());
          
          // Zero-sized types should have size 0 or 1 (implementation defined)
          if size_empty <= 1 && size_unit <= 1 {
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Sizeof in compile-time context
    Given I have the following Asthra code:
      """
      package test;

      const BUFFER_ELEMENT_SIZE: usize = sizeof(f32);
      const BUFFER_COUNT: usize = 256;
      const TOTAL_BUFFER_SIZE: usize = BUFFER_ELEMENT_SIZE * BUFFER_COUNT;

      pub fn main(none) -> i32 {
          if TOTAL_BUFFER_SIZE == 1024 {  // 4 * 256
              return 1;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1