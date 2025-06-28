Feature: Basic Compiler Functionality
  As a developer
  I want the Asthra compiler to correctly compile valid programs
  So that I can build reliable applications

  Background:
    Given the Asthra compiler is available

  Scenario: Compile and run a simple Hello World program
    Given I have a file "hello.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          log("Hello, World!");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Hello, World!"
    And the exit code should be 0

  Scenario: Compile and run a program with multiple log statements
    Given I have a file "multiple_logs.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          log("Starting program");
          log("Processing data");
          log("Program completed");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Starting program"
    And the output should contain "Processing data"
    And the output should contain "Program completed"
    And the exit code should be 0

  Scenario: Compile and run a program with basic arithmetic
    Given I have a file "arithmetic.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 10;
          let y: i32 = 20;
          let sum: i32 = x + y;
          
          log("x = 10");
          log("y = 20");
          log("x + y = 30");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "x = 10"
    And the output should contain "y = 20"
    And the output should contain "x + y = 30"
    And the exit code should be 0

  Scenario: Handle syntax errors gracefully
    Given I have a file "syntax_error.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          log("Missing semicolon")
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "expected ';'"

  Scenario: Compile and run a program with function calls
    Given I have a file "function_calls.asthra" with:
      """
      package main;
      
      fn greet(none) -> void {
          log("Hello from greet function!");
          return ();
      }
      
      pub fn main(none) -> void {
          log("Main function starting");
          greet();
          log("Main function ending");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Main function starting"
    And the output should contain "Hello from greet function!"
    And the output should contain "Main function ending"
    And the exit code should be 0

  Scenario: Compile and run a program that returns 1
    Given I have a file "return_one.asthra" with:
      """
      package main;
      
      pub fn main(none) -> i32 {
          log("Program will exit with code 1");
          return 1;
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Program will exit with code 1"
    And the exit code should be 0

  Scenario: Compile and run a program with boolean operations
    Given I have a file "boolean_ops.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let is_true: bool = true;
          let is_false: bool = false;
          
          if is_true {
              log("is_true is true");
          }
          
          if !is_false {
              log("not false is true");
          }
          
          if is_true && !is_false {
              log("true AND (NOT false) is true");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "is_true is true"
    And the output should contain "not false is true"
    And the output should contain "true AND (NOT false) is true"
    And the exit code should be 0