Feature: Control Flow Loops
  As an Asthra developer
  I want to use for loops with break and continue
  So that I can iterate over collections and control loop flow

  Background:
    Given I have a clean test environment
    And I am testing "control_flow_loops"

  # Basic for loops
  Scenario: Simple for loop with range
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(3) {
              log("Iteration");
          }
          log("For loop with range works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "For loop with range works"

  Scenario: For loop with range start and end
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(5, 8) {
              log("Counting");
          }
          log("Range with start/end works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Range with start/end works"

  Scenario: For loop over array slice
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let arr: [5]i32 = [10, 20, 30, 40, 50];
          for elem in arr[:] {
              log("Element");
          }
          log("Array iteration works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Array iteration works"

  # Break statements
  Scenario: For loop with break
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(10) {
              if i == 3 {
                  break;
              }
              log("Before break");
          }
          log("Break statement works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Break statement works"

  Scenario: Nested loops with break
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(3) {
              for j in range(3) {
                  if j == 1 {
                      break;
                  }
                  log("Inner");
              }
              log("Outer");
          }
          log("Nested break works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Nested break works"

  # Continue statements
  Scenario: For loop with continue
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(5) {
              if i == 2 {
                  continue;
              }
              log("Not skipped");
          }
          log("Continue statement works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Continue statement works"

  Scenario: Nested loops with continue
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(2) {
              for j in range(3) {
                  if j == 1 {
                      continue;
                  }
                  log("Inner iteration");
              }
              log("Outer iteration");
          }
          log("Nested continue works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Nested continue works"

  # Complex loop patterns
  Scenario: Loop with break and continue
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(10) {
              if i == 2 {
                  continue;
              }
              if i == 5 {
                  break;
              }
              log("Processing");
          }
          log("Break and continue work together");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Break and continue work together"

  Scenario: Loop variable is immutable
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(3) {
              i = 10;
              log("Should not compile");
          }
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "immutable"

  # Empty and single iteration loops
  Scenario: Empty range loop
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(0) {
              log("Should not execute");
          }
          log("Empty range works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Empty range works"

  Scenario: Single iteration loop
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(1) {
              log("Single iteration");
          }
          log("Single iteration works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Single iteration works"

  # Error scenarios
  Scenario: Break outside of loop
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          break;
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "break"

  Scenario: Continue outside of loop
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          continue;
          log("Should not compile");
          return ();
      }
      """
    When I compile the program
    Then the compilation should fail
    And the error should contain "continue"

  # Loop with mutable accumulator
  Scenario: Loop with mutable accumulator
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let mut sum: i32 = 0;
          for i in range(5) {
              sum = sum + 1;
          }
          log("Mutable accumulator works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Mutable accumulator works"

  # Iterating over string slices

  Scenario: For loop over string characters
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          let text: string = "ABC";
          for ch in text {
              log("Char");
          }
          log("String iteration works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "String iteration works"

  # Multiple breaks in nested loops
  Scenario: Multiple breaks affect only innermost loop
    Given the following Asthra code:
      """
      package test;

      pub fn main(none) -> void {
          for i in range(2) {
              log("Outer start");
              for j in range(5) {
                  if j == 2 {
                      break;
                  }
                  log("Inner");
              }
              log("Outer end");
          }
          log("Multiple breaks work correctly");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Multiple breaks work correctly"

  # Early return from loop
  Scenario: Return from within loop
    Given the following Asthra code:
      """
      package test;

      pub fn find_value(none) -> i32 {
          for i in range(10) {
              if i == 5 {
                  return i;
              }
          }
          return -1;
      }

      pub fn main(none) -> void {
          let result: i32 = find_value();
          log("Early return from loop works");
          return ();
      }
      """
    When I compile and run the program
    Then the compilation should succeed
    And the output should contain "Early return from loop works"