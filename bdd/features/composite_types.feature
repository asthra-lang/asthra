Feature: Composite Types
  As an Asthra developer
  I want to use composite types like arrays, slices, and tuples
  So that I can work with collections and grouped data

  Background:
    Given the Asthra compiler is available

  Scenario: Fixed-size array declaration
    Given I have a file "fixed_array.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let numbers: [5]i32 = [1, 2, 3, 4, 5];
          let bytes: [10]u8 = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
          log("Fixed arrays work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Fixed arrays work"
    And the exit code should be 0

  Scenario: Array with constant size expression
    Given I have a file "array_const_size.asthra" with:
      """
      package main;
      
      pub const BUFFER_SIZE: i32 = 256;
      
      pub fn main(none) -> void {
          let buffer: [BUFFER_SIZE]u8 = [0; BUFFER_SIZE];
          log("Array with const size works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Array with const size works"
    And the exit code should be 0

  Scenario: Dynamic slice type
    Given I have a file "slice_type.asthra" with:
      """
      package main;
      
      pub fn process_slice(data: []i32) -> void {
          log("Processing slice");
          return ();
      }
      
      pub fn main(none) -> void {
          let numbers: []i32 = [1, 2, 3, 4, 5];
          process_slice(numbers);
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Processing slice"
    And the exit code should be 0

  Scenario: Tuple type with two elements
    Given I have a file "tuple_two.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let pair: (i32, string) = (42, "answer");
          let coords: (f64, f64) = (3.14, 2.71);
          log("Tuples work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Tuples work"
    And the exit code should be 0

  @wip
  Scenario: Tuple type with multiple elements
    Given I have a file "tuple_multiple.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let triple: (i32, string, bool) = (1, "hello", true);
          let quad: (u8, u16, u32, u64) = (1, 2, 3, 4);
          log("Multiple element tuples work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Multiple element tuples work"
    And the exit code should be 0

  @wip
  Scenario: Nested arrays
    Given I have a file "nested_arrays.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let matrix: [3][3]i32 = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];
          log("Nested arrays work");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Nested arrays work"
    And the exit code should be 0

  @wip
  Scenario: Array of tuples
    Given I have a file "array_of_tuples.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let points: [3](i32, i32) = [(0, 0), (1, 1), (2, 2)];
          log("Array of tuples works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Array of tuples works"
    And the exit code should be 0

  @wip
  Scenario: Tuple of arrays
    Given I have a file "tuple_of_arrays.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let data: ([3]i32, [3]string) = ([1, 2, 3], ["a", "b", "c"]);
          log("Tuple of arrays works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Tuple of arrays works"
    And the exit code should be 0

  @wip
  Scenario: Slice of slices
    Given I have a file "slice_of_slices.asthra" with:
      """
      package main;
      
      pub fn process_matrix(data: [][]i32) -> void {
          log("Processing matrix");
          return ();
      }
      
      pub fn main(none) -> void {
          let matrix: [][]i32 = [[1, 2], [3, 4]];
          process_matrix(matrix);
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Processing matrix"
    And the exit code should be 0

  @wip
  Scenario: Array size mismatch error
    Given I have a file "array_size_mismatch.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let numbers: [3]i32 = [1, 2, 3, 4, 5];
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "array size mismatch"

  @wip
  Scenario: Invalid tuple with one element
    Given I have a file "single_tuple.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let single: (i32) = (42);
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "tuple must have at least 2 elements"

  @wip
  Scenario: Zero-sized array error
    Given I have a file "zero_array.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let empty: [0]i32 = [];
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "array size must be greater than 0"

  Scenario: Mutable pointer type
    Given I have a file "mut_pointer.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let mut x: i32 = 42;
          let ptr: *mut i32 = &mut x;
          log("Mutable pointer works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Mutable pointer works"
    And the exit code should be 0

  @wip
  Scenario: Const pointer type
    Given I have a file "const_pointer.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 42;
          let ptr: *const i32 = &x;
          log("Const pointer works");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Const pointer works"
    And the exit code should be 0