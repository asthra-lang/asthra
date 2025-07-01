Feature: Predeclared identifiers
  As a developer
  I want to use predeclared utility functions
  So that I can access common functionality without explicit imports

  Background:
    Given the Asthra compiler is available

  Scenario: Basic log function
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          log("Hello, World!");
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0
    And the stderr should contain "Hello, World!"

  Scenario: Log with variable
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let message: string = "Debug message";
          log(message);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0
    And the stderr should contain "Debug message"

  Scenario: Multiple log calls
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          log("First message");
          log("Second message");
          log("Third message");
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0
    And the stderr should contain "First message"
    And the stderr should contain "Second message"
    And the stderr should contain "Third message"

  Scenario: Range function with single argument
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut sum: i32 = 0;
          for i in range(5) {
              sum = sum + i;
          }
          return sum;
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Range function with start and end
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut sum: i32 = 0;
          for i in range(3, 7) {
              sum = sum + i;
          }
          return sum;
      }
      """
    When I compile and run the program
    Then the exit code should be 18

  Scenario: Range with variable arguments
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let start: i32 = 2;
          let end: i32 = 6;
          let mut count: i32 = 0;
          
          for i in range(start, end) {
              count = count + 1;
          }
          
          return count;
      }
      """
    When I compile and run the program
    Then the exit code should be 4

  Scenario: Empty range
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut iterations: i32 = 0;
          for i in range(0) {
              iterations = iterations + 1;
          }
          return iterations;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Range with equal start and end
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let mut iterations: i32 = 0;
          for i in range(5, 5) {
              iterations = iterations + 1;
          }
          return iterations;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Panic function
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          panic("This is a panic message");
          return 0;  // Should never reach here
      }
      """
    When I compile and run the program
    Then the program should terminate with non-zero exit code
    And the stderr should contain "This is a panic message"

  Scenario: Conditional panic
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let should_panic: bool = false;
          
          if should_panic {
              panic("Should not panic");
          }
          
          return 42;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Exit function with status code
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          exit(123);
          return 0;  // Should never reach here
      }
      """
    When I compile and run the program
    Then the exit code should be 123

  Scenario: Conditional exit
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let error_condition: bool = true;
          
          if error_condition {
              exit(99);
          }
          
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 99

  Scenario: Args function basic usage
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let arguments: []string = args();
          return 0;  // Just test that it compiles and runs
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Args function with command line arguments
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let arguments: []string = args();
          log("Program started");
          return 0;
      }
      """
    When I compile and run the program with arguments "arg1 arg2 arg3"
    Then the exit code should be 0
    And the stderr should contain "Program started"

  Scenario: Infinite iterator with non-deterministic annotation
    Given I have the following Asthra code:
      """
      package test;

      #[non_deterministic]
      pub fn main(none) -> i32 {
          let inf_iter = infinite();
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Shadowing predeclared log function
    Given I have the following Asthra code:
      """
      package test;

      pub fn log(level: string, message: string) -> void {
          // Custom log implementation
          return ();
      }

      pub fn main(none) -> i32 {
          log("INFO", "Custom logging");
          return 42;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Shadowing predeclared range function
    Given I have the following Asthra code:
      """
      package test;

      pub fn range(custom_param: string) -> []i32 {
          return [1, 2, 3];
      }

      pub fn main(none) -> i32 {
          let custom_range = range("custom");
          return custom_range.len;
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Using both predeclared and shadowed functions
    Given I have the following Asthra code:
      """
      package test;

      pub fn log(level: i32, message: string) -> void {
          // Custom log with different signature
          return ();
      }

      pub fn main(none) -> i32 {
          // Use predeclared range
          let mut total: i32 = 0;
          for i in range(4) {
              total = total + i;
          }
          
          // Use custom log
          log(1, "Custom message");
          
          return total;
      }
      """
    When I compile and run the program
    Then the exit code should be 6

  Scenario: Combining predeclared functions
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let arguments: []string = args();
          log("Arguments count: " + string(arguments.len));
          
          let mut sum: i32 = 0;
          for i in range(1, 6) {
              sum = sum + i;
          }
          
          if sum != 15 {
              panic("Sum calculation error");
          }
          
          return sum;
      }
      """
    When I compile and run the program
    Then the exit code should be 15

  Scenario: Error - Wrong type for log parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          log(42);  // Error: log expects string, not i32
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected string"

  Scenario: Error - Wrong type for range parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          for i in range("invalid") {  // Error: range expects i32
              return i;
          }
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected i32"

  Scenario: Error - Wrong type for panic parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          panic(123);  // Error: panic expects string
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected string"

  Scenario: Error - Wrong type for exit parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          exit("invalid");  // Error: exit expects i32
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "type mismatch" or "expected i32"

  Scenario: Error - Infinite without non-deterministic annotation
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let inf = infinite();  // Error: requires #[non_deterministic]
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "non_deterministic" or "annotation required"

  Scenario: Predeclared functions in different scopes
    Given I have the following Asthra code:
      """
      package test;

      pub fn helper_function(none) -> i32 {
          log("Helper function called");
          return 5;
      }

      pub fn main(none) -> i32 {
          log("Main function started");
          let result = helper_function();
          
          for i in range(result) {
              log("Loop iteration: " + string(i));
          }
          
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 5

  Scenario: Predeclared functions with complex expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let start: i32 = 2;
          let count: i32 = 3;
          let mut result: i32 = 0;
          
          for i in range(start, start + count) {
              result = result + i;
              log("Processing: " + string(i));
          }
          
          if result == 9 {  // 2 + 3 + 4 = 9
              log("Calculation correct");
              return 0;
          } else {
              panic("Calculation error");
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 0