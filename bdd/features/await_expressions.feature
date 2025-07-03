Feature: Await expressions
  As a developer
  I want to await asynchronous results from spawned tasks
  So that I can synchronize and get values from concurrent operations

  Background:
    Given the Asthra compiler is available

  Scenario: Basic await expression
    Given I have the following Asthra code:
      """
      package test;

      pub fn compute_value(none) -> i32 {
          return 42;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = compute_value();
          let result: i32 = await handle;
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 42

  Scenario: Await with arithmetic operations
    Given I have the following Asthra code:
      """
      package test;

      pub fn multiply(value: i32) -> i32 {
          return value * 3;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = multiply(7);
          let result: i32 = await handle;
          let final_result = result + 10;
          return final_result;
      }
      """
    When I compile and run the program
    Then the exit code should be 31

  Scenario: Multiple await expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn add_five(value: i32) -> i32 {
          return value + 5;
      }

      pub fn multiply_two(value: i32) -> i32 {
          return value * 2;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle1 = add_five(10);
          spawn_with_handle handle2 = multiply_two(6);
          
          let result1: i32 = await handle1;
          let result2: i32 = await handle2;
          
          return result1 + result2;
      }
      """
    When I compile and run the program
    Then the exit code should be 27

  Scenario: Await void function result
    Given I have the following Asthra code:
      """
      package test;

      pub fn side_effect_task(none) -> void {
          return ();
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = side_effect_task();
          await handle;  // Wait for completion
          return 0;
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Await with custom struct return type
    Given I have the following Asthra code:
      """
      package test;

      struct Point {
          x: i32,
          y: i32
      }

      pub fn create_point(x_val: i32, y_val: i32) -> Point {
          return Point { x: x_val, y: y_val };
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = create_point(3, 4);
          let point: Point = await handle;
          return point.x + point.y;
      }
      """
    When I compile and run the program
    Then the exit code should be 7

  Scenario: Await with method call
    Given I have the following Asthra code:
      """
      package test;

      struct Calculator {
          base: i32
      }

      impl Calculator {
          pub fn calculate(self, multiplier: i32) -> i32 {
              return self.base * multiplier;
          }
      }

      pub fn main(none) -> i32 {
          let calc: Calculator = Calculator { base: 5 };
          spawn_with_handle handle = calc.calculate(8);
          let result: i32 = await handle;
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 40

  Scenario: Await in expressions
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_number(none) -> i32 {
          return 15;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = get_number();
          let result = (await handle) * 2 + 5;
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 35

  Scenario: Await with Result type
    Given I have the following Asthra code:
      """
      package test;

      pub fn safe_divide(a: i32, b: i32) -> Result<i32, string> {
          if b == 0 {
              return Result.Err("Division by zero");
          } else {
              return Result.Ok(a / b);
          }
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = safe_divide(20, 4);
          let result: Result<i32, string> = await handle;
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Await with Option type
    Given I have the following Asthra code:
      """
      package test;

      pub fn find_value(search: bool) -> Option<i32> {
          if search {
              return Option.Some(99);
          } else {
              return Option.None;
          }
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = find_value(true);
          let result: Option<i32> = await handle;
          return 0;  // Just test compilation
      }
      """
    When I compile and run the program
    Then the exit code should be 0

  Scenario: Error - Await on non-handle type
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let value: i32 = 42;
          let result = await value;  // Error: not a handle
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "expected handle" or "invalid await target"

  Scenario: Error - Await undefined handle
    Given I have the following Asthra code:
      """
      package test;

      pub fn main(none) -> i32 {
          let result = await undefined_handle;  // Error: handle not defined
          return result;
      }
      """
    When I try to compile the program
    Then the compilation should fail
    And the error should mention "undefined" or "not found"

  Scenario: Await in control flow
    Given I have the following Asthra code:
      """
      package test;

      pub fn conditional_compute(value: i32) -> i32 {
          return value * 2;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = conditional_compute(5);
          
          if true {
              let result: i32 = await handle;
              return result;
          } else {
              return 0;
          }
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Await in loop
    Given I have the following Asthra code:
      """
      package test;

      pub fn process_index(index: i32) -> i32 {
          return index + 1;
      }

      pub fn main(none) -> i32 {
          let mut total: i32 = 0;
          
          for i in range(3) {
              spawn_with_handle handle = process_index(i);
              let result: i32 = await handle;
              total = total + result;
          }
          
          return total;
      }
      """
    When I compile and run the program
    Then the exit code should be 6

  Scenario: Await precedence with unary operators
    Given I have the following Asthra code:
      """
      package test;

      pub fn get_negative(none) -> i32 {
          return -10;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = get_negative();
          let result = -(await handle);  // Parentheses required for clarity
          return result;
      }
      """
    When I compile and run the program
    Then the exit code should be 10

  Scenario: Sequential awaits
    Given I have the following Asthra code:
      """
      package test;

      pub fn step_one(none) -> i32 {
          return 1;
      }

      pub fn step_two(input: i32) -> i32 {
          return input + 2;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle1 = step_one();
          let result1: i32 = await handle1;
          
          spawn_with_handle handle2 = step_two(result1);
          let result2: i32 = await handle2;
          
          return result2;
      }
      """
    When I compile and run the program
    Then the exit code should be 3

  Scenario: Await with function parameters
    Given I have the following Asthra code:
      """
      package test;

      pub fn multiply_by_two(value: i32) -> i32 {
          return value * 2;
      }

      pub fn process_result(value: i32) -> i32 {
          return value + 10;
      }

      pub fn main(none) -> i32 {
          spawn_with_handle handle = multiply_by_two(5);
          let final_result = process_result(await handle);
          return final_result;
      }
      """
    When I compile and run the program
    Then the exit code should be 20