Feature: Spawn statements
  As a developer
  I want to spawn concurrent tasks without handles
  So that I can perform fire-and-forget asynchronous operations

  Background:
    Given the Asthra compiler is available

  Scenario: Basic spawn statement
    Given I have the following Asthra code:
      """
      package test;

      pub fn background_task(none) -> void {
          // Simulate some work
          return ();
      }

      pub fn main(none) -> i32 {
          spawn background_task();
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn function with parameters
    Given I have the following Asthra code:
      """
      package test;

      pub fn worker_task(value: i32) -> void {
          // Process the value
          return ();
      }

      pub fn main(none) -> i32 {
          spawn worker_task(42);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Multiple spawn statements
    Given I have the following Asthra code:
      """
      package test;

      pub fn task_one(none) -> void {
          return ();
      }

      pub fn task_two(value: i32) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          spawn task_one();
          spawn task_two(100);
          spawn task_one();
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn with method call
    Given I have the following Asthra code:
      """
      package test;

      struct TaskManager {
          id: i32
      }

      impl TaskManager {
          pub fn process(self, data: i32) -> void {
              return ();
          }
      }

      pub fn main(none) -> i32 {
          let manager: TaskManager = TaskManager { id: 1 };
          spawn manager.process(42);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn statement with semicolon termination
    Given I have the following Asthra code:
      """
      package test;

      pub fn simple_task(none) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          spawn simple_task();
          let x: i32 = 5;
          return x;
      }
      """
    When I compile and run the program
    Then the exit code should be 5

  Scenario: Spawn with complex function call
    Given I have the following Asthra code:
      """
      package test;

      pub fn calculate_and_process(a: i32, b: i32, c: i32) -> void {
          let result = a + b * c;
          return ();
      }

      pub fn main(none) -> i32 {
          spawn calculate_and_process(1, 2, 3);
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Error - Spawn without function call
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          spawn value;  // Error: spawn expects function call
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected function call" or "invalid spawn target"

  Scenario: Error - Missing semicolon after spawn
    Given I have the following Asthra code:
      """
      package test;

      pub fn task(none) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          spawn task()  // Error: missing semicolon
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected semicolon" or "syntax error"

  Scenario: Error - Spawn with undefined function
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          spawn undefined_function();  // Error: function not defined
          return 0;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "undefined" or "not found"

  Scenario: Spawn with function returning non-void
    Given I have the following Asthra code:
      """
      package test;

      pub fn compute_value(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn compute_value();  // Function returns i32, not void
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn inside control flow
    Given I have the following Asthra code:
      """
      package test;

      pub fn conditional_task(none) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          let should_spawn: bool = true;
          
          if should_spawn {
              spawn conditional_task();
          }
          
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Spawn in loop
    Given I have the following Asthra code:
      """
      package test;

      pub fn indexed_task(index: i32) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          for i in range(3) {
              spawn indexed_task(i);
          }
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0