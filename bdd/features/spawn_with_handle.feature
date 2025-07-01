Feature: Spawn with handle statements
  As a developer
  I want to spawn concurrent tasks with result handles
  So that I can capture and await results from asynchronous operations

  Background:
    Given the Asthra compiler is available

  Scenario: Basic spawn with handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn compute_task(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = compute_task();
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with handle and parameter
    Given I have the following Asthra code:
      """
      package test;

      pub fn multiply_task(value: i32) -> i32 {
          return value * 2;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle result_handle = multiply_task(21);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Multiple spawn with handle statements
    Given I have the following Asthra code:
      """
      package test;

      pub fn task_one(none) -> i32 {
          return 10;
      }

      pub fn task_two(value: i32) -> i32 {
          return value + 5;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle1 = task_one();
          spawn_with_handle handle2 = task_two(15);
          spawn_with_handle handle3 = task_one();
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with handle using method call
    Given I have the following Asthra code:
      """
      package test;

      struct Calculator {
          base: i32
      }

      impl Calculator {
          pub fn add(self, value: i32) -> i32 {
              return self.base + value;
          }
      }

      pub fn main(none) -> i32 {
          let calc: Calculator = Calculator { base: 10 };
          spawn_with_handle result = calc.add(5);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with handle for void function
    Given I have the following Asthra code:
      """
      package test;

      pub fn side_effect_task(none) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = side_effect_task();
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with handle and complex parameters
    Given I have the following Asthra code:
      """
      package test;

      pub fn complex_calculation(a: i32, b: i32, c: i32) -> i32 {
          return a * b + c;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle result = complex_calculation(2, 3, 4);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with handle returning custom type
    Given I have the following Asthra code:
      """
      package test;

      struct Result {
          value: i32,
          success: bool
      }

      pub fn create_result(val: i32) -> Result {
          return Result { value: val, success: true };
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = create_result(100);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Error - Missing variable name in spawn_with_handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn task(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle = task();  // Error: missing handle variable name
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected identifier" or "syntax error"

  Scenario: Error - Missing assignment in spawn_with_handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn task(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle task();  // Error: missing = operator
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected '='" or "syntax error"

  Scenario: Error - Missing semicolon after spawn_with_handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn task(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = task()  // Error: missing semicolon
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected semicolon" or "syntax error"

  Scenario: Error - Invalid expression after spawn_with_handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          spawn_with_handle handle = value;  // Error: not a function call
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected function call" or "invalid expression"

  Scenario: Error - Undefined function in spawn_with_handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          spawn_with_handle handle = undefined_function();  // Error: function not defined
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "undefined" or "not found"

  Scenario: Spawn with handle inside control flow
    Given I have the following Asthra code:
      """
      package test;

      pub fn conditional_compute(value: i32) -> i32 {
          return value * 2;
      }

      pub fn main(none) -> i32 {
          let should_compute: bool = true;
          
          if should_compute {
              spawn_with_handle result = conditional_compute(10);
          }
          
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with handle in loop
    Given I have the following Asthra code:
      """
      package test;

      pub fn process_item(item: i32) -> i32 {
          return item + 1;
      }

      pub fn main(none) -> i32 {
          for i in range(3) {
              spawn_with_handle result = process_item(i);
          }
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Handle variable scoping
    Given I have the following Asthra code:
      """
      package test;

      pub fn compute(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle outer_handle = compute();
          
          if true {
              spawn_with_handle inner_handle = compute();
              // inner_handle is scoped to this block
          }
          
          // outer_handle is still accessible here
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0