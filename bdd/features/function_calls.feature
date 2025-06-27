Feature: Function Call Functionality
  As a developer
  I want to define and call functions in Asthra
  So that I can write modular and reusable code

  Background:
    Given the Asthra compiler is available

  Scenario: Call a simple function with no parameters
    Given I have a file "simple_function.asthra" with:
      """
      package main;
      
      fn greet(none) -> void {
          log("Hello from greet function!");
          return ();
      }
      
      pub fn main(none) -> void {
          greet();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Hello from greet function!"
    And the exit code should be 0

  Scenario: Call multiple functions in sequence
    Given I have a file "multiple_functions.asthra" with:
      """
      package main;
      
      fn first(none) -> void {
          log("First function called");
          return ();
      }
      
      fn second(none) -> void {
          log("Second function called");
          return ();
      }
      
      fn third(none) -> void {
          log("Third function called");
          return ();
      }
      
      pub fn main(none) -> void {
          first();
          second();
          third();
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "First function called"
    And the output should contain "Second function called"
    And the output should contain "Third function called"
    And the exit code should be 0

  Scenario: Call a function with integer parameters
    Given I have a file "function_with_params.asthra" with:
      """
      package main;
      
      fn add(a: i32, b: i32) -> i32 {
          return a + b;
      }
      
      pub fn main(none) -> void {
          let result: i32 = add(5, 3);
          if result == 8 {
              log("Addition result is correct: 8");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Addition result is correct: 8"
    And the exit code should be 0

  @wip
  Scenario: Call a function that returns a value
    Given I have a file "function_return.asthra" with:
      """
      package main;
      
      fn get_answer(none) -> i32 {
          return 42;
      }
      
      pub fn main(none) -> void {
          let answer: i32 = get_answer();
          if answer == 42 {
              log("The answer is 42");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "The answer is 42"
    And the exit code should be 0

  Scenario: Nested function calls
    Given I have a file "nested_calls.asthra" with:
      """
      package main;
      
      fn inner(x: i32) -> i32 {
          log("Inner function called");
          return x * 2;
      }
      
      fn middle(y: i32) -> i32 {
          log("Middle function called");
          let result: i32 = inner(y);
          return result + 1;
      }
      
      fn outer(z: i32) -> i32 {
          log("Outer function called");
          let result: i32 = middle(z);
          return result + 10;
      }
      
      pub fn main(none) -> void {
          let final_result: i32 = outer(5);
          // outer(5) -> middle(5) -> inner(5) -> 10 -> 11 -> 21
          if final_result == 21 {
              log("Nested calls result: 21");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Outer function called"
    And the output should contain "Middle function called"
    And the output should contain "Inner function called"
    And the output should contain "Nested calls result: 21"
    And the exit code should be 0

  Scenario: Function with multiple parameters of different types
    Given I have a file "mixed_params.asthra" with:
      """
      package main;
      
      fn process_data(count: i32, flag: bool, value: f32) -> void {
          if flag {
              log("Processing with flag enabled");
              if count > 0 {
                  log("Count is positive");
              }
              if value > 0.0 {
                  log("Value is positive");
              }
          } else {
              log("Processing with flag disabled");
          }
          return ();
      }
      
      pub fn main(none) -> void {
          process_data(5, true, 3.14);
          process_data(0, false, -1.0);
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Processing with flag enabled"
    And the output should contain "Count is positive"
    And the output should contain "Value is positive"
    And the output should contain "Processing with flag disabled"
    And the exit code should be 0

  Scenario: Recursive function calls
    Given I have a file "recursive.asthra" with:
      """
      package main;
      
      fn factorial(n: i32) -> i32 {
          if n <= 1 {
              return 1;
          } else {
              return n * factorial(n - 1);
          }
      }
      
      pub fn main(none) -> void {
          let result: i32 = factorial(5);
          // 5! = 5 * 4 * 3 * 2 * 1 = 120
          if result == 120 {
              log("Factorial of 5 is 120");
          }
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Factorial of 5 is 120"
    And the exit code should be 0

  Scenario: Function call in expression context
    Given I have a file "function_in_expression.asthra" with:
      """
      package main;
      
      fn double(x: i32) -> i32 {
          return x * 2;
      }
      
      fn triple(x: i32) -> i32 {
          return x * 3;
      }
      
      pub fn main(none) -> void {
          let result: i32 = double(5) + triple(3);
          // double(5) = 10, triple(3) = 9, total = 19
          if result == 19 {
              log("Expression result is 19");
          }
          
          // Function calls in condition
          if double(2) == 4 {
              log("Double of 2 is 4");
          }
          
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Expression result is 19"
    And the output should contain "Double of 2 is 4"
    And the exit code should be 0

  Scenario: Forward function declaration
    Given I have a file "forward_declaration.asthra" with:
      """
      package main;
      
      // main calls helper before it's defined
      pub fn main(none) -> void {
          helper();
          return ();
      }
      
      fn helper(none) -> void {
          log("Helper function called");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Helper function called"
    And the exit code should be 0

  @wip
  Scenario: Function with string parameters
    Given I have a file "string_function.asthra" with:
      """
      package main;
      
      fn greet_person(name: string) -> void {
          log("Hello, ");
          log(name);
          log("!");
          return ();
      }
      
      pub fn main(none) -> void {
          greet_person("Alice");
          greet_person("Bob");
          return ();
      }
      """
    When I compile the file
    Then the compilation should succeed
    And an executable should be created
    When I run the executable
    Then the output should contain "Hello, Alice!"
    And the output should contain "Hello, Bob!"
    And the exit code should be 0

  Scenario: Function overloading (if supported)
    Given I have a file "function_overload.asthra" with:
      """
      package main;
      
      priv fn print_value(x: i32) -> void {
          log("Integer value");
          return ();
      }
      
      priv fn print_value(x: f32) -> void {
          log("Float value");
          return ();
      }
      
      pub fn main(none) -> void {
          print_value(42);
          print_value(3.14);
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "Duplicate function declaration"

  @wip
  Scenario: Error - calling undefined function
    Given I have a file "undefined_function.asthra" with:
      """
      package main;
      
      pub fn main(none) -> void {
          undefined_function();
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "undefined function"

  @wip
  Scenario: Error - incorrect number of arguments
    Given I have a file "wrong_arg_count.asthra" with:
      """
      package main;
      
      fn add(a: i32, b: i32) -> i32 {
          return a + b;
      }
      
      pub fn main(none) -> void {
          let result: i32 = add(5);  // Missing second argument
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "incorrect number of arguments"

  @wip
  Scenario: Error - type mismatch in function arguments
    Given I have a file "type_mismatch.asthra" with:
      """
      package main;
      
      fn process_int(x: i32) -> void {
          return ();
      }
      
      pub fn main(none) -> void {
          process_int(3.14);  // Passing float to int parameter
          return ();
      }
      """
    When I compile the file
    Then the compilation should fail
    And the error message should contain "type mismatch"