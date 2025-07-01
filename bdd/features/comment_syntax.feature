Feature: Comment syntax
  As a developer
  I want to use single-line and multi-line comments
  So that I can document my code and add explanatory notes

  Background:
    Given the Asthra compiler is available

  Scenario: Single-line comments with double slash
    Given I have the following Asthra code:
      """
      package test;

      // This is a single-line comment
      pub fn main(none) -> i32 {
          // Another comment inside function
          let value: i32 = 42; // End-of-line comment
          return value;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Multiple single-line comments
    Given I have the following Asthra code:
      """
      package test;

      // First comment line
      // Second comment line
      // Third comment line
      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Multi-line comments with block syntax
    Given I have the following Asthra code:
      """
      package test;

      /*
       * This is a multi-line comment
       * spanning several lines
       * with detailed explanation
       */
      pub fn main(none) -> i32 {
          return 1;
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Nested multi-line comments
    Given I have the following Asthra code:
      """
      package test;

      /*
       * Outer comment starts here
       * /* This is a nested comment */
       * Outer comment continues
       */
      pub fn main(none) -> i32 {
          return 2;
      }
      """
    When I compile and run the program
    Then the exit code should be 2

  Scenario: Comments between declarations
    Given I have the following Asthra code:
      """
      package test;

      // Function to calculate square
      pub fn square(x: i32) -> i32 {
          return x * x;
      }

      /* 
       * Main function entry point
       */
      pub fn main(none) -> i32 {
          // Calculate square of 5
          let result = square(5);
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 25

  Scenario: Comments in struct definitions
    Given I have the following Asthra code:
      """
      package test;

      // Point structure for 2D coordinates
      struct Point {
          x: i32, // X coordinate
          y: i32  // Y coordinate
      }

      pub fn main(none) -> i32 {
          /* Create a new point */
          let point: Point = Point { 
              x: 3, // Set X
              y: 4  // Set Y
          };
          return point.x + point.y;
      }
      """
    When I compile and run the program
    Then the exit code should be 7

  Scenario: Comments in function parameters
    Given I have the following Asthra code:
      """
      package test;

      pub fn calculate(
          a: i32, // First parameter
          b: i32, // Second parameter
          c: i32  // Third parameter
      ) -> i32 {
          // Perform calculation
          return a + b * c;
      }

      pub fn main(none) -> i32 {
          return calculate(
              2, // Base value
              3, // Multiplier
              4  // Multiplicand
          );
      }
      """
    When I compile and run the program
    Then the exit code should be 14

  Scenario: Comments in control flow
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 10;
          
          // Check if value is positive
          if value > 0 {
              // Value is positive
              return 1;
          } else {
              /* Value is not positive */
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 1

  Scenario: Comments in loops
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut sum: i32 = 0;
          
          // Iterate through numbers 0 to 4
          for i in range(5) {
              // Add current number to sum
              sum = sum + i;
              /* Continue to next iteration */
          }
          
          return sum; // Return total sum
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Comments with special characters
    Given I have the following Asthra code:
      """
      package test;

      // Comment with special chars: !@#$%^&*()_+-={}[]|\\:";'<>?,.
      pub fn main(none) -> i32 {
          /* 
           * Multi-line with symbols: ~`!@#$%^&*()_+-={}[]|\\:";'<>?,.
           * And unicode: αβγδε
           */
          return 42;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Comment-only lines
    Given I have the following Asthra code:
      """
      package test;

      //
      // Empty comment lines
      //

      /*
       *
       * Empty multi-line sections
       *
       */

      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Comments at end of file
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          return 123;
      }

      // End of file comment
      /* Final multi-line comment */
      """
    When I compile and run the program
    Then the exit code should be 123

  Scenario: Mixed comment styles
    Given I have the following Asthra code:
      """
      package test;

      // Single line comment
      pub fn helper(value: i32) -> i32 {
          /* Multi-line comment in function */
          return value * 2; // End-of-line comment
      }

      /*
       * Main function documentation
       * Uses both comment styles
       */
      pub fn main(none) -> i32 {
          // Call helper function
          let result = helper(21); /* Get doubled value */
          return result; // Return result
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Comments in array and struct literals
    Given I have the following Asthra code:
      """
      package test;

      struct Config {
          width: i32,
          height: i32
      }

      pub fn main(none) -> i32 {
          // Array with comments
          let numbers: [3]i32 = [
              1, // First element
              2, // Second element
              3  // Third element
          ];

          /* Struct with comments */
          let config: Config = Config {
              width: 800,  // Screen width
              height: 600  // Screen height
          };

          return numbers[0] + config.width / 100;
      }
      """
    When I compile and run the program
    Then the exit code should be 9

  Scenario: Comments in enum definitions
    Given I have the following Asthra code:
      """
      package test;

      // Status enumeration
      enum Status {
          // Success variant with value
          Success(i32),
          // Error variant without value
          Error
      }

      pub fn main(none) -> i32 {
          /* Create success status */
          let status: Status = Status.Success(42);
          return 0; // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Comments in match statements
    Given I have the following Asthra code:
      """
      package test;

      enum Result {
          Ok(i32),
          Err
      }

      pub fn main(none) -> i32 {
          let result: Result = Result.Ok(10);
          
          // Match on result
          match result {
              // Success case
              Result.Ok(value) => {
                  return value; // Return the value
              }
              /* Error case */
              Result.Err => {
                  return -1; // Return error code
              }
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Long comment blocks
    Given I have the following Asthra code:
      """
      package test;

      /*
       * This is a very long comment block that spans multiple lines
       * and contains detailed documentation about the function below.
       * It explains the purpose, parameters, return value, and any
       * special considerations that developers should be aware of.
       * 
       * The function calculates the factorial of a given number
       * using iterative approach for better performance compared
       * to recursive implementation.
       */
      pub fn factorial(n: i32) -> i32 {
          // Initialize result to 1
          let mut result: i32 = 1;
          
          // Iterate from 1 to n
          for i in range(1, n + 1) {
              // Multiply result by current number
              result = result * i;
          }
          
          // Return computed factorial
          return result;
      }

      pub fn main(none) -> i32 {
          return factorial(5); // 5! = 120
      }
      """
    When I compile and run the program
    Then the exit code should be 120

  Scenario: Error - Unclosed multi-line comment
    Given I have the following Asthra code:
      """
      package test;

      /*
       * This comment is never closed
       * Missing closing delimiter
      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "unclosed comment" or "expected */"

  Scenario: Comments with code-like content
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          // let commented_var: i32 = 999;
          /* 
           * if false_condition {
           *     return 999;
           * }
           */
          return 42; // This should execute normally
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Comments in import and package sections
    Given I have the following Asthra code:
      """
      // Package declaration with comment
      package test;

      // Import statements (none in this example)
      /* No imports needed for this simple test */

      // Main function
      pub fn main(none) -> i32 {
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0