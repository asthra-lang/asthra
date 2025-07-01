Feature: Slice operations
  As a developer
  I want to perform Go-style array slicing operations
  So that I can create sub-arrays and manipulate array segments

  Background:
    Given the Asthra compiler is available

  Scenario: Basic slice with start and end
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [5]i32 = [10, 20, 30, 40, 50];
          let slice: []i32 = array[1:4];  // Elements 20, 30, 40
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Slice with start only
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [4]i32 = [1, 2, 3, 4];
          let slice: []i32 = array[2:];  // Elements 3, 4
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 2

  Scenario: Slice with end only
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [6]i32 = [1, 2, 3, 4, 5, 6];
          let slice: []i32 = array[:3];  // Elements 1, 2, 3
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Full slice copy
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [3]i32 = [100, 200, 300];
          let slice: []i32 = array[:];  // Full copy
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Slice with variables as bounds
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [7]i32 = [1, 2, 3, 4, 5, 6, 7];
          let start: i32 = 2;
          let end: i32 = 5;
          let slice: []i32 = array[start:end];  // Elements 3, 4, 5
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Slice access with indexing
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [5]i32 = [10, 20, 30, 40, 50];
          let slice: []i32 = array[1:4];
          return slice[1];  // Second element of slice (30)
      }
      """
    When I compile and run the program
    Then the exit code should be 30

  Scenario: Nested slicing
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [8]i32 = [1, 2, 3, 4, 5, 6, 7, 8];
          let first_slice: []i32 = array[1:6];  // [2, 3, 4, 5, 6]
          let second_slice: []i32 = first_slice[1:3];  // [3, 4]
          return len(second_slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 2

  Scenario: Slice of different types
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let bool_array: [4]bool = [true, false, true, false];
          let bool_slice: []bool = bool_array[1:3];
          
          let float_array: [3]f32 = [1.0, 2.5, 3.7];
          let float_slice: []f32 = float_array[:2];
          
          return len(bool_slice) + len(float_slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 4

  Scenario: Empty slice
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [5]i32 = [1, 2, 3, 4, 5];
          let empty_slice: []i32 = array[2:2];  // Empty slice
          return len(empty_slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Slice with struct elements
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      pub fn main(none) -> i32 {
          let points: [3]Point = [
              Point { x: 1, y: 2 },
              Point { x: 3, y: 4 },
              Point { x: 5, y: 6 }
          ];
          let slice: []Point = points[1:];  // Last 2 points
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 2

  Scenario: Slice in function parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn process_slice(data: []i32) -> i32 {
          let mut sum: i32 = 0;
          for i in range(len(data)) {
              sum = sum + data[i];
          }
          return sum;
      }

      pub fn main(none) -> i32 {
          let array: [4]i32 = [10, 20, 30, 40];
          let slice: []i32 = array[1:3];  // [20, 30]
          return process_slice(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 50

  Scenario: Slice assignment and mutation
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut array: [4]i32 = [1, 2, 3, 4];
          let mut slice: []i32 = array[1:3];  // [2, 3]
          slice[0] = 99;  // Modify through slice
          return array[1];  // Should reflect the change
      }
      """
    When I compile and run the program
    Then the exit code should be 99

  Scenario: Slice bounds with expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [6]i32 = [1, 2, 3, 4, 5, 6];
          let offset: i32 = 1;
          let size: i32 = 3;
          let slice: []i32 = array[offset:offset + size];  // [2, 3, 4]
          return len(slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Zero-based slice indexing
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [5]i32 = [100, 200, 300, 400, 500];
          let slice: []i32 = array[2:4];  // [300, 400]
          return slice[0];  // First element of slice (300)
      }
      """
    When I compile and run the program
    Then the exit code should be 300

  Scenario: Slice from slice type
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_slice(none) -> []i32 {
          let array: [5]i32 = [10, 20, 30, 40, 50];
          return array[1:4];
      }

      pub fn main(none) -> i32 {
          let slice: []i32 = get_slice();
          let sub_slice: []i32 = slice[1:];  // Slice from slice
          return len(sub_slice);
      }
      """
    When I compile and run the program
    Then the exit code should be 2

  Scenario: Error - Slice bounds out of range
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [3]i32 = [1, 2, 3];
          let slice: []i32 = array[1:5];  // End beyond array bounds
          return len(slice);
      }
      """
    When I compile and run the program
    Then the program should terminate with runtime error or handle bounds checking

  Scenario: Error - Invalid slice bounds (start > end)
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [4]i32 = [1, 2, 3, 4];
          let slice: []i32 = array[3:1];  // Invalid: start > end
          return len(slice);
      }
      """
    When I compile and run the program
    Then the program should terminate with runtime error or handle bounds checking

  Scenario: Error - Negative slice bounds
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [4]i32 = [1, 2, 3, 4];
          let slice: []i32 = array[-1:2];  // Negative start index
          return len(slice);
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "invalid bounds" or "negative index"

  Scenario: Error - Wrong slice syntax
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [4]i32 = [1, 2, 3, 4];
          let slice: []i32 = array[1,3];  // Error: should use colon
          return len(slice);
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected ':'" or "slice syntax"

  Scenario: Slice iteration
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [5]i32 = [2, 4, 6, 8, 10];
          let slice: []i32 = array[1:4];  // [4, 6, 8]
          let mut sum: i32 = 0;
          
          for element in slice {
              sum = sum + element;
          }
          
          return sum;
      }
      """
    When I compile and run the program
    Then the exit code should be 18

  Scenario: Slice length property
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let array: [10]i32 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
          let slice1: []i32 = array[2:7];
          let slice2: []i32 = array[:5];
          let slice3: []i32 = array[8:];
          
          return len(slice1) + len(slice2) + len(slice3);
      }
      """
    When I compile and run the program
    Then the exit code should be 12

  Scenario: Complex slice operations
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let data: [8]i32 = [1, 2, 3, 4, 5, 6, 7, 8];
          
          // Multiple slice operations
          let middle: []i32 = data[2:6];  // [3, 4, 5, 6]
          let start_of_middle: []i32 = middle[:2];  // [3, 4]
          let end_of_middle: []i32 = middle[2:];  // [5, 6]
          
          let total_length = len(start_of_middle) + len(end_of_middle);
          return total_length;
      }
      """
    When I compile and run the program
    Then the exit code should be 4