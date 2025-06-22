Feature: If Condition Functionality
  As a developer
  I want to use if conditions in Asthra
  So that I can write conditional logic in my programs

  Background:
    Given the Asthra compiler is available

  Scenario: Simple if condition with true branch
    Given I have a file "if_true.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          if true {
              log("Condition is true");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Condition is true"
    And the exit code should be 0

  Scenario: Simple if condition with false branch
    Given I have a file "if_false.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          if false {
              log("This should not print");
          }
          log("Program completed");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should not contain "This should not print"
    And the output should contain "Program completed"
    And the exit code should be 0

  Scenario: If-else condition
    Given I have a file "if_else.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 10;
          if x > 5 {
              log("x is greater than 5");
          } else {
              log("x is not greater than 5");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "x is greater than 5"
    And the output should not contain "x is not greater than 5"
    And the exit code should be 0

  Scenario: Nested if conditions
    Given I have a file "nested_if.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let a: i32 = 10;
          let b: i32 = 20;
          
          if a > 5 {
              log("a is greater than 5");
              if b > 15 {
                  log("b is also greater than 15");
              }
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "a is greater than 5"
    And the output should contain "b is also greater than 15"
    And the exit code should be 0

  @wip
  Scenario: If condition with expression result
    Given I have a file "if_expression.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let result: i32 = if true { 42 } else { 0 };
          if result == 42 {
              log("Result is 42");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Result is 42"
    And the exit code should be 0

  @wip
  Scenario: If condition with complex boolean expression
    Given I have a file "complex_condition.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          let x: i32 = 10;
          let y: i32 = 20;
          let z: i32 = 30;
          
          if x < y && y < z {
              log("x < y < z is true");
          }
          
          if x > 5 || y < 10 {
              log("At least one condition is true");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "x < y < z is true"
    And the output should contain "At least one condition is true"
    And the exit code should be 0