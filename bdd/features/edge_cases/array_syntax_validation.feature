@edge_cases @parser @negative
Feature: Array and Slice Syntax Validation
  As a language designer
  I want to ensure arrays and slices use correct Asthra syntax
  So that AI doesn't hallucinate syntax from other languages

  Background:
    Given the Asthra compiler is available

  # Negative test cases - Common mistakes from other languages

  Scenario: Reject C-style array declaration with size after name
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr[5]: i32;  // Error: Size goes in type, not after variable name
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected ':' but found '['"

  Scenario: Reject Go-style array declaration with size before element type
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [5]i32;  // Error: Size goes after type: [i32; 5]
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected type but found integer literal"

  Scenario: Reject JavaScript-style array literal with Array constructor
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [i32] = Array(1, 2, 3);  // Error: No Array constructor
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "undefined identifier 'Array'"

  Scenario: Reject C++-style vector syntax
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let vec: vector<i32>;  // Error: Use [i32] for slices
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "unknown type 'vector'"

  Scenario: Reject Python-style list comprehension
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let squares: [i32] = [x * x for x in range(5)];  // Error: No list comprehensions
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected expression but found 'for'"

  Scenario: Reject TypeScript-style array type with Array<T>
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let numbers: Array<i32>;  // Error: Use [i32] not Array<i32>
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "unknown type 'Array'"

  Scenario: Reject Rust-style vec! macro
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let v: [i32] = vec![1, 2, 3];  // Error: No vec! macro
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "undefined identifier 'vec'"

  Scenario: Reject incorrect slice range syntax with dots
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr: [i32; 5] = [1, 2, 3, 4, 5];
        let slice: [i32] = arr[1..3];  // Error: Use ':' not '..' for slices
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "expected ':' or ']' but found '..'"

  Scenario: Reject empty array literal without explicit type
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let arr = [];  // Error: Empty array needs type annotation
        return 42;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should contain "cannot infer type of empty array"

  # Positive test cases - Correct syntax
  
  Scenario: Correct array and slice syntax
    Given I have the following Asthra code:
      """
      package test;
      pub fn main(none) -> i32 {
        let fixed: [i32; 3] = [1, 2, 3];
        let slice: [i32] = [10, 20, 30, 40];
        let empty: [i32] = none;
        let sub: [i32] = slice[1:3];
        return fixed[1] + slice[0];
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the exit code should be 12